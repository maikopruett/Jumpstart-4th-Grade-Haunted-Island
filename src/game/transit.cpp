// Unit "transit" (0x419175..0x419a56): screens shown between activities
// (assignments from Ms. Grunkle, progress pictures, the kid movies) and
// module 19.
#include "game/common.h"

// 0x419175: assignment screen for the current "chore".
void FUN_00419175(int param_1) {
    D32(0x43b458) = 1;
    Hotspot_Set(0x20, g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    Gfx_FillRect(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h, 0x24);
    Spr_Update(0);
    Screen_FlushAll();
    if (param_1 != 0) {
        void* pal = Res_Load(D32(0x43b43c) + 0x582);
        Pal_Set((const u8*)pal, 1);
        Res_Free(pal);
    }
    void* img = Res_Load(D32(0x43b43c) + 0x58f);
    Img_Draw(D32(0x436ee4), D32(0x436ee8), img, 0);
    Res_Free(img);
    FUN_004192af();
    Gfx_SaveBg(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    Spr_InvalidateAll();
    Spr_Update(0);
    Screen_FlushAll();
    if (param_1 == 0) {
        void* pal = Res_Load(D32(0x43b43c) + 0x582);
        Pal_FadeTo((const u8*)pal, 1);
        Res_Free(pal);
    }
    FUN_004192ff();
    Hotspot_Remove(0x20);
    Pal_FadeToBlack(1);
}

// 0x4192af: assignment text.
void FUN_004192af() {
    int task = D32(0x43b41c);
    char* text = (char*)Res_Load(D32(0x43b424 + task * 8) * 3 + task + 0x4c7);
    Text_DrawBoxFont(D32(0x436514), D32(0x436518), D32(0x43651c), D32(0x436520), text, P(0x4318b0), 0x24);
    // The original never frees this resource.
}

// 0x4192ff: assignment narration.
void FUN_004192ff() {
    int task = D32(0x43b41c);
    FUN_00402e72(D32(0x43b424 + task * 8) * 3 + task + 0x88c, nullptr, 0, 1, 1);
}

// 0x419330: count progress towards the current assignment.
void FUN_00419330(int param_1) {
    if (D32(0x43b414) != 0 && g_module - 3 == D32(0x43b424 + D32(0x43b41c) * 8))
        D32(0x43b420) = D32(0x43b420) + param_1;
}

// 0x41935e: module 19.
void FUN_0041935e() {
    FUN_00402c78(0x7e9);
    void* img = Res_Load(10000);
    Img_Draw(D32(0x436ee4), D32(0x436ee8), img, 0);
    Res_Free(img);
    Screen_FlushAll();
    void* pal = Res_Load(0x2b0b);
    Pal_FadeTo((const u8*)pal, 1);
    Res_Free(pal);
    Hotspot_Set(0x20, g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    while ((s16)Event_Poll() != 0x20) {
    }
}

// 0x4193ec: what happens between two modules.
void FUN_004193ec() {
    Hotspot_Set(0x20, g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    switch (FUN_004194a0()) {
        case 0:
            Hotspot_Remove(0x20);
            return;
        case 1:
            if (Rand() % 5 < 2) FUN_004195a7();
            break;
        case 2:
        case 3:
        case 4:
            FUN_00419175(0);
            break;
        case 5:
            FUN_00419632();
            break;
        case 6:
        case 7:
            FUN_00419632();
            Pal_FadeToBlack(1);
            FUN_004197ed();
            D32(0x43b43c) = D32(0x43b43c) + 1;
            break;
    }
    Hotspot_Remove(0x20);
    Pal_FadeToBlack(1);
}

// 0x4194a0: decide the transition after leaving an activity.
int FUN_004194a0() {
    if (g_prevModule < 3 || 0xc < g_prevModule) return 0;
    if (D32(0x43b414) == 0) return DU32(0x43b43c) < 0xd ? 1 : 8; // jae: unsigned
    int task = D32(0x43b41c);
    if (g_prevModule - 3 != D32(0x43b424 + task * 8)) return 2;
    if (DU32(0x43b420) < DU32(0x43b428 + task * 8)) return 3;
    D32(0x43b41c) = D32(0x43b41c) + 1;
    if (D32(0x43b41c) == 1) D32(0x43b41c) = 2;
    FUN_004186a3();
    D32(0x43b420) = 0;
    if (DU32(0x43b41c) < DU32(0x43b418)) return 4; // jae: unsigned
    D32(0x43b414) = 0;
    D32(0x43b440) = D32(0x43b440) + 1;
    D32(0x43b45c) = 0;
    for (int i = 0; i < 4; i++) {
        if (D32(0x43b444 + i * 4) == 1) D32(0x43b444 + i * 4) = 2;
        if (D32(0x43b444 + i * 4) == 2) D32(0x43b45c) = D32(0x43b45c) + 1;
    }
    if (D32(0x43b45c) < 4) return 5;
    for (int i = 0; i < 4; i++) D32(0x43b444 + i * 4) = 0;
    return DU32(0x43b43c) < 0xc ? 6 : 7; // jae: unsigned
}

// 0x4195a7
void FUN_004195a7() {
    void* img = Res_Load(0x4ec);
    Img_Draw(D32(0x436ee4), D32(0x436ee8), img, 0);
    Res_Free(img);
    Gfx_SaveBg(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    Screen_FlushAll();
    void* pal = Res_Load(0x4ed);
    Pal_FadeTo((const u8*)pal, 1);
    Res_Free(pal);
    FUN_00402e72(0x8a7, nullptr, 0, 1, 1);
}

// 0x419632: show the collected picture pieces and reveal the new one.
void FUN_00419632() {
    Gfx_FillRect(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h, 0x24);
    u32 i;
    for (i = 0; (int)i < D32(0x43b45c) - 1; i++) {
        void* img = Res_Load(D32(0x43b43c) * 4 + i + 0x5d4);
        u32 y = (int)i < 2 ? 0 : (u32)g_screenH >> 1;
        u32 x = (i & 1) == 0 ? 0 : (u32)g_screenW >> 1;
        Img_Draw((int)x, (int)y, img, 1);
        Res_Free(img);
    }
    Screen_FlushAll();
    void* pal = Res_Load(D32(0x43b43c) + 0x608);
    Pal_FadeTo((const u8*)pal, 1);
    Res_Free(pal);
    void* img = Res_Load(D32(0x43b43c) * 4 + i + 0x5d4);
    u32 y = (int)i < 2 ? 0 : (u32)g_screenH >> 1;
    u32 x = (i & 1) == 0 ? 0 : (u32)g_screenW >> 1;
    Img_Draw((int)x, (int)y, img, 1);
    Res_Free(img);
    void* snd = Res_Load(0x1688);
    int h = Sound_Play(snd, 0, 1);
    Fx_Dissolve((int)x, (int)y, (u32)g_screenW >> 1, (u32)g_screenH >> 1, 3, 3);
    while (Sound_IsPlaying(h)) Event_Poll();
    Res_Free(snd);
    FUN_00402e72(Rand() % 4 + 0x8a8, nullptr, 0, 1, 1); // signed % 4 (and 0x80000003 + fixup)
}

// 0x4197ed: the kid movies after a completed picture.
void FUN_004197ed() {
    void* img = Res_Load(0x4f6);
    Img_Draw(D32(0x436ee4), D32(0x436ee8), img, 0);
    Res_Free(img);
    Screen_FlushAll();
    void* pal = Res_Load(D32(0x43b43c) + 0x4b3);
    Pal_FadeTo((const u8*)pal, 1);
    Res_Free(pal);
    void* snd = Res_Load(0x1689);
    int h = Sound_Play(snd, 0, 1);
    char path[260];
    sprintf(path, DSTR(0x4365a6), D32(0x43b43c));
    Video_Play(path, D32(0x436ee4), D32(0x436ee8), FUN_00418fe9);
    while (Sound_IsPlaying(h)) Event_Poll();
    Res_Free(snd);
    if (FUN_00402e72(D32(0x43b43c) * 2 + 0x8ac, nullptr, 0, 1, 1) == 0)
        FUN_00402e72(D32(0x43b43c) * 2 + 0x8ad, nullptr, 0, 1, 1);
    snd = Res_Load(0x168a);
    h = Sound_Play(snd, 0, 1);
    sprintf(path, DSTR(0x4365bc), D32(0x43b43c));
    Video_Play(path, D32(0x436ee4), D32(0x436ee8), FUN_00418fe9);
    while (Sound_IsPlaying(h)) Event_Poll();
    Res_Free(snd);
    Timer_Set(0, 1000);
    while (!Timer_Expired(0)) Event_Poll();
}
