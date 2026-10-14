// Event queue, hotspots, timers and cursor (0x41dbdc..0x41e757).
#pragma once
#include "engine/core.h"

// Event codes returned by Event_Wait: a hotspot id (left button down),
// id|0x8000 (left up), id|0x4000 (right/middle down), id|0x2000 (up),
// a WM_CHAR code, or 0x13b..0x190 for special keys:
enum {
    KEY_F1 = 0x13b, KEY_F10 = 0x144, KEY_F11 = 0x145, KEY_F12 = 0x146,
    KEY_HOME = 0x147, KEY_UP = 0x148, KEY_PGUP = 0x149, KEY_LEFT = 0x14b,
    KEY_RIGHT = 0x14d, KEY_END = 0x14f, KEY_DOWN = 0x150, KEY_PGDN = 0x151,
    KEY_INS = 0x152, KEY_DEL = 0x153, KEY_CTRL_0 = 0x180, KEY_CTRL_DEL = 0x190,
    EVT_USER = 0xf800,
};

struct Hotspot {
    s16 id;
    Rect r;
};

struct GameEvent {
    int type;  // 1 key, 2..7 mouse buttons, 10 code, 100 quit, 200 user
    int a, b, c;
};

extern List g_hotspots;        // 0x43b5d4
extern List g_hotspotStack;    // 0x43b5e4
extern int g_mouseX, g_mouseY; // 0x43b5f4 / 0x43b5f8
extern int g_userEvent[3];     // 0x43b5fc

void Hotspot_Init();                                // 0x41dbdc
void Hotspot_Shutdown();                            // 0x41dbff
void Hotspot_Set(s16 id, int x, int y, int w, int h); // 0x41dc1a
static inline void Hotspot_Set(s16 id, const Rect& r) { Hotspot_Set(id, r.x, r.y, r.w, r.h); }
void Hotspot_Remove(s16 id);                        // 0x41dc98
void Hotspot_Clear();                               // 0x41dcdc
s16 Hotspot_HitTest(int x, int y);                  // 0x41dcec
void Hotspot_Push();                                // 0x41dd4e
void Hotspot_Pop();                                 // 0x41ddb6
void Hotspot_DebugDraw();                           // 0x41ddd2

void Event_Init();                                  // 0x41de2c
void Event_Shutdown();                              // 0x41de4c
void Event_OnKeyDown(int vk, int mods);             // 0x41de5c (vk = Win32 VK code)
void Event_OnChar(int ch);                          // 0x41e083
void Event_OnMouseButton(int msg, int x, int y);    // 0x41e0ad (msg = WM_xBUTTONxxx)
void Event_SetMousePos(int x, int y);               // 0x41e156
void Event_PostQuit();                              // 0x41e1be
void Event_PostUser(int a, int b, int c);           // 0x41e1e0
bool Event_Pump();                                  // 0x41e214: true if events queued
int Event_Wait();                                   // 0x41e27b
int Event_Poll();                                   // 0x41e355: 0 if nothing pending
void Event_Flush();                                 // 0x41e36c
void Mouse_GetPos(Point* p);                        // 0x41e386
int Mouse_Buttons();                                // 0x41e3c4
int Key_Modifiers();                                // 0x41e42d

// Called by the platform layer.
extern bool g_appInactive;  // 0x436db8

// ---- timers ----
u32 Time_Get();                                     // 0x41e4c8
void Timer_Set(int slot, int ms);                   // 0x41e4d4
bool Timer_Expired(int slot);                       // 0x41e4eb
void Timer_Wait(int slot);                          // 0x41e505
void Time_Delay(int ms);                            // 0x41e51d
void Stopwatch_Reset();                             // 0x41e537
void Stopwatch_Pause();                             // 0x41e54d
void Stopwatch_Resume();                            // 0x41e563
int Stopwatch_Get();                                // 0x41e572
void Time_PauseBegin();                             // 0x41e58d
void Time_PauseEnd();                               // 0x41e59c

// ---- cursor ----
enum { CURSOR_ARROW = 0, CURSOR_WAIT = 1 };
typedef void (*CursorCallback)();
void Cursor_InitStd();                              // 0x41e5c8
void Cursor_Load(const u16* resIds);                // 0x41e5ef (0-terminated, max 14)
void Cursor_Free();                                 // 0x41e637
int Cursor_Set(int index);                          // 0x41e663, returns previous
void Cursor_Refresh();                              // 0x41e68c
CursorCallback Cursor_SetCallback(CursorCallback cb); // 0x41e69f
void Cursor_OnSetCursor();                          // 0x41e6b2 (WM_SETCURSOR)
bool Cursor_Show(bool show);                        // 0x41e6c6
void Mouse_Warp(int x, int y);                      // 0x41e757
extern int g_curCursor;                             // 0x436f04

// ---- configuration (replaces 4G.INI access) ----
int Ini_GetInt(const char* section, const char* key);                     // 0x41e848
void Ini_WriteInt(const char* section, const char* key, int value);       // 0x41e8ba
void Ini_GetString(const char* section, const char* key, char* buf, int size); // 0x41e904
void Ini_WriteString(const char* section, const char* key, const char* v); // 0x41e93b
