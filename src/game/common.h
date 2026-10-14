// Shared include for ported game code: engine API, emulated data segment and
// declarations of all game functions.
#pragma once
#include <cctype>
#include <cstdio>
#include <cstring>
#include <ctime>

#include <SDL3/SDL_stdinc.h>

#include "engine/engine.h"
#include "game/funcs.h"
#include "game/seg.h"

// Borland toupper(): ASCII letters only.
static inline int Util_ToUpper(int c) { return (c >= 'a' && c <= 'z') ? c - 0x20 : c; }
