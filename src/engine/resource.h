// Knowledge Adventure .DF resource archives (0x427140..0x427426, 0x425980).
#pragma once
#include "engine/core.h"

bool Res_OpenArchive(const char* path);   // 0x427140
void Res_CloseAll();                      // 0x427282
// Loads a resource into a fresh buffer (NUL-terminated). Returns null if the
// id is not present. Free with Res_Free.
void* Res_Load(u32 id);                   // 0x4272c4
void Res_Free(const void* p);             // 0x427417
int Res_LastSize();                       // 0x427426

// 'COMP' container decompression (0x425980). If src isn't compressed it is
// copied. *dst may be preallocated; dstOffset bytes are reserved in front.
u32 Res_Decompress(const u8* src, u32 srcSize, u8** dst, int dstOffset);
