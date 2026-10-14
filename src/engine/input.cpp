#include "engine/input.h"

#include <cstdio>
#include <map>
#include <string>

#include "engine/gfx.h"
#include "engine/hostfs.h"
#include "engine/module.h"
#include "engine/sound.h"
#include "engine/sys.h"

List g_hotspots;
List g_hotspotStack;
int g_mouseX, g_mouseY;
int g_userEvent[3];
bool g_appInactive = false;
int g_curCursor = 0;

static List g_events;          // 0x43b608
static bool g_swapButtons = false;
static u32 g_timers[16];       // 0x43b61c
static u32 g_stopwatchStart;   // 0x43b65c
static int g_stopwatchTotal;   // 0x43b660
static u32 g_pauseStart;       // 0x436f00
static CursorCallback g_cursorCallback;

// ---------------------------------------------------------------- hotspots

void Hotspot_Init() {
    List_Init(&g_hotspots, sizeof(Hotspot));
    List_Init(&g_hotspotStack, sizeof(List));
}

void Hotspot_Shutdown() {
    List_Destroy(&g_hotspotStack);
    List_Destroy(&g_hotspots);
}

static void TraceHotspot(const char* what, s16 id) {
    static bool trace = getenv("JS4_TRACE_HOTSPOT") != nullptr;
    if (!trace) return;
    fprintf(stderr, "[hotspot] %s %x  list:", what, id);
    for (ListNode* nd = g_hotspots.head->next; nd != g_hotspots.head; nd = nd->next)
        fprintf(stderr, " %x", ((Hotspot*)nd->data)->id);
    fprintf(stderr, "\n");
}

void Hotspot_Set(s16 id, int x, int y, int w, int h) {
    TraceHotspot("set", id);
    Hotspot* found = nullptr;
    for (List_First(&g_hotspots); !List_AtEnd(&g_hotspots); List_Next(&g_hotspots)) {
        Hotspot* hs = (Hotspot*)List_CurData(&g_hotspots);
        if (hs->id == id) {
            found = hs;
            break;
        }
    }
    if (found) {
        Rect_Set(&found->r, x, y, w, h);
    } else {
        Hotspot hs;
        hs.id = id;
        Rect_Set(&hs.r, x, y, w, h);
        List_PushFront(&g_hotspots, &hs);
    }
}

void Hotspot_Remove(s16 id) {
    TraceHotspot("remove", id);
    for (List_First(&g_hotspots); !List_AtEnd(&g_hotspots); List_Next(&g_hotspots)) {
        Hotspot* hs = (Hotspot*)List_CurData(&g_hotspots);
        if (hs->id == id) {
            List_RemoveCur(&g_hotspots);
            return;
        }
    }
}

void Hotspot_Clear() {
    TraceHotspot("clear", 0x20);
    List_Clear(&g_hotspots);
}

s16 Hotspot_HitTest(int x, int y) {
    for (List_First(&g_hotspots); !List_AtEnd(&g_hotspots); List_Next(&g_hotspots)) {
        Hotspot* hs = (Hotspot*)List_CurData(&g_hotspots);
        if (Pt_InRect(x, y, hs->r.x, hs->r.y, hs->r.w, hs->r.h)) return hs->id;
    }
    return 0;
}

// Save the current hotspot set on a stack (the list struct itself is copied).
void Hotspot_Push() {
    TraceHotspot("push", 0x20);
    List copy;
    List_Init(&copy, sizeof(Hotspot));
    Hotspot hs;
    for (List_First(&g_hotspots); !List_AtEnd(&g_hotspots); List_Next(&g_hotspots)) {
        List_GetCur(&g_hotspots, &hs);
        List_InsertAfter(&copy, &hs);
    }
    List_PushFront(&g_hotspotStack, &copy);
}

void Hotspot_Pop() {
    TraceHotspot("pop", 0x20);
    Hotspot_Clear();
    List_Destroy(&g_hotspots);
    if (!List_PopFront(&g_hotspotStack, &g_hotspots)) List_Init(&g_hotspots, sizeof(Hotspot));
    TraceHotspot("after-pop", 0x20);
}

void Hotspot_DebugDraw() {
    for (List_First(&g_hotspots); !List_AtEnd(&g_hotspots); List_Next(&g_hotspots)) {
        Hotspot* hs = (Hotspot*)List_CurData(&g_hotspots);
        Gfx_InvertRect(hs->r.x, hs->r.y, hs->r.w, hs->r.h);
        Screen_Flush(hs->r.x, hs->r.y, hs->r.w, hs->r.h);
    }
}

// ---------------------------------------------------------------- events

