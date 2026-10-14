#include <cctype>
#include "engine/exeres.h"

#include <cstdio>
#include <string>

#include "engine/vfs.h"

namespace {

const u32 kImageBase = 0x400000;
std::vector<u8> g_image; // mapped by RVA
u32 g_rsrcRva = 0;

const u8* RvaPtr(u32 rva) { return rva < g_image.size() ? &g_image[rva] : nullptr; }

// Finds resource data (type/id/first language) in the .rsrc tree. If
// `name` is given the second level is matched by (ASCII) name instead of id.
const u8* FindResource(u32 type, u32 id, u32* size, const char* name = nullptr) {
    if (!g_rsrcRva) return nullptr;
    const u8* root = RvaPtr(g_rsrcRva);
    auto findEntry = [&](const u8* dir, u32 want, bool any, const char* wantName) -> u32 {
        int nNamed = RdU16(dir + 12), nId = RdU16(dir + 14);
        for (int i = 0; i < nNamed + nId; i++) {
            const u8* e = dir + 16 + i * 8;
            u32 n = RdU32(e), off = RdU32(e + 4);
            if (any) return off;
            if (wantName) {
                if (!(n & 0x80000000u)) continue;
                const u8* str = root + (n & 0x7fffffff);
                int len = RdU16(str);
                bool eq = (int)strlen(wantName) == len;
                for (int k = 0; eq && k < len; k++)
                    eq = RdU16(str + 2 + k * 2) == (u16)toupper((unsigned char)wantName[k]);
                if (eq) return off;
            } else if (!(n & 0x80000000u) && n == want) {
                return off;
            }
        }
        return 0xffffffffu;
    };
    u32 off = findEntry(root, type, false, nullptr);
    if (off == 0xffffffffu || !(off & 0x80000000u)) return nullptr;
    const u8* d1 = root + (off & 0x7fffffff);
    off = findEntry(d1, id, false, name);
    if (off == 0xffffffffu || !(off & 0x80000000u)) return nullptr;
    const u8* d2 = root + (off & 0x7fffffff);
    off = findEntry(d2, 0, true, nullptr);
    if (off == 0xffffffffu || (off & 0x80000000u)) return nullptr;
    const u8* leaf = root + off;
    if (size) *size = RdU32(leaf + 4);
    return RvaPtr(RdU32(leaf));
}

} // namespace

bool ExeRes_Load() {
    if (!g_image.empty()) return true;
    std::string file;
    if (!Vfs_ReadAll("SUPPORT\\CMN\\4G\\4G.EXE", file) || file.size() < 0x400) return false;
    const u8* d = (const u8*)file.data();
    u32 pe = RdU32(d + 0x3c);
    if (memcmp(d + pe, "PE\0\0", 4) != 0) return false;
    int nsec = RdU16(d + pe + 6);
    int optSize = RdU16(d + pe + 20);
    u32 imageSize = RdU32(d + pe + 24 + 56);
    g_image.assign(imageSize, 0);
    u32 headerSize = RdU32(d + pe + 24 + 60);
    if (headerSize > file.size()) headerSize = (u32)file.size();
    if (headerSize > g_image.size()) headerSize = (u32)g_image.size();
    memcpy(g_image.data(), d, headerSize);
    for (int i = 0; i < nsec; i++) {
        const u8* s = d + pe + 24 + optSize + i * 40;
        u32 vsize = RdU32(s + 8), va = RdU32(s + 12), rsize = RdU32(s + 16), rptr = RdU32(s + 20);
        u32 n = rsize < vsize || vsize == 0 ? rsize : vsize;
        if (va + n > g_image.size() || rptr + n > file.size()) continue;
        memcpy(&g_image[va], d + rptr, n);
        if (memcmp(s, ".rsrc", 5) == 0) g_rsrcRva = va;
    }
    return true;
}

const u8* Exe_VA(u32 va) {
    static const u8 zeros[16] = {0};
    if (va < kImageBase || va - kImageBase >= g_image.size()) {
        fprintf(stderr, "Exe_VA: address %08x outside image\n", va);
        return zeros;
    }
    return &g_image[va - kImageBase];
}

