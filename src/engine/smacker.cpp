#include "engine/smacker.h"

#include <string>
#include <vector>

#include "engine/vfs.h"

namespace {

struct Bits {
    const u8* data;
    size_t size;  // bytes
    size_t pos = 0; // bit position
    int Get1() {
        size_t byte = pos >> 3;
        int b = byte < size ? (data[byte] >> (pos & 7)) & 1 : 0;
        pos++;
        return b;
    }
    u32 Get8() {
        u32 v = 0;
        for (int i = 0; i < 8; i++) v |= (u32)Get1() << i;
        return v;
    }
};

struct Huff8 {
    struct Node {
        int left = -1, right = -1;
        int value = 0;
    };
    std::vector<Node> nodes;

    int BuildRec(Bits& b, int depth) {
        int idx = (int)nodes.size();
        nodes.emplace_back();
        if (depth > 32) return idx;
        if (b.Get1()) {
            int l = BuildRec(b, depth + 1);
            int r = BuildRec(b, depth + 1);
            nodes[idx].left = l;
            nodes[idx].right = r;
        } else {
            nodes[idx].value = (int)b.Get8();
        }
        return idx;
    }

    void Build(Bits& b) {
        nodes.clear();
        if (!b.Get1()) {
            nodes.emplace_back(); // empty tree: always 0
            return;
        }
        BuildRec(b, 0);
        b.Get1(); // terminating 0 bit
    }

    int Lookup(Bits& b) const {
        int n = 0;
        while (nodes[n].left >= 0) n = b.Get1() ? nodes[n].right : nodes[n].left;
        return nodes[n].value;
    }
};

const u32 kCacheRef = 0x80000000u;

struct Huff16 {
    struct Node {
        int left = -1, right = -1;
        u32 value = 0;
    };
    std::vector<Node> nodes;
    u32 escape[3] = {0, 0, 0};
    u32 cache[3] = {0, 0, 0};
    bool empty = true;

    int BuildRec(Bits& b, const Huff8& lo, const Huff8& hi, int depth) {
        int idx = (int)nodes.size();
        nodes.emplace_back();
        if (depth > 32) return idx;
        if (b.Get1()) {
            int l = BuildRec(b, lo, hi, depth + 1);
            int r = BuildRec(b, lo, hi, depth + 1);
            nodes[idx].left = l;
            nodes[idx].right = r;
        } else {
            u32 v = (u32)lo.Lookup(b) | ((u32)hi.Lookup(b) << 8);
            if (v == escape[0]) v = kCacheRef | 0;
            else if (v == escape[1]) v = kCacheRef | 1;
            else if (v == escape[2]) v = kCacheRef | 2;
            nodes[idx].value = v;
        }
        return idx;
    }

    void Build(Bits& b) {
        nodes.clear();
        if (!b.Get1()) {
            empty = true;
            nodes.emplace_back();
            return;
        }
        empty = false;
        Huff8 lo, hi;
        lo.Build(b);
        hi.Build(b);
        for (u32& e : escape) e = b.Get8() | (b.Get8() << 8);
        BuildRec(b, lo, hi, 0);
        b.Get1();
    }

    void Reset() { cache[0] = cache[1] = cache[2] = 0; }

    u32 Lookup(Bits& b) {
        if (empty) return 0;
        int n = 0;
        while (nodes[n].left >= 0) n = b.Get1() ? nodes[n].right : nodes[n].left;
        u32 v = nodes[n].value;
        if (v & kCacheRef) v = cache[v & 3];
        if (cache[0] != v) {
            cache[2] = cache[1];
            cache[1] = cache[0];
            cache[0] = v;
        }
        return v;
    }
};

const int kBlockRuns[64] = {1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12,  13,  14,  15,   16,
                            17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,  29,  30,  31,   32,
                            33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44,  45,  46,  47,   48,
                            49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 128, 256, 512, 1024, 2048};

} // namespace

struct SmkFile {
    std::string data;
    bool v4 = false;
    int width = 0, height = 0, frames = 0;
    u32 frameUsec = 100000;
    std::vector<u32> frameOffset, frameSize;
    std::vector<u8> frameType;
    Huff16 mmap, mclr, full, type;
    std::vector<u8> pixels;
    u8 palette[768] = {0};
    int cur = -1;
};