void Event_Init() {
    List_Init(&g_events, sizeof(GameEvent));
    g_swapButtons = false;
}

void Event_Shutdown() { List_Destroy(&g_events); }

static void Push(int type, int a) {
    GameEvent e = {type, a, 0, 0};
    List_PushBack(&g_events, &e);
}

// Windows virtual key codes -> the game's DOS-style key codes.
void Event_OnKeyDown(int vk, int mods) {
    bool ctrl = (mods & 2) != 0;
    int code = 0;
    switch (vk) {
        case 0x25: code = KEY_LEFT; break;
        case 0x27: code = KEY_RIGHT; break;
        case 0x26: code = KEY_UP; break;
        case 0x28: code = KEY_DOWN; break;
        case 0x21: code = KEY_PGUP; break;
        case 0x22: code = KEY_PGDN; break;
        case 0x24: code = KEY_HOME; break;
        case 0x23: code = KEY_END; break;
        case 0x2d: code = KEY_INS; break;
        case 0x2e: code = ctrl ? KEY_CTRL_DEL : KEY_DEL; break;
        case 0x08: if (ctrl) code = KEY_CTRL_DEL; break;
        default:
            if (vk >= 0x70 && vk <= 0x7b) code = KEY_F1 + (vk - 0x70);
            else if (vk >= 0x30 && vk <= 0x39 && ctrl) code = KEY_CTRL_0 + (vk - 0x30);
            break;
    }
    if (code) Push(1, code);
}

void Event_OnChar(int ch) { Push(1, ch & 0xffff); }

void Event_OnMouseButton(int msg, int x, int y) {
    int type = 0;
    switch (msg) {
        case 0x201: type = 2; break;
        case 0x202: type = 3; break;
        case 0x204: type = 6; break;
        case 0x205: type = 7; break;
        case 0x207: type = 4; break;
        case 0x208: type = 5; break;
    }
    Event_SetMousePos(x, y);
    s16 id = Hotspot_HitTest(g_mouseX, g_mouseY);
    static bool trace = getenv("JS4_TRACE_EVENTS") != nullptr;
    if (trace) {
        fprintf(stderr, "[mouse] %x at %d,%d -> hotspot %x; hotspots:", msg, x, y, id);
        for (List_First(&g_hotspots); !List_AtEnd(&g_hotspots); List_Next(&g_hotspots)) {
            Hotspot* hs = (Hotspot*)List_CurData(&g_hotspots);
            fprintf(stderr, " %x(%d,%d,%d,%d)", hs->id, hs->r.x, hs->r.y, hs->r.w, hs->r.h);
        }
        fprintf(stderr, "\n");
    }
    if (id != 0) Push(type, (u16)id);
}

void Event_SetMousePos(int x, int y) {
    g_mouseX = x;
    g_mouseY = y;
}

void Event_PostQuit() { Push(100, 0); }

void Event_PostUser(int a, int b, int c) {
    GameEvent e = {200, a, b, c};
    List_PushBack(&g_events, &e);
}

bool Event_Pump() {
    for (;;) {
        Sys_ProcessEvents();
        if (!g_appInactive || g_fastQuit) {
            Sound_Update();
            bool pending = !List_IsEmpty(&g_events);
            // The game polls in tight loops (e.g. while a sound plays); yield
            // a little when idle so it doesn't spin a core at 100%.
            if (!pending) Sys_Delay(1);
            return pending;
        }
        Sys_Delay(15);
    }
}

int Event_Wait() {
    while (!Event_Pump()) Sys_Delay(5);
    GameEvent e;
    List_PopFront(&g_events, &e);
    int code = e.a & 0xffff;
    static bool trace = getenv("JS4_TRACE_EVENTS") != nullptr;
    if (trace) fprintf(stderr, "[event] type %d code %x\n", e.type, code);
    switch (e.type) {
        case 1: return code;
        case 2: return code;
        case 3: return code | 0x8000;
        case 4: return code | 0x4000;
        case 5: return code | 0x2000;
        case 6: return code | 0x4000;
        case 7: return code | 0x2000;
        case 10: return code;
        case 100: App_Quit(); return 0;
        case 200:
            g_userEvent[0] = e.a;
            g_userEvent[1] = e.b;
            g_userEvent[2] = e.c;
            return EVT_USER;
    }
    return 0;
}

int Event_Poll() {
    if (!Event_Pump()) return 0;
    return Event_Wait();
}

void Event_Flush() {
    while ((s16)Event_Poll() != 0) {
    }
    List_Clear(&g_events);
}

void Mouse_GetPos(Point* p) {
    Sys_MousePos(&p->x, &p->y);
}

