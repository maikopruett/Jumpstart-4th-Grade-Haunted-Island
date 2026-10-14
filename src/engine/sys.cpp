#include "engine/sys.h"

#include <SDL3/SDL.h>

#include <cstdio>
#include <string>
#include <vector>

#include "engine/exeres.h"
#include "engine/hostfs.h"
#include "engine/input.h"
#include "engine/sound.h"

namespace {

SDL_Window* g_window;
SDL_Renderer* g_renderer;
SDL_Texture* g_texture;
u8 g_display[kScreenW * kScreenH];
u8 g_displayPal[768];
bool g_displayDirty = true;
u64 g_lastPresent;
u64 g_lastPump;
u32 g_lastTime;
int g_sameTimeCalls;
bool g_quitPosted;
u64 g_fastQuitStart;
int g_mouseLX, g_mouseLY;
int g_buttons;
bool g_cursorVisible = true;
bool g_fullscreen = false;

// ---- debug automation (JS4_SCRIPT / JS4_SNAP_DIR) for headless testing
struct ScriptCmd {
    std::string op;
    int a = 0, b = 0;
    std::string text;
};
std::vector<ScriptCmd> g_script;
size_t g_scriptPos = 0;
u64 g_scriptDue = 0;
std::string g_snapDir;

void LoadScript() {
    if (const char* d = SDL_getenv("JS4_SNAP_DIR")) g_snapDir = std::string(d) + "/";
    const char* path = SDL_getenv("JS4_SCRIPT");
    if (!path) return;
    FILE* f = Host_FOpen(path, "r");
    if (!f) return;
    char line[512];
    while (fgets(line, sizeof line, f)) {
        char op[64] = {0}, rest[400] = {0};
        if (sscanf(line, "%63s %399[^\n]", op, rest) < 1 || op[0] == '#') continue;
        ScriptCmd c;
        c.op = op;
        c.text = rest;
        sscanf(rest, "%d %d", &c.a, &c.b);
        g_script.push_back(c);
    }
    fclose(f);
}

void Snapshot(const std::string& name) {
    SDL_Surface* s = SDL_CreateSurface(kScreenW, kScreenH, SDL_PIXELFORMAT_XRGB8888);
    if (!s) return;
    for (int y = 0; y < kScreenH; y++) {
        u32* row = (u32*)((u8*)s->pixels + y * s->pitch);
        for (int x = 0; x < kScreenW; x++) {
            const u8* c = g_displayPal + g_display[y * kScreenW + x] * 3;
            row[x] = 0xff000000u | (c[0] << 16) | (c[1] << 8) | c[2];
        }
    }
    std::string path = g_snapDir + name + ".png";
    SDL_SavePNG(s, path.c_str());
    SDL_DestroySurface(s);
    fprintf(stderr, "[script] snap %s\n", path.c_str());
}

void HandleEvent(const SDL_Event& e);
void RequestQuit();

void RunScript() {
    u64 now = SDL_GetTicks();
    while (g_scriptPos < g_script.size() && now >= g_scriptDue) {
        const ScriptCmd& c = g_script[g_scriptPos++];
        if (c.op == "wait") {
            g_scriptDue = now + c.a;
        } else if (c.op == "click" || c.op == "rclick") {
            g_mouseLX = c.a;
            g_mouseLY = c.b;
            Event_SetMousePos(c.a, c.b);
            Cursor_OnSetCursor();
            bool right = c.op == "rclick";
            Event_OnMouseButton(right ? 0x204 : 0x201, c.a, c.b);
            Event_OnMouseButton(right ? 0x205 : 0x202, c.a, c.b);
            if (!SDL_getenv("JS4_QUIET")) fprintf(stderr, "[script] %s %d %d\n", c.op.c_str(), c.a, c.b);
        } else if (c.op == "move") {
            g_mouseLX = c.a;
            g_mouseLY = c.b;
            Event_SetMousePos(c.a, c.b);
            Cursor_OnSetCursor();
        } else if (c.op == "key") {
            Event_OnKeyDown(c.a, 0);
        } else if (c.op == "char") {
            Event_OnChar(c.a);
        } else if (c.op == "type") {
            for (char ch : c.text) Event_OnChar((u8)ch);
        } else if (c.op == "snap") {
            Snapshot(c.text);
        } else if (c.op == "quit") {
            Event_PostQuit();
        } else if (c.op == "exit") {
            fprintf(stderr, "[script] exit\n");
            exit(0);
        } else if (c.op == "fuzz") {
            // fuzz COUNT INTERVAL: random clicks (and some keys) inserted here.
            static u32 seed = SDL_getenv("JS4_FUZZ_SEED") ? (u32)atoi(SDL_getenv("JS4_FUZZ_SEED")) : 12345u;
            std::vector<ScriptCmd> extra;
            for (int i = 0; i < c.a; i++) {
                seed = seed * 1103515245u + 12345u;
                u32 r = seed >> 8;
                ScriptCmd k;
                if (r % 23 == 0) {
                    static const int keys[] = {0x0d, 0x1b, 0x20, 'a', 'e', '7', 0x08};
                    k.op = "char";
                    k.a = keys[(r / 23) % 7];
                } else {
                    k.op = (r % 17 == 0) ? "rclick" : "click";
                    k.a = (int)(r % 640);
                    k.b = (int)((r / 640) % 480);
                }
                extra.push_back(k);
                ScriptCmd w;
                w.op = "wait";
                w.a = c.b;
                extra.push_back(w);
            }
            g_script.insert(g_script.begin() + g_scriptPos, extra.begin(), extra.end());
            break;
        } else if (c.op == "log") {
            fprintf(stderr, "[script] %s\n", c.text.c_str());
        }
    }
}

// Cursors: 0 arrow, 1 wait, 2.. game cursors (resource ids 101..104).
std::vector<SDL_Cursor*> g_cursors;
int g_cursorScale = 0;

SDL_Cursor* MakeGameCursor(int resId, int scale) {
    ExeCursor c;
    if (!ExeRes_LoadCursor(resId, &c)) return nullptr;
    SDL_Surface* s = SDL_CreateSurface(c.w * scale, c.h * scale, SDL_PIXELFORMAT_ARGB8888);
    if (!s) return nullptr;
    for (int y = 0; y < c.h * scale; y++) {
        u32* row = (u32*)((u8*)s->pixels + y * s->pitch);
        for (int x = 0; x < c.w * scale; x++) row[x] = c.argb[(y / scale) * c.w + (x / scale)];
    }
    SDL_Cursor* cur = SDL_CreateColorCursor(s, c.hotX * scale, c.hotY * scale);
    SDL_DestroySurface(s);
    return cur;
}

int CurrentScale() {
    int w = 0, h = 0;
    SDL_GetWindowSize(g_window, &w, &h);
    int s = w / kScreenW < h / kScreenH ? w / kScreenW : h / kScreenH;
    if (s < 1) s = 1;
    if (s > 4) s = 4;
    return s;
}

void RebuildCursors() {
    int scale = CurrentScale();
    if (scale == g_cursorScale && !g_cursors.empty()) return;
    g_cursorScale = scale;
    for (size_t i = 2; i < g_cursors.size(); i++)
        if (g_cursors[i]) SDL_DestroyCursor(g_cursors[i]);
    g_cursors.resize(2);
    if (!g_cursors[0]) g_cursors[0] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
    if (!g_cursors[1]) g_cursors[1] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
    for (int id = 101; id <= 104; id++) g_cursors.push_back(MakeGameCursor(id, scale));
    Sys_SetCursor(g_curCursor);
}

int WinKeyFromSdl(SDL_Keycode k) {
    switch (k) {
        case SDLK_LEFT: return 0x25;
        case SDLK_UP: return 0x26;
        case SDLK_RIGHT: return 0x27;
        case SDLK_DOWN: return 0x28;
        case SDLK_PAGEUP: return 0x21;
        case SDLK_PAGEDOWN: return 0x22;
        case SDLK_END: return 0x23;
        case SDLK_HOME: return 0x24;
        case SDLK_INSERT: return 0x2d;
        case SDLK_DELETE: return 0x2e;
        case SDLK_BACKSPACE: return 0x08;
        default: break;
    }
    if (k >= SDLK_F1 && k <= SDLK_F12) return 0x70 + (int)(k - SDLK_F1);
    if (k >= SDLK_0 && k <= SDLK_9) return 0x30 + (int)(k - SDLK_0);
    return 0;
}

void ToggleFullscreen() {
    g_fullscreen = !g_fullscreen;
    SDL_SetWindowFullscreen(g_window, g_fullscreen);
    Ini_WriteInt("Window", "Fullscreen", g_fullscreen ? 1 : 2);
}

void HandleEvent(const SDL_Event& e) {
    if (!g_script.empty() && (e.type == SDL_EVENT_WINDOW_FOCUS_LOST || e.type == SDL_EVENT_MOUSE_MOTION ||
                              e.type == SDL_EVENT_MOUSE_BUTTON_DOWN || e.type == SDL_EVENT_MOUSE_BUTTON_UP))
        return; // scripted runs ignore the real mouse and focus
    switch (e.type) {
        case SDL_EVENT_QUIT:
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            RequestQuit();
            break;
        case SDL_EVENT_WINDOW_FOCUS_LOST:
            if (!g_appInactive) {
                g_appInactive = true;
                Time_PauseBegin();
                Sound_Pause();
            }
            break;
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
            if (g_appInactive) {
                g_appInactive = false;
                Time_PauseEnd();
                Sound_Resume();
            }
            break;
        case SDL_EVENT_WINDOW_RESIZED:
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        case SDL_EVENT_WINDOW_EXPOSED:
            g_displayDirty = true;
            RebuildCursors();
            break;
        case SDL_EVENT_MOUSE_MOTION: {
            SDL_Event c = e;
            SDL_ConvertEventToRenderCoordinates(g_renderer, &c);
            g_mouseLX = (int)c.motion.x;
            g_mouseLY = (int)c.motion.y;
            Event_SetMousePos(g_mouseLX, g_mouseLY);
            Cursor_OnSetCursor();
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            SDL_Event c = e;
            SDL_ConvertEventToRenderCoordinates(g_renderer, &c);
            g_mouseLX = (int)c.button.x;
            g_mouseLY = (int)c.button.y;
            bool down = e.type == SDL_EVENT_MOUSE_BUTTON_DOWN;
            int bit = 0, msg = 0;
            if (e.button.button == SDL_BUTTON_LEFT) {
                bit = 1;
                msg = down ? 0x201 : 0x202;
            } else if (e.button.button == SDL_BUTTON_RIGHT) {
                bit = 4;
                msg = down ? 0x204 : 0x205;
            } else if (e.button.button == SDL_BUTTON_MIDDLE) {
                bit = 2;
                msg = down ? 0x207 : 0x208;
            }
            if (down) g_buttons |= bit;
            else g_buttons &= ~bit;
            if (msg) Event_OnMouseButton(msg, g_mouseLX, g_mouseLY);
            break;
        }
        case SDL_EVENT_KEY_DOWN: {
            SDL_Keycode k = e.key.key;
            SDL_Keymod m = e.key.mod;
#if defined(__APPLE__)
            bool cmd = (m & SDL_KMOD_GUI) != 0; // Command-key shortcuts (macOS only)
            bool f11 = false;
#else
            bool cmd = false;
            bool f11 = k == SDLK_F11;
#endif
            bool alt = (m & SDL_KMOD_ALT) != 0;
            if ((cmd && k == SDLK_F) || (alt && k == SDLK_RETURN) || (cmd && k == SDLK_RETURN) || f11) {
                ToggleFullscreen();
                break;
            }
            if (cmd && k == SDLK_Q) {
                RequestQuit();
                break;
            }
            int mods = Sys_KeyModifiers();
            if (cmd) mods |= 2; // treat Command like Control for shortcuts
            int vk = WinKeyFromSdl(k);
            if (vk) Event_OnKeyDown(vk, mods);
            // WM_CHAR equivalents for control characters.
            if (k == SDLK_RETURN || k == SDLK_KP_ENTER) Event_OnChar(0x0d);
            else if (k == SDLK_ESCAPE) Event_OnChar(0x1b);
            else if (k == SDLK_BACKSPACE) Event_OnChar(0x08);
            else if (k == SDLK_TAB) Event_OnChar(0x09);
            else if ((mods & 2) && k >= SDLK_A && k <= SDLK_Z) Event_OnChar((int)(k - SDLK_A) + 1);
            break;
        }
        case SDL_EVENT_TEXT_INPUT: {
            if (Sys_KeyModifiers() & 2) break;
            const unsigned char* t = (const unsigned char*)e.text.text;
            while (*t) {
                u32 cp;
                if (*t < 0x80) cp = *t++;
                else if ((*t & 0xe0) == 0xc0 && t[1]) {
                    cp = ((t[0] & 0x1f) << 6) | (t[1] & 0x3f);
                    t += 2;
                } else {
                    while (*t & 0x80) t++;
                    continue;
                }
                if (cp >= 0x20 && cp < 0x100 && cp != 0x7f) Event_OnChar((int)cp);
            }
            break;
        }
        default:
            break;
    }
}

void RequestQuit() {
    if (g_quitPosted) return;
    g_quitPosted = true;
    g_fastQuit = true;
    g_fastQuitStart = SDL_GetTicks();
    SDL_HideWindow(g_window);
    Event_PostQuit();
}

} // namespace

