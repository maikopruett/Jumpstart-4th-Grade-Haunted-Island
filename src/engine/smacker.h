// Minimal Smacker (SMK2/SMK4) video decoder replacing SMACKW32.DLL.
// Audio tracks are skipped: none of the game's movies carry audio.
#pragma once
#include "engine/core.h"

struct SmkFile;

SmkFile* Smk_Open(const char* path);
void Smk_Close(SmkFile* s);
int Smk_Width(const SmkFile* s);
int Smk_Height(const SmkFile* s);
int Smk_Frames(const SmkFile* s);
u32 Smk_FrameUsec(const SmkFile* s);
// Decodes the next frame. Returns false at the end of the file.
// *dirty receives the changed area; *palChanged is set when the palette changed.
bool Smk_NextFrame(SmkFile* s, Rect* dirty, bool* palChanged);
const u8* Smk_Pixels(const SmkFile* s);   // width * height palette indices
const u8* Smk_Palette(const SmkFile* s);  // 256 RGB triples
int Smk_CurrentFrame(const SmkFile* s);