bool ExeRes_LoadCursor(int groupId, ExeCursor* out) {
    u32 gsize;
    const u8* grp = FindResource(12 /* RT_GROUP_CURSOR */, groupId, &gsize);
    if (!grp || gsize < 20) return false;
    u32 curId = RdU16(grp + 6 + 12);
    u32 csize;
    const u8* cur = FindResource(1 /* RT_CURSOR */, curId, &csize);
    if (!cur || csize < 4 + 40 + 8) return false;
    out->hotX = RdU16(cur);
    out->hotY = RdU16(cur + 2);
    const u8* bih = cur + 4;
    int w = RdS32(bih + 4);
    int h2 = RdS32(bih + 8);
    int bpp = RdU16(bih + 14);
    if (bpp != 1 || w <= 0 || h2 <= 0) return false;
    int h = h2 / 2;
    const u8* pal = bih + RdU32(bih);
    const u8* xorMask = pal + 8;
    int stride = ((w + 31) / 32) * 4;
    const u8* andMask = xorMask + stride * h;
    out->w = w;
    out->h = h;
    out->argb.assign(w * h, 0);
    for (int y = 0; y < h; y++) {
        int srcRow = h - 1 - y; // bottom-up
        for (int x = 0; x < w; x++) {
            int bit = 7 - (x & 7);
            int a = (andMask[srcRow * stride + x / 8] >> bit) & 1;
            int xo = (xorMask[srcRow * stride + x / 8] >> bit) & 1;
            u32 c;
            if (a == 0) c = xo ? 0xffffffffu : 0xff000000u;
            else c = xo ? 0xff000000u : 0x00000000u; // inverted pixels drawn black
            out->argb[y * w + x] = c;
        }
    }
    return true;
}

bool ExeRes_LoadIcon(const char* groupName, ExeCursor* out) {
    u32 gsize;
    const u8* grp = FindResource(14 /* RT_GROUP_ICON */, 0, &gsize, groupName);
    if (!grp || gsize < 20) return false;
    int count = RdU16(grp + 4);
    int best = -1, bestScore = -1;
    for (int i = 0; i < count; i++) {
        const u8* e = grp + 6 + i * 14;
        int w = e[0] ? e[0] : 256, bpp = RdU16(e + 6);
        int score = w * 64 + bpp;
        if (score > bestScore) {
            bestScore = score;
            best = i;
        }
    }
    if (best < 0) return false;
    u32 isize;
    const u8* ico = FindResource(3 /* RT_ICON */, RdU16(grp + 6 + best * 14 + 12), &isize);
    if (!ico || isize < 40) return false;
    int w = RdS32(ico + 4), h = RdS32(ico + 8) / 2, bpp = RdU16(ico + 14);
    if (w <= 0 || h <= 0 || (bpp != 1 && bpp != 4 && bpp != 8)) return false;
    int colors = RdU32(ico + 32) ? (int)RdU32(ico + 32) : (1 << bpp);
    const u8* pal = ico + RdU32(ico);
    const u8* xorBits = pal + colors * 4;
    int xstride = ((w * bpp + 31) / 32) * 4;
    const u8* andBits = xorBits + xstride * h;
    int astride = ((w + 31) / 32) * 4;
    out->w = w;
    out->h = h;
    out->hotX = out->hotY = 0;
    out->argb.assign(w * h, 0);
    for (int y = 0; y < h; y++) {
        int row = h - 1 - y;
        for (int x = 0; x < w; x++) {
            int idx;
            if (bpp == 8) idx = xorBits[row * xstride + x];
            else if (bpp == 4) idx = (xorBits[row * xstride + x / 2] >> ((x & 1) ? 0 : 4)) & 15;
            else idx = (xorBits[row * xstride + x / 8] >> (7 - (x & 7))) & 1;
            bool transparent = (andBits[row * astride + x / 8] >> (7 - (x & 7))) & 1;
            const u8* c = pal + idx * 4;
            out->argb[y * w + x] = transparent ? 0 : (0xff000000u | (c[2] << 16) | (c[1] << 8) | c[0]);
        }
    }
    return true;
}
