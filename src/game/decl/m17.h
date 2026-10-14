// Declarations of unit "m17" (see re/units.json). Generated from the
// decompilation; the unit owner may correct signatures of its own functions.
#pragma once
#include "engine/core.h"

// 0x218-byte progress block of a kid, passed BY VALUE to FUN_0041b3b1 (the
// callers push 0x86 dwords). Global copy of the current kid at 0x4395f0; in
// the 0x329-byte kid records (P(0x4341d8)) it starts at offset 0x11.
struct ProgressRec {
    u32 unk00[2];                       // +0x00
    u32 level[10];                      // +0x08 current level per subject
    struct { u32 total, correct; } stats[10][6]; // +0x30 per subject, per level
    u32 unk210[2];                      // +0x210
};
static_assert(sizeof(ProgressRec) == 0x218, "ProgressRec layout");

void FUN_0041a8c8();                                            // m17
void FUN_0041a93d();                                            // m17
void FUN_0041a9ce();                                            // m17
void FUN_0041aaa7(int param_1);                                 // m17
void FUN_0041acc3();                                            // m17
void FUN_0041adeb();                                            // m17
void FUN_0041ae6b();                                            // m17
void FUN_0041af05();                                            // m17
void FUN_0041afdf();                                            // m17
int FUN_0041b054(int ev);                                       // m17 callback
void FUN_0041b069();                                            // m17
void FUN_0041b113();                                            // m17
void FUN_0041b38a();                                            // m17
void FUN_0041b3b1(const ProgressRec& rec);                      // m17 (struct by value)
void FUN_0041b48c(const void* rec);                             // m17 (kid name string)
void FUN_0041b734(char* param_1);                               // m17
void FUN_0041b78a(int param_1, int param_2, int param_3, int param_4); // m17
void FUN_0041b811();                                            // m17
void FUN_0041b8a1();                                            // m17
void FUN_0041b912();                                            // m17
