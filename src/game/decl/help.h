// Declarations of unit "help": the toolbar help button (0x406274, missed by
// Ghidra) and the per-module help narrations it dispatches to.
#pragma once
#include "engine/core.h"

void FUN_00406274();          // help button (Bosco)
int FUN_00404d27();           // m02 location -> hint group
void FUN_00404e27(int first); // m02
void FUN_0040845a(int first); // m04
int FUN_00409f61(int first);  // m03
int FUN_0040bd66(int first);  // m07
int FUN_0040cefe(int first);  // m10
int FUN_0040f72a(int first);  // m08
void FUN_004114bb(int first); // m09
int FUN_00413324(int first);  // m06
void FUN_00414783(int first); // m05
int FUN_00418548(int first);  // m11
void FUN_00419995(int first); // m13
void FUN_0041a650(int first); // m14
void FUN_0041a87f(int first); // m15
void FUN_0041b9df(int first); // m17
int FUN_0041c5b5(int first);  // report
