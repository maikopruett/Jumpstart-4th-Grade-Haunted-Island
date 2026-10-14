// File helpers (0x425434..0x4255a1). Paths under g_exeDir (the original's
// install directory) map to the per-user save directory; anything else is
// read from the game disc.
#pragma once
#include "engine/core.h"

extern char g_exeDir[260];   // 0x436f10, always ends with '/'

enum { FILE_READ = 0, FILE_CREATE = 1, FILE_APPEND = 2 };
int File_Open(const char* path, int mode);        // 0x425434, -1 on failure
void File_Close(int h);                           // 0x425484
bool File_Delete(const char* path);               // 0x425491
int File_Read(int h, void* buf, int n);           // 0x4254bc
int File_Write(int h, const void* buf, int n);    // 0x4254cf
int File_Seek(int h, int off, int whence);        // 0x4254e2
int File_Size(int h);                             // 0x4254f5
bool File_Exists(const char* path);               // 0x42552f
void* File_LoadAll(const char* path);             // 0x42555a
int File_Copy(const char* src, const char* dst, int confirm); // 0x4255a1
