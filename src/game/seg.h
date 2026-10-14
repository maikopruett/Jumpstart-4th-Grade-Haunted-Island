// Emulation of 4G.EXE's data segment (0x42f000..0x443000: DATA + BSS).
//
// The game logic was ported from the decompiled program, which keeps nearly
// all of its state in fixed global addresses. Rather than re-typing hundreds
// of globals, the port keeps the original memory image (loaded from the
// user's 4G.EXE at startup) and accesses it by address:
//
//   D32(0x4395bc)        int global            DSTR(0x4319b4)  string literal
//   D16 / DU16 / D8      shorter globals       DPTR(va)        pointer into the segment
//   P(va)                a global holding a pointer (64-bit shadow slot)
//
// Pointer-valued slots can't live in 32-bit memory on arm64, so every slot
// the original used as a data pointer is accessed through P(va). Slots that
// are pointers in the EXE's initialized data (found via its relocation table)
// start out pointing into this segment. Slots that hold *code* addresses keep
// the original 32-bit x86 address in D32() and are invoked via CODE(va).
#pragma once
#include "engine/engine.h"

enum : u32 { SEG_BASE = 0x42f000, SEG_END = 0x443000, SEG_SIZE = SEG_END - SEG_BASE };

extern u8 g_seg[SEG_SIZE];

typedef s32 __attribute__((aligned(1))) s32u;
typedef u32 __attribute__((aligned(1))) u32u;
typedef s16 __attribute__((aligned(1))) s16u;
typedef u16 __attribute__((aligned(1))) u16u;

void Seg_Init();
[[noreturn]] void Seg_Fault(u32 va);

static inline u8* DPTR(u32 va) {
#ifndef NDEBUG
    if (va < SEG_BASE || va >= SEG_END) Seg_Fault(va);
#endif
    return g_seg + (va - SEG_BASE);
}
static inline s32u& D32(u32 va) { return *(s32u*)DPTR(va); }
static inline u32u& DU32(u32 va) { return *(u32u*)DPTR(va); }
static inline s16u& D16(u32 va) { return *(s16u*)DPTR(va); }
static inline u16u& DU16(u32 va) { return *(u16u*)DPTR(va); }
static inline u8& D8(u32 va) { return *DPTR(va); }
static inline s8& DS8(u32 va) { return *(s8*)DPTR(va); }
static inline char* DSTR(u32 va) { return (char*)DPTR(va); }

// 64-bit shadow for 32-bit pointer slots.
void*& P(u32 va);
template <typename T>
static inline T* PT(u32 va) { return (T*)P(va); }

// ---- code addresses stored in data
typedef void (*CodeFn)();
void Code_Register(u32 va, CodeFn fn);
CodeFn Code_Lookup(u32 va); // aborts on unknown addresses
#define CODE(va) (Code_Lookup((u32)(va)))
// Registers a ported function under its original address at startup.
#define REGISTER_CODE(va, fn) \
    static const bool _code_reg_##fn = (Code_Register((va), (CodeFn)(fn)), true)