SmkFile* Smk_Open(const char* path) {
    SmkFile* s = new SmkFile;
    if (!Vfs_ReadAll(path, s->data) || s->data.size() < 104) {
        delete s;
        return nullptr;
    }
    const u8* d = (const u8*)s->data.data();
    if (memcmp(d, "SMK2", 4) != 0 && memcmp(d, "SMK4", 4) != 0) {
        delete s;
        return nullptr;
    }
    s->v4 = d[3] == '4';
    s->width = (int)RdU32(d + 4);
    s->height = (int)RdU32(d + 8);
    s->frames = (int)RdU32(d + 12);
    s32 rate = RdS32(d + 16);
    u32 flags = RdU32(d + 20);
    if (rate > 0) s->frameUsec = (u32)rate * 1000;
    else if (rate < 0) s->frameUsec = (u32)(-rate) * 10;
    u32 treesSize = RdU32(d + 52);
    int nframes = s->frames + ((flags & 1) ? 1 : 0);
    size_t pos = 104;
    if (pos + (size_t)nframes * 5 > s->data.size()) {
        delete s;
        return nullptr;
    }
    s->frameSize.resize(nframes);
    s->frameType.resize(nframes);
    s->frameOffset.resize(nframes);
    for (int i = 0; i < nframes; i++) s->frameSize[i] = RdU32(d + pos + i * 4) & ~3u;
    pos += nframes * 4;
    for (int i = 0; i < nframes; i++) s->frameType[i] = d[pos + i];
    pos += nframes;
    Bits b{d + pos, treesSize};
    s->mmap.Build(b);
    s->mclr.Build(b);
    s->full.Build(b);
    s->type.Build(b);
    pos += treesSize;
    for (int i = 0; i < nframes; i++) {
        s->frameOffset[i] = (u32)pos;
        pos += s->frameSize[i];
    }
    s->pixels.assign((size_t)s->width * s->height, 0);
    return s;
}

void Smk_Close(SmkFile* s) { delete s; }
int Smk_Width(const SmkFile* s) { return s->width; }
int Smk_Height(const SmkFile* s) { return s->height; }
int Smk_Frames(const SmkFile* s) { return s->frames; }
u32 Smk_FrameUsec(const SmkFile* s) { return s->frameUsec; }
const u8* Smk_Pixels(const SmkFile* s) { return s->pixels.data(); }
const u8* Smk_Palette(const SmkFile* s) { return s->palette; }
int Smk_CurrentFrame(const SmkFile* s) { return s->cur; }

static void DecodePalette(SmkFile* s, const u8* p, const u8* end) {
    u8 old[768];
    memcpy(old, s->palette, 768);
    int i = 0;
    while (i < 256 && p < end) {
        u8 t = *p++;
        if (t & 0x80) {
            i += (t & 0x7f) + 1;
        } else if (t & 0x40) {
            if (p >= end) break;
            int off = *p++;
            int n = (t & 0x3f) + 1;
            while (n-- && i < 256 && off < 256) {
                memcpy(s->palette + i * 3, old + off * 3, 3);
                i++;
                off++;
            }
        } else {
            if (p + 2 > end) break;
            u8 c[3] = {t, (u8)(*p++ & 0x3f), (u8)(*p++ & 0x3f)};
            for (int k = 0; k < 3; k++) s->palette[i * 3 + k] = (u8)((c[k] << 2) | (c[k] >> 4));
            i++;
        }
    }
}

