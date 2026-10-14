// Umbrella header for the reimplemented Knowledge Adventure engine.
#pragma once
#include "engine/core.h"
#include "engine/exeres.h"
#include "engine/file.h"
#include "engine/gfx.h"
#include "engine/input.h"
#include "engine/module.h"
#include "engine/resource.h"
#include "engine/sound.h"
#include "engine/sprite.h"
#include "engine/sys.h"
#include "engine/text.h"
#include "engine/video.h"

bool App_InitEngine();     // 0x41cf9d
[[noreturn]] void Fatal_Error(const char* text); // 0x41d220
void App_ShutdownEngine(); // 0x41d1d5
