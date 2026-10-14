#include "engine/video.h"

#include <cstdio>

#include "engine/gfx.h"
#include "engine/input.h"
#include "engine/smacker.h"
#include "engine/sys.h"

char g_resDir[260];

namespace {

SmkFile* g_smk;          // 0x437f3c
u8 g_remap[256];
u64 g_nextFrameUs;

// SmackColorRemap: map the movie palette onto the current game palette.
void BuildRemap(const u8* vpal) {
    const u8* gp = g_palette;
    if (!gp) {
        for (int i = 0; i < 256; i++) g_remap[i] = (u8)i;
        return;
    }
    for (int i = 0; i < 256; i++) {
        const u8* c = vpal + i * 3;
        if (memcmp(c, gp + i * 3, 3) == 0) {
            g_remap[i] = (u8)i;
            continue;
        }
        int best = 0, bestD = 1 << 30;
        for (int j = 0; j < 256; j++) {
            int dr = c[0] - gp[j * 3], dg = c[1] - gp[j * 3 + 1], db = c[2] - gp[j * 3 + 2];
            int dd = dr * dr * 30 + dg * dg * 59 + db * db * 11;
            if (dd < bestD) {
                bestD = dd;
                best = j;
                if (dd == 0) break;
            }
        }
        g_remap[i] = (u8)best;
    }
}

// SmackToBuffer + SmackToBufferRect: copy the changed area into the back
// buffer at (bx, by). Returns false when there is nothing to copy.
bool CopyFrame(int bx, int by, Rect* out) {
    Rect r;
    bool palChanged;
    if (!Smk_NextFrame(g_smk, &r, &palChanged)) return false;
    int w = Smk_Width(g_smk), h = Smk_Height(g_smk);
    if (palChanged) BuildRemap(Smk_Palette(g_smk));
    if (palChanged || Smk_CurrentFrame(g_smk) == 0) r = {0, 0, w, h};
    if (r.w <= 0 || r.h <= 0) {
        *out = {0, 0, 0, 0};
        return true;
    }
    const u8* src = Smk_Pixels(g_smk);
    u8* dst = g_screenSurf.bits;
    int pitch = g_screenSurf.rect.w;
    for (int y = 0; y < r.h; y++) {
        int sy = r.y + y, dy = by + r.y + y;
        if (dy < 0 || dy >= g_screenSurf.rect.h) continue;
        for (int x = 0; x < r.w; x++) {
            int dx = bx + r.x + x;
            if (dx < 0 || dx >= pitch) continue;
            dst[dy * pitch + dx] = g_remap[src[sy * w + r.x + x]];
        }
    }
    *out = {bx + r.x, by + r.y, r.w, r.h};
    return true;
}

u64 NowUs() { return (u64)Sys_Time() * 1000; }

// SmackWait: true while the next frame isn't due yet.
bool Wait() {
    u64 now = NowUs();
    if (now >= g_nextFrameUs) return false;
    if (g_nextFrameUs - now > 2000) Sys_Delay(1);
    return true;
}

SmkFile* Open(const char* name) {
    char path[600];
    snprintf(path, sizeof path, "%s%s", g_resDir, name);
    SmkFile* s = Smk_Open(path);
    if (!s) fprintf(stderr, "movie missing: %s\n", path);
    return s;
}

} // namespace

int Video_Play(const char* name, int x, int y, AbortCallback abortCb) {
    int aborted = 0;
    if (g_fastQuit) return 1;
    if (g_smk) {
        Smk_Close(g_smk);
        g_smk = nullptr;
    }
    g_smk = Open(name);
    if (!g_smk) return 0;
    BuildRemap(Smk_Palette(g_smk));
    g_nextFrameUs = NowUs();
    bool done = false;
    while (!done) {
        int ev = Event_Poll();
        if (abortCb && abortCb((u16)ev)) {
            aborted = 1;
            break;
        }
        if (Wait()) continue;
        Rect r;
        if (CopyFrame(x, y, &r)) {
            if (r.w > 0) {
                Spr_InvalidateAll();
                Spr_Update(0);
                Screen_Flush(r);
            }
        }
        if (Smk_CurrentFrame(g_smk) >= Smk_Frames(g_smk) - 1) done = true;
        else g_nextFrameUs += Smk_FrameUsec(g_smk);
    }
    Smk_Close(g_smk);
    g_smk = nullptr;
    return aborted;
}

int Video_PlayDoubled(const char* name, int x, int y, AbortCallback abortCb) {
    int aborted = 0;
    if (g_fastQuit) return 1;
    if (g_smk) {
        Smk_Close(g_smk);
        g_smk = nullptr;
    }
    g_smk = Open(name);
    if (!g_smk) return 0;
    BuildRemap(Smk_Palette(g_smk));
    g_nextFrameUs = NowUs();
    bool done = false;
    while (!done) {
        int ev = Event_Poll();
        if (abortCb && abortCb((u16)ev)) {
            aborted = 1;
            break;
        }
        if (Wait()) continue;
        Rect r;
        if (CopyFrame(0, g_screenH, &r) && r.w > 0) {
            int dx = r.x * 2 + x;
            int dy = (r.y - g_screenH) * 2 + y;
            Gfx_Copy2x(r.x, r.y, r.w, r.h, dx, dy);
            Spr_InvalidateAll();
            Spr_Update(0);
            Screen_Flush(dx, dy, r.w * 2, r.h * 2);
        }
        if (Smk_CurrentFrame(g_smk) >= Smk_Frames(g_smk) - 1) done = true;
        else g_nextFrameUs += Smk_FrameUsec(g_smk);
    }
    Video_Close();
    return aborted;
}

void Video_Close() {
    if (g_smk) {
        Smk_Close(g_smk);
        g_smk = nullptr;
    }
}
