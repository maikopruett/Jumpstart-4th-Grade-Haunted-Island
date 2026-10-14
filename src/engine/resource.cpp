#include "engine/resource.h"

#include <cstdio>
#include <vector>

#include "engine/sound.h"
#include "engine/vfs.h"

namespace {

struct Archive {
    VFile* file;
    std::vector<u32> offsets; // absolute file offsets
    u32 base;
    u32 count;
};

Archive g_archives[8];
int g_numArchives = 0;
int g_lastSize = 0;

// ---- Huffman + LZ77 chunk decoder (hand-written asm at 0x401a44..0x40207a)

const int kLitSyms = 0x11e;  // 0..255 literal, 0x100..0x11c len 3..31, 0x11d long len
const int kDistSyms = 0x80;
const u8 kDefaultDist[] = {0x02, 0x01, 0x01, 0x03, 0x06, 0x0d, 0x1f, 0x4e, 0x00};

struct BitReader {
    const u8* p;
    const u8* end;
    u32 bit = 0;
    u32 Get(int n) {
        u32 v = 0;
        for (int i = 0; i < n; i++) {
            u8 b = (p < end) ? *p : 0;
            v = (v << 1) | ((b >> (7 - bit)) & 1);
            if (++bit == 8) {
                bit = 0;
                p++;
            }
        }
        return v;
    }
};

struct Huff {
    int nsyms = 0;
    int single = -1;
    int root = 0;
    std::vector<int> left, right;
    // Builder state mirroring the asm's recursive walk.
    const u8* lens = nullptr;
    int depth = 0, target = 1, next = 0, idx = -1;
    bool bad = false;

    int Build() {
        if (target == depth) {
            while (++idx < nsyms) {
                if (lens[idx] == target) return idx;
            }
            idx = -1;
            target++;
            if (target > 32) {
                bad = true;
                return 0;
            }
        }
        int node = next++;
        if ((int)left.size() <= node - nsyms) {
            left.resize(node - nsyms + 1);
            right.resize(node - nsyms + 1);
        }
        depth++;
        int l = Build();
        if (bad) return 0;
        int r = Build();
        if (bad) return 0;
        depth--;
        left[node - nsyms] = l;
        right[node - nsyms] = r;
        return node;
    }

    bool Init(const u8* lengths, int n) {
        nsyms = n;
        lens = lengths;
        depth = 0;
        target = 1;
        next = n;
        idx = -1;
        root = Build();
        return !bad;
    }

    int Decode(BitReader& br) const {
        if (single >= 0) return single;
        int node = root;
        while (node >= nsyms) node = br.Get(1) ? right[node - nsyms] : left[node - nsyms];
        return node;
    }
};

bool ReadLitTable(BitReader& br, Huff& h) {
    u8 lens[kLitSyms] = {0};
    for (int i = 0; i < 3; i++) lens[i] = br.Get(1) ? (u8)(br.Get(4) + 1) : 0;
    if (lens[0] == 1 && lens[1] == 1 && lens[2] == 1) {
        h.single = (int)br.Get(9);
        return true;
    }
    for (int i = 3; i < kLitSyms; i++) lens[i] = br.Get(1) ? (u8)(br.Get(4) + 1) : 0;
    return h.Init(lens, kLitSyms);
}

bool ReadDistTable(BitReader& br, Huff& h) {
    u8 lens[kDistSyms] = {0};
    if (!br.Get(1)) {
        u8 len = kDefaultDist[0];
        int k = 1;
        for (int sym = 0; sym < kDistSyms; sym++) {
            while (sym == kDefaultDist[k]) {
                len++;
                k++;
            }
            lens[sym] = len;
        }
        return h.Init(lens, kDistSyms);
    }
    for (int i = 0; i < 3; i++) lens[i] = (u8)br.Get(4);
    if (lens[0] == 1 && lens[1] == 1 && lens[2] == 1) {
        h.single = (int)br.Get(7);
        return true;
    }
    for (int i = 3; i < kDistSyms; i++) lens[i] = (u8)br.Get(4);
    return h.Init(lens, kDistSyms);
}

u32 DecompressChunk(const u8* src, u32 srcLen, u8* out, u32 outCap) {
    if (srcLen < 6) return 0;
    u32 size = RdU32(src) & 0x3fffffff;
    if (size > outCap) size = outCap;
    BitReader br{src + 6, src + srcLen};
    br.Get(1);
    Huff lit, dist;
    if (!ReadLitTable(br, lit) || !ReadDistTable(br, dist)) return 0;
    u32 o = 0;
    while (o < size) {
        int sym = lit.Decode(br);
        if (sym < 0x100) {
            out[o++] = (u8)sym;
            continue;
        }
        if (sym == 0x11d) sym += (int)br.Get(8);
        int length = sym - 0xfd;
        u32 d = (((u32)dist.Decode(br) << 6) | br.Get(6)) + 1;
        if (d > o) return o; // corrupt
        for (int i = 0; i < length && o < size; i++, o++) out[o] = out[o - d];
    }
    return o;
}

} // namespace