bool g_fastQuit = false;

bool Sys_Init() {
    SDL_SetAppMetadata("JumpStart 4th Grade: Haunted Island", "1.0", "com.maikopruett.jumpstart4");
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS)) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }
    // Largest quarter-step scale (up to 3x) that fits the usable desktop.
    float scale = 2.0f;
    SDL_Rect usable;
    if (SDL_GetDisplayUsableBounds(SDL_GetPrimaryDisplay(), &usable)) {
        float fit = (float)usable.w / kScreenW < (float)(usable.h - 32) / kScreenH ? (float)usable.w / kScreenW
                                                                                : (float)(usable.h - 32) / kScreenH;
        scale = (float)(int)(fit * 4) / 4.0f;
        if (scale > 3.0f) scale = 3.0f;
        if (scale < 1.0f) scale = 1.0f;
    }
    g_fullscreen = Ini_GetInt("Window", "Fullscreen") == 1;
    g_window = SDL_CreateWindow("JumpStart 4th Grade: Haunted Island", (int)(kScreenW * scale), (int)(kScreenH * scale),
                                SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY |
                                    (g_fullscreen ? SDL_WINDOW_FULLSCREEN : 0));
    if (!g_window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }
    SDL_SetWindowMinimumSize(g_window, kScreenW, kScreenH);
    ExeCursor icon;
    if (ExeRes_LoadIcon("OURICONA32", &icon)) {
        // Scale the 32x32 icon up with nearest-neighbour so the Dock shows it crisply.
        const int k = 8;
        if (SDL_Surface* s = SDL_CreateSurface(icon.w * k, icon.h * k, SDL_PIXELFORMAT_ARGB8888)) {
            for (int y = 0; y < icon.h * k; y++) {
                u32* row = (u32*)((u8*)s->pixels + y * s->pitch);
                for (int x = 0; x < icon.w * k; x++) row[x] = icon.argb[(y / k) * icon.w + x / k];
            }
            SDL_SetWindowIcon(g_window, s);
            SDL_DestroySurface(s);
        }
    }
    g_renderer = SDL_CreateRenderer(g_window, nullptr);
    if (!g_renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return false;
    }
    SDL_SetRenderVSync(g_renderer, 0);
    SDL_SetRenderLogicalPresentation(g_renderer, kScreenW, kScreenH, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    g_texture = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING, kScreenW,
                                  kScreenH);
    SDL_SetTextureScaleMode(g_texture, SDL_SCALEMODE_PIXELART);
    SDL_StartTextInput(g_window);
    g_cursors.resize(2, nullptr);
    RebuildCursors();
    memset(g_display, 0, sizeof g_display);
    memset(g_displayPal, 0, sizeof g_displayPal);
    LoadScript();
    g_scriptDue = SDL_GetTicks();
    Sys_Present(true);
    return true;
}

