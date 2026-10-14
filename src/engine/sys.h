// Platform layer: window, 8-bit display emulation, timing and OS events.
// Replaces the Win32/WinG code of 4G.EXE (MainWndProc, 0x41cbbc, 0x41f3b5...).
#pragma once
#include <string>

#include "engine/core.h"

enum { kScreenW = 640, kScreenH = 480 };

bool Sys_Init();
void Sys_Shutdown();

// timeGetTime() replacement (0x41e4c8). Also keeps the window alive and
// presents pending display changes, so the game's busy-wait loops work.
u32 Sys_Time();

// Fetch OS events and feed them to the input layer (PeekMessage loop).
void Sys_ProcessEvents();

// The emulated 8-bit display: 640x480 palette indices + hardware palette.
u8* Sys_Display();
void Sys_DisplayChanged();
void Sys_SetDisplayPalette(const u8* rgb768);
void Sys_Present(bool force);

// Mouse/keyboard state queries (GetAsyncKeyState / GetKeyState).
void Sys_MousePos(int* x, int* y);
int Sys_MouseButtons();   // bit0 left, bit1 middle, bit2 right
int Sys_KeyModifiers();   // bit0 shift, bit1 ctrl, bit2 alt
void Sys_WarpMouse(int x, int y);

// Cursors: 0 arrow, 1 wait, 2+ the game's resource cursors (101..104).
void Sys_SetCursor(int index);
bool Sys_ShowCursor(bool show);

bool Sys_AppActive();

// Set when the user closes the window / quits from the OS. The game's own
// quit sequence (saving the roster, transitions, credits) still runs, but
// hidden and without waiting for movies, fades or narration.
extern bool g_fastQuit;
void Sys_Delay(u32 ms);

std::string Sys_SaveDir();      // writable per-user directory, with trailing '/'
std::string Sys_PrefsPath();    // INI replacement

// Native modal dialogs.
void Sys_MessageBox(const char* title, const char* text);
bool Sys_YesNo(const char* title, const char* text);
bool Sys_InputBox(const char* title, const char* prompt, char* buf, int bufSize);

// Clipboard (text edit control).
void Sys_SetClipboardText(const char* text);
char* Sys_GetClipboardText(); // Mem_Alloc'd or null