u32 Res_Decompress(const u8* src, u32 srcSize, u8** dst, int dstOffset) {
    if (srcSize < 4 || memcmp(src, "COMP", 4) != 0) {
        if (*dst == nullptr) *dst = (u8*)Mem_Alloc(srcSize + dstOffset);
        memcpy(*dst + dstOffset, src, srcSize);
        return srcSize;
    }
    s32 total = RdS32(src + 4);
    u8* out = *dst ? *dst : (u8*)Mem_Alloc(total + dstOffset);
    u8* o = out + dstOffset;
    const u8* p = src + 8;
    const u8* end = src + srcSize;
    u32 done = 0;
    while ((s32)done < total && p + 8 <= end) {
        u32 usize = RdU32(p);
        u32 csize = RdU32(p + 4);
        p += 8;
        if (p + csize > end) break;
        u32 cap = (u32)total - done;
        u32 n;
        if (usize == csize) {
            n = usize < cap ? usize : cap;
            memmove(o, p, n);
        } else {
            n = DecompressChunk(p, csize, o, cap);
            if (n == 0) break;
        }
        done += n;
        o += n;
        p += csize;
    }
    *dst = out;
    return done;
}

bool Res_OpenArchive(const char* path) {
    if (g_numArchives >= 8) return false;
    VFile* f = Vfs_Open(path);
    if (!f) {
        fprintf(stderr, "resource archive missing: %s\n", path);
        return false;
    }
    u32 hdr[3];
    if (Vfs_Read(f, hdr, 12) != 12 || hdr[0] != 0x4644) {
        Vfs_Close(f);
        return false;
    }
    Archive& a = g_archives[g_numArchives];
    a.file = f;
    a.base = hdr[1];
    a.count = hdr[2];
    a.offsets.resize(a.count);
    Vfs_Read(f, a.offsets.data(), a.count * 4);
    for (u32& o : a.offsets) o += a.count * 4 + 12;
    g_numArchives++;
    return true;
}

void Res_CloseAll() {
    while (g_numArchives > 0) {
        Archive& a = g_archives[--g_numArchives];
        Vfs_Close(a.file);
        a.file = nullptr;
        a.offsets.clear();
    }
}

void* Res_Load(u32 id) {
    Archive* a = nullptr;
    for (int i = g_numArchives - 1; i >= 0; i--) {
        if (id - g_archives[i].base < g_archives[i].count) {
            a = &g_archives[i];
            break;
        }
    }
    if (!a || id < a->base) return nullptr;
    u32 idx = id - a->base;
    u32 off = a->offsets[idx];
    u32 next = (idx + 1 < a->count) ? a->offsets[idx + 1] : (u32)Vfs_Size(a->file);
    u32 span = next - off;
    if (span < 16) return nullptr;
    u8 hdr[16];
    Vfs_Seek(a->file, off, SEEK_SET);
    Vfs_Read(a->file, hdr, 16);
    u32 flags = RdU32(hdr + 4);
    s32 size = RdS32(hdr + 12);
    g_lastSize = size;
    u8* buf = (u8*)Mem_Alloc(size + 1);
    buf[size] = 0;
    Sound_Update();
    if ((flags & 0xff) == 0) {
        Vfs_Read(a->file, buf, size);
    } else if ((flags & 0xff) == 1) {
        u32 csize = span - 16;
        u8* tmp = (u8*)Mem_Alloc(csize);
        Vfs_Read(a->file, tmp, csize);
        Res_Decompress(tmp, csize, &buf, 0);
        Mem_Free(tmp);
    } else {
        Mem_Free(buf);
        return nullptr;
    }
    Sound_Update();
    return buf;
}

void Res_Free(const void* p) { Mem_Free(p); }
int Res_LastSize() { return g_lastSize; }