static void DecodeVideo(SmkFile* s, const u8* data, size_t len, Rect* dirty) {
    Bits b{data, len};
    s->mmap.Reset();
    s->mclr.Reset();
    s->full.Reset();
    s->type.Reset();
    int bw = s->width / 4, bh = s->height / 4;
    int blocks = bw * bh;
    int stride = s->width;
    int minX = bw, minY = bh, maxX = -1, maxY = -1;
    int blk = 0;
    auto touch = [&](int bi) {
        int bx = bi % bw, by = bi / bw;
        if (bx < minX) minX = bx;
        if (bx > maxX) maxX = bx;
        if (by < minY) minY = by;
        if (by > maxY) maxY = by;
    };
    while (blk < blocks) {
        u32 t = s->type.Lookup(b);
        int run = kBlockRuns[(t >> 2) & 0x3f];
        switch (t & 3) {
            case 0: // two-color block
                while (run-- && blk < blocks) {
                    u32 clr = s->mclr.Lookup(b);
                    u32 map = s->mmap.Lookup(b);
                    u8 hi = (u8)(clr >> 8), lo = (u8)clr;
                    u8* out = &s->pixels[(blk / bw) * 4 * stride + (blk % bw) * 4];
                    for (int i = 0; i < 4; i++) {
                        out[0] = (map & 1) ? hi : lo;
                        out[1] = (map & 2) ? hi : lo;
                        out[2] = (map & 4) ? hi : lo;
                        out[3] = (map & 8) ? hi : lo;
                        map >>= 4;
                        out += stride;
                    }
                    touch(blk);
                    blk++;
                }
                break;
            case 1: { // full block
                int mode = 0;
                if (s->v4) {
                    if (b.Get1()) mode = 1;
                    else if (b.Get1()) mode = 2;
                }
                while (run-- && blk < blocks) {
                    u8* out = &s->pixels[(blk / bw) * 4 * stride + (blk % bw) * 4];
                    if (mode == 0) {
                        for (int i = 0; i < 4; i++) {
                            u32 pix = s->full.Lookup(b);
                            out[2] = (u8)pix;
                            out[3] = (u8)(pix >> 8);
                            pix = s->full.Lookup(b);
                            out[0] = (u8)pix;
                            out[1] = (u8)(pix >> 8);
                            out += stride;
                        }
                    } else if (mode == 1) {
                        for (int k = 0; k < 2; k++) {
                            u32 pix = s->full.Lookup(b);
                            for (int r = 0; r < 2; r++) {
                                out[0] = out[1] = (u8)pix;
                                out[2] = out[3] = (u8)(pix >> 8);
                                out += stride;
                            }
                        }
                    } else {
                        for (int k = 0; k < 2; k++) {
                            u32 pix2 = s->full.Lookup(b);
                            u32 pix1 = s->full.Lookup(b);
                            for (int r = 0; r < 2; r++) {
                                out[0] = (u8)pix1;
                                out[1] = (u8)(pix1 >> 8);
                                out[2] = (u8)pix2;
                                out[3] = (u8)(pix2 >> 8);
                                out += stride;
                            }
                        }
                    }
                    touch(blk);
                    blk++;
                }
                break;
            }
            case 2: // skip
                while (run-- && blk < blocks) blk++;
                break;
            case 3: { // solid fill
                u8 c = (u8)(t >> 8);
                while (run-- && blk < blocks) {
                    u8* out = &s->pixels[(blk / bw) * 4 * stride + (blk % bw) * 4];
                    for (int i = 0; i < 4; i++, out += stride) memset(out, c, 4);
                    touch(blk);
                    blk++;
                }
                break;
            }
        }
    }
    if (maxX < 0) *dirty = {0, 0, 0, 0};
    else *dirty = {minX * 4, minY * 4, (maxX - minX + 1) * 4, (maxY - minY + 1) * 4};
}

bool Smk_NextFrame(SmkFile* s, Rect* dirty, bool* palChanged) {
    if (s->cur + 1 >= s->frames) return false;
    s->cur++;
    const u8* d = (const u8*)s->data.data();
    size_t pos = s->frameOffset[s->cur];
    size_t end = pos + s->frameSize[s->cur];
    if (end > s->data.size()) end = s->data.size();
    u8 type = s->frameType[s->cur];
    *palChanged = false;
    if (type & 1) {
        size_t len = (size_t)d[pos] * 4;
        if (len == 0) len = 1;
        DecodePalette(s, d + pos + 1, d + (pos + len < end ? pos + len : end));
        pos += len;
        *palChanged = true;
    }
    for (int t = 0; t < 7; t++) {
        if (type & (2 << t)) {
            if (pos + 4 > end) break;
            pos += RdU32(d + pos);
        }
    }
    if (pos < end) DecodeVideo(s, d + pos, end - pos, dirty);
    else *dirty = {0, 0, 0, 0};
    return true;
}