void Sys_Shutdown() {
    for (SDL_Cursor* c : g_cursors)
        if (c) SDL_DestroyCursor(c);
    g_cursors.clear();
    if (g_texture) SDL_DestroyTexture(g_texture);
    if (g_renderer) SDL_DestroyRenderer(g_renderer);
    if (g_window) SDL_DestroyWindow(g_window);
    SDL_Quit();
}

u8* Sys_Display() { return g_display; }
void Sys_DisplayChanged() { g_displayDirty = true; }

void Sys_SetDisplayPalette(const u8* rgb) {
    memcpy(g_displayPal, rgb, 768);
    g_displayDirty = true;
}

void Sys_Present(bool force) {
    u64 now = SDL_GetTicksNS();
    if (!force && (!g_displayDirty || now - g_lastPresent < 8000000ull)) return;
    g_lastPresent = now;
    g_displayDirty = false;
    void* pixels;
    int pitch;
    if (SDL_LockTexture(g_texture, nullptr, &pixels, &pitch)) {
        u32 lut[256];
        for (int i = 0; i < 256; i++)
            lut[i] = 0xff000000u | (g_displayPal[i * 3] << 16) | (g_displayPal[i * 3 + 1] << 8) | g_displayPal[i * 3 + 2];
        for (int y = 0; y < kScreenH; y++) {
            u32* row = (u32*)((u8*)pixels + y * pitch);
            const u8* src = g_display + y * kScreenW;
            for (int x = 0; x < kScreenW; x++) row[x] = lut[src[x]];
        }
        SDL_UnlockTexture(g_texture);
    }
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
    SDL_RenderClear(g_renderer);
    SDL_RenderTexture(g_renderer, g_texture, nullptr, nullptr);
    SDL_RenderPresent(g_renderer);
}

