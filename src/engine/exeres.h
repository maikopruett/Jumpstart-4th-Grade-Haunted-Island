// Access to the original 4G.EXE on the game disc: its Win32 resources
// (cursors) and its initialized data section, so static game tables are read
// from the user's copy instead of being duplicated in this source tree.
#pragma once
#include <vector>

#include "engine/core.h"

bool ExeRes_Load();  // reads SUPPORT\CMN\4G\4G.EXE from the mounted disc

struct ExeCursor {
    int w = 0, h = 0, hotX = 0, hotY = 0;
    std::vector<u32> argb;
};
bool ExeRes_LoadCursor(int groupId, ExeCursor* out);
// Largest image of a named RT_GROUP_ICON (e.g. "OURICONA32"), as ARGB.
bool ExeRes_LoadIcon(const char* groupName, ExeCursor* out);

// Pointer to the loaded image at a virtual address (0x42f000.. DATA etc).
const u8* Exe_VA(u32 va);
template <typename T>
static inline const T* Exe_Ptr(u32 va) { return (const T*)Exe_VA(va); }
static inline const char* Exe_Str(u32 va) { return (const char*)Exe_VA(va); }
static inline s32 Exe_I32(u32 va) { return RdS32(Exe_VA(va)); }
static inline s16 Exe_I16(u32 va) { return RdS16(Exe_VA(va)); }
static inline u8 Exe_U8(u32 va) { return *Exe_VA(va); }
