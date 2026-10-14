// Unit m22 (0x40112c..0x4015d0): module 0x16.
//
// Below location 0x66 this is a menu of 11 picture buttons; otherwise a
// single screen that waits for a click.
//
// Button table at 0x42f074, 11 entries of 0x18 bytes:
//   +0x00 int x, +0x04 int y        sprite position
//   +0x08 int img                   image index (normal) in resource 0x70c
//   +0x0c int imgHover              image index (mouse over)
//   +0x10 int hover                 current state (0x42f084)
//   +0x14 code callback             x86 address, called with no arguments
// Hotspot rects at 0x42f17c (4 ints each), sprite ids at 0x42f230,
// count at 0x42f25c, image resource (pointer) at 0x42f22c.
#include "game/common.h"

// 0x40112c
void FUN_0040112c() {
    int r = Rand() % 4;
    FUN_00402c78(r + 0x7f8);
    if (D32(0x432154) < 0x66) {
        FUN_004011d0();
        return;
    }
    FUN_00401506();
}

// 0x401163
void FUN_00401163() {}

// 0x401168
void FUN_00401168() {
    for (int i = 0; i < D32(0x42f25c); i = i + 1) {
        Spr_Delete(D32(0x42f230 + i * 4), 0);
        Hotspot_Remove((s16)i + 0x370);
    }
    Res_Free(P(0x42f22c));
    P(0x42f22c) = nullptr;
    Hotspot_Remove(0x20);
}

// 0x4011b6
void FUN_004011b6() {
    if (D32(0x432154) < 0x66) {
        FUN_00401168();
        return;
    }
    FUN_00401163();
}

// 0x4011d0
void FUN_004011d0() {
    FUN_004032e3();
    FUN_0040144c();
    do {
        int ev;
        do {
            ev = Event_Poll();
        } while (FUN_0040394c((u16)ev) != 0);
        FUN_00401200((u16)ev);
    } while (true);
}

// 0x401200
int FUN_00401200(u16 param_1) {
    int my = g_mouseY;
    int mx = g_mouseX;
    for (int i = 0; i < D32(0x42f25c); i = i + 1) {
        bool in = Pt_InRect(mx, my, D32(0x42f17c + i * 0x10), D32(0x42f180 + i * 0x10), D32(0x42f184 + i * 0x10),
                            D32(0x42f188 + i * 0x10));
        if (!in) {
            if (D32(0x42f084 + i * 0x18) != 0) {
                D32(0x42f084 + i * 0x18) = 0;
                FUN_004013b3(i);
                Spr_Update(1);
            }
        } else if (D32(0x42f084 + i * 0x18) == 0) {
            D32(0x42f084 + i * 0x18) = 1;
            FUN_004013b3(i);
            Spr_Update(1);
        }
    }
    if (param_1 < 0x370 || D32(0x42f25c) + 0x370 <= (int)(u32)param_1) return 0;
    int idx = param_1 - 0x370;
    if (D32(0x42f088 + idx * 0x18) != 0) {
        FUN_00418669();
        int snd = FUN_00402c30(2, 1);
        while (Sound_IsPlaying(snd)) Event_Poll();
        CODE(D32(0x42f088 + idx * 0x18))();
    }
    return 1;
}

// 0x401312
void FUN_00401312() {
    P(0x42f22c) = Res_Load(0x70c);
    D32(0x42f25c) = 0xb;
    for (int i = 0; i < D32(0x42f25c); i = i + 1) {
        Spr_Delete(D32(0x42f230 + i * 4), 1);
        D32(0x42f084 + i * 0x18) = 0;
    }
    for (int i = 0; i < D32(0x42f25c); i = i + 1) {
        D32(0x42f230 + i * 4) = Spr_Create(nullptr, D32(0x42f074 + i * 0x18), D32(0x42f078 + i * 0x18), 1);
    }
    FUN_00401398();
    Spr_Update(0);
    FUN_0040140c();
}

// 0x401398
void FUN_00401398() {
    for (int i = 0; i < D32(0x42f25c); i = i + 1) FUN_004013b3(i);
}

// 0x4013b3
void FUN_004013b3(int param_1) {
    u8* res = (u8*)P(0x42f22c);
    s32 off;
    if (D32(0x42f084 + param_1 * 0x18) == 0)
        off = RdS32(res + D32(0x42f07c + param_1 * 0x18) * 4);
    else
        off = RdS32(res + D32(0x42f080 + param_1 * 0x18) * 4);
    Spr_SetImage(D32(0x42f230 + param_1 * 4), res + off);
}

// 0x40140c
void FUN_0040140c() {
    for (int i = 0; i < D32(0x42f25c); i = i + 1) {
        Hotspot_Set((s16)i + 0x370, D32(0x42f17c + i * 0x10), D32(0x42f180 + i * 0x10), D32(0x42f184 + i * 0x10),
                    D32(0x42f188 + i * 0x10));
    }
}

// 0x40144c
void FUN_0040144c() {
    Gfx_FillRect(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h, 0x24);
    void* img = Res_Load(0x708);
    Img_Draw(D32(0x436ee4), D32(0x436ee8), img, 0);
    Res_Free(img);
    Gfx_SaveBg(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    FUN_00401312();
    Spr_InvalidateAll();
    Spr_Update(0);
    Screen_FlushAll();
    FUN_004014e2(0x709);
}

// 0x4014e2
void FUN_004014e2(u32 param_1) {
    void* rgb = Res_Load(param_1);
    Pal_FadeTo((const u8*)rgb, 1);
    Res_Free(rgb);
}

// 0x401506
void FUN_00401506() {
    bool done = false;
    FUN_004032e3();
    FUN_0040153c();
    do {
        int ev = Event_Poll();
        int r = FUN_0040394c((u16)ev);
        if (r == 0 && (u16)ev == 0x20) done = true;
    } while (!done);
}

// 0x40153c
void FUN_0040153c() {
    Gfx_FillRect(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h, 0x24);
    void* img = Res_Load(0x70a);
    Img_Draw(D32(0x436ee4), D32(0x436ee8), img, 0);
    Res_Free(img);
    Gfx_SaveBg(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    Spr_InvalidateAll();
    Spr_Update(0);
    Screen_FlushAll();
    FUN_004014e2(0x70b);
}
