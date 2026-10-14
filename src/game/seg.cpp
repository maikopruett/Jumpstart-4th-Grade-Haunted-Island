#include "game/seg.h"

#include <cstdio>
#include <unordered_map>

u8 g_seg[SEG_SIZE];

namespace {

void* g_ptrs[SEG_SIZE / 4];
std::unordered_map<u32, void*>& Unaligned() {
    static std::unordered_map<u32, void*> m;
    return m;
}
std::unordered_map<u32, CodeFn>& Registry() {
    static std::unordered_map<u32, CodeFn> m;
    return m;
}

// Apply the EXE's base relocations: slots in the data segment that hold the
// address of other data become real pointers in P().
void SeedPointersFromRelocs() {
    const u8* hdr = Exe_VA(0x400000);
    u32 pe = RdU32(hdr + 0x3c);
    u32 relocRva = RdU32(hdr + pe + 24 + 96 + 5 * 8);
    u32 relocSize = RdU32(hdr + pe + 24 + 96 + 5 * 8 + 4);
    if (!relocRva) return;
    const u8* p = Exe_VA(0x400000 + relocRva);
    const u8* end = p + relocSize;
    while (p + 8 <= end) {
        u32 page = RdU32(p), size = RdU32(p + 4);
        if (size < 8) break;
        for (u32 i = 0; i < (size - 8) / 2; i++) {
            u16 e = RdU16(p + 8 + i * 2);
            if ((e >> 12) != 3) continue;
            u32 site = 0x400000 + page + (e & 0xfff);
            if (site < SEG_BASE || site + 4 > SEG_END) continue;
            u32 target = RdU32(g_seg + (site - SEG_BASE));
            if (target >= SEG_BASE && target < SEG_END) P(site) = g_seg + (target - SEG_BASE);
        }
        p += size;
    }
}

} // namespace

void Seg_Fault(u32 va) {
    fprintf(stderr, "segment access outside data image: %08x\n", va);
    abort();
}

void Seg_Init() {
    memcpy(g_seg, Exe_VA(SEG_BASE), SEG_SIZE);
    SeedPointersFromRelocs();
}

void*& P(u32 va) {
    if (va < SEG_BASE || va + 4 > SEG_END) Seg_Fault(va);
    if (va & 3) return Unaligned()[va];
    return g_ptrs[(va - SEG_BASE) / 4];
}

void Code_Register(u32 va, CodeFn fn) { Registry()[va] = fn; }

CodeFn Code_Lookup(u32 va) {
    auto it = Registry().find(va);
    if (it == Registry().end()) {
        fprintf(stderr, "call through unported code address %08x\n", va);
        abort();
    }
    return it->second;
}
