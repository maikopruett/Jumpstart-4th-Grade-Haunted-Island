// Movie playback (0x427504 / 0x427696 / 0x427884), built on smacker.h.
#pragma once
#include "engine/sprite.h"

extern char g_resDir[260];  // 0x437e54 ("" = root of the mounted disc)

// Plays name (relative to g_resDir) into the back buffer at (x, y).
// Returns 1 if abortCb returned nonzero for some event.
int Video_Play(const char* name, int x, int y, AbortCallback abortCb);        // 0x427504
// Plays a quarter-size movie doubled to (x, y) via the off-screen half.
int Video_PlayDoubled(const char* name, int x, int y, AbortCallback abortCb); // 0x427696
void Video_Close();                                                           // 0x427884