void Sys_ProcessEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) HandleEvent(e);
    if (!g_script.empty()) RunScript();
    if (g_fastQuit && SDL_GetTicks() - g_fastQuitStart > 8000) {
        fprintf(stderr, "quit sequence took too long; exiting\n");
        exit(0);
    }
    g_lastPump = SDL_GetTicksNS();
    Sys_Present(false);
}

u32 Sys_Time() {
    u32 t = (u32)SDL_GetTicks();
    u64 now = SDL_GetTicksNS();
    // Keep the OS happy during the game's busy-wait loops: let macOS process
    // its event queue (input stays queued for the game) and show progress.
    if (now - g_lastPump > 4000000ull) {
        SDL_PumpEvents();
        g_lastPump = now;
        Sys_Present(false);
    }
    if (t == g_lastTime) {
        if (++g_sameTimeCalls > 4) {
            SDL_Delay(1);
            g_sameTimeCalls = 0;
            t = (u32)SDL_GetTicks();
        }
    } else {
        g_sameTimeCalls = 0;
    }
    g_lastTime = t;
    return t;
}

void Sys_MousePos(int* x, int* y) {
    *x = g_mouseLX;
    *y = g_mouseLY;
}

int Sys_MouseButtons() {
    SDL_PumpEvents();
    SDL_MouseButtonFlags f = SDL_GetMouseState(nullptr, nullptr);
    int b = 0;
    if (f & SDL_BUTTON_LMASK) b |= 1;
    if (f & SDL_BUTTON_MMASK) b |= 2;
    if (f & SDL_BUTTON_RMASK) b |= 4;
    return b;
}