int Mouse_Buttons() {
    int b = Sys_MouseButtons();
    int r = 0;
    if (b & (g_swapButtons ? 4 : 1)) r |= 1;
    if (b & 2) r |= 2;
    if (b & (g_swapButtons ? 1 : 4)) r |= 4;
    return r;
}

int Key_Modifiers() { return Sys_KeyModifiers(); }

// ---------------------------------------------------------------- timers

u32 Time_Get() { return Sys_Time(); }

void Timer_Set(int slot, int ms) { g_timers[slot] = Time_Get() + ms; }
bool Timer_Expired(int slot) { return g_timers[slot] <= Time_Get(); }

void Timer_Wait(int slot) {
    while (Time_Get() < g_timers[slot]) {
    }
}

void Time_Delay(int ms) {
    u32 start = Time_Get();
    while (Time_Get() < start + (u32)ms) {
    }
}

void Stopwatch_Reset() {
    g_stopwatchTotal = 0;
    g_stopwatchStart = Time_Get();
}

void Stopwatch_Pause() { g_stopwatchTotal += (int)(Time_Get() - g_stopwatchStart); }
void Stopwatch_Resume() { g_stopwatchStart = Time_Get(); }

int Stopwatch_Get() {
    g_stopwatchTotal += (int)(Time_Get() - g_stopwatchStart);
    return g_stopwatchTotal;
}

void Time_PauseBegin() { g_pauseStart = Time_Get(); }

void Time_PauseEnd() {
    if (g_pauseStart != 0) {
        u32 d = Time_Get() - g_pauseStart;
        for (u32& t : g_timers) t += d;
    }
}

// ---------------------------------------------------------------- cursor

void Cursor_InitStd() {}
void Cursor_Load(const u16* ids) { (void)ids; }
void Cursor_Free() {}

int Cursor_Set(int index) {
    int prev = g_curCursor;
    g_curCursor = index;
    Sys_SetCursor(index);
    return prev;
}

void Cursor_Refresh() { Cursor_Set(g_curCursor); }

CursorCallback Cursor_SetCallback(CursorCallback cb) {
    CursorCallback prev = g_cursorCallback;
    g_cursorCallback = cb;
    return prev;
}

void Cursor_OnSetCursor() {
    if (g_cursorCallback) g_cursorCallback();
}

bool Cursor_Show(bool show) { return Sys_ShowCursor(show); }

void Mouse_Warp(int x, int y) { Sys_WarpMouse(x, y); }

// ---------------------------------------------------------------- settings

static std::map<std::string, std::string> g_ini;
static bool g_iniLoaded = false;

static void IniLoad() {
    if (g_iniLoaded) return;
    g_iniLoaded = true;
    FILE* f = Host_FOpen(Sys_PrefsPath(), "r");
    if (!f) return;
    char line[512];
    std::string section;
    while (fgets(line, sizeof line, f)) {
        std::string s = line;
        while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) s.pop_back();
        if (s.size() > 2 && s[0] == '[' && s.back() == ']') {
            section = s.substr(1, s.size() - 2);
            continue;
        }
        size_t eq = s.find('=');
        if (eq == std::string::npos) continue;
        g_ini[section + "/" + s.substr(0, eq)] = s.substr(eq + 1);
    }
    fclose(f);
}

static void IniSave() {
    FILE* f = Host_FOpen(Sys_PrefsPath(), "w");
    if (!f) return;
    std::string section;
    for (auto& kv : g_ini) {
        size_t slash = kv.first.find('/');
        std::string sec = kv.first.substr(0, slash);
        if (sec != section) {
            fprintf(f, "[%s]\n", sec.c_str());
            section = sec;
        }
        fprintf(f, "%s=%s\n", kv.first.substr(slash + 1).c_str(), kv.second.c_str());
    }
    fclose(f);
}

int Ini_GetInt(const char* section, const char* key) {
    IniLoad();
    auto it = g_ini.find(std::string(section) + "/" + key);
    if (it == g_ini.end()) return 0;
    return atoi(it->second.c_str());
}

void Ini_WriteInt(const char* section, const char* key, int value) {
    IniLoad();
    g_ini[std::string(section) + "/" + key] = std::to_string(value);
    IniSave();
}

void Ini_GetString(const char* section, const char* key, char* buf, int size) {
    IniLoad();
    auto it = g_ini.find(std::string(section) + "/" + key);
    snprintf(buf, size, "%s", it == g_ini.end() ? "" : it->second.c_str());
}

void Ini_WriteString(const char* section, const char* key, const char* v) {
    IniLoad();
    g_ini[std::string(section) + "/" + key] = v;
    IniSave();
}