int Sys_KeyModifiers() {
    SDL_Keymod m = SDL_GetModState();
    int r = 0;
    if (m & SDL_KMOD_SHIFT) r |= 1;
    if (m & SDL_KMOD_CTRL) r |= 2;
    if (m & SDL_KMOD_ALT) r |= 4;
    if (m & SDL_KMOD_CAPS) r |= 0x100;
    if (m & SDL_KMOD_NUM) r |= 0x200;
    return r;
}

void Sys_WarpMouse(int x, int y) {
    float wx, wy;
    SDL_RenderCoordinatesToWindow(g_renderer, (float)x, (float)y, &wx, &wy);
    SDL_WarpMouseInWindow(g_window, wx, wy);
    g_mouseLX = x;
    g_mouseLY = y;
}

void Sys_SetCursor(int index) {
    if (index < 0 || index >= (int)g_cursors.size() || !g_cursors[index]) index = 0;
    if (g_cursors[index]) SDL_SetCursor(g_cursors[index]);
}

bool Sys_ShowCursor(bool show) {
    bool prev = g_cursorVisible;
    g_cursorVisible = show;
    if (show) SDL_ShowCursor();
    else SDL_HideCursor();
    return prev;
}

bool Sys_AppActive() { return !g_appInactive; }
void Sys_Delay(u32 ms) { SDL_Delay(ms); }

std::string Sys_SaveDir() {
    static std::string dir;
    if (dir.empty() && SDL_getenv("JS4_SAVE_DIR")) {
        dir = SDL_getenv("JS4_SAVE_DIR");
        if (!dir.empty() && dir.back() != '/') dir += '/';
    }
    if (dir.empty()) {
        char* p = SDL_GetPrefPath("Knowledge Adventure", "JumpStart 4th Grade");
        dir = p ? p : "./";
        SDL_free(p);
    }
    return dir;
}

std::string Sys_PrefsPath() { return Sys_SaveDir() + "4G.INI"; }

void Sys_MessageBox(const char* title, const char* text) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title, text, g_window);
}

bool Sys_YesNo(const char* title, const char* text) {
    const SDL_MessageBoxButtonData buttons[] = {
        {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "No"},
        {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Yes"},
    };
    SDL_MessageBoxData data = {SDL_MESSAGEBOX_INFORMATION, g_window, title, text, 2, buttons, nullptr};
    int id = 0;
    SDL_ShowMessageBox(&data, &id);
    return id == 1;
}

bool Sys_InputBox(const char*, const char*, char*, int) { return false; }

void Sys_SetClipboardText(const char* text) { SDL_SetClipboardText(text); }

char* Sys_GetClipboardText() {
    char* t = SDL_GetClipboardText();
    if (!t) return nullptr;
    size_t n = strlen(t);
    char* out = (char*)Mem_Alloc(n + 1);
    memcpy(out, t, n + 1);
    SDL_free(t);
    return out;
}
