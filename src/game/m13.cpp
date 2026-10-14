// Module 13 (0x4186af..0x419175): the ghost's lever/pull scene between
// activities ("Movies\M\Pull.smk"); picks the next three activities.
//
// Globals (data segment):
//   P(0x4364c8)      talking animation (res 0x4c1), sprite D32(0x4364d0)
//   P(0x4364cc)      second character images (res 0x4c2), sprite D32(0x4364d4)
//   P(0x4364b4)      object image (res 0x514 + step), sprite D32(0x4364d8)
//   D32(0x4364b8)[4] animation ids 0x4c3..0x4c6, D32(0x4364dc)[4] their sprites
//   D32(0x43b43c)    step (unsigned compares), D32(0x43b444)[4] items done,
//   D32(0x43b454)    items left
#include "game/common.h"

// Sub image idx of an offset-table resource: base + ((s32*)base)[idx].
static inline void* M13_Sub(void* base, int idx) {
    return (u8*)base + RdS32((u8*)base + idx * 4);
}

REGISTER_CODE(0x418fe9, FUN_00418fe9);

// 0x4186af
void FUN_004186af() {
    bool done = false;
    // EBX is not initialised by the original before the first loop; the
    // timer (2 s) cannot expire before Event_Poll has run once.
    u32 ebx = 0;
    s16 ev16;
    u32 ev;
    int r;

    FUN_00418b4f();
    FUN_004032e3();
    Hotspot_Set(0x20, g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    if (D32(0x43b414) == 0) {
        if (0xc < DU32(0x43b43c)) {
            FUN_00418c2f(1, 2);
            FUN_00418f58(2);
            FUN_00402e72(0x88b, P(0x4364c8), D32(0x4364d0), 1, 1);
            return;
        }
        if (D32(0x43b454) == 4) {
            FUN_00418c2f(0, 0);
            Timer_Set(0, 2000);
            do {
                ev16 = (s16)ebx;
                if (Timer_Expired(0)) break;
                ebx = (u32)Event_Poll();
                ev16 = (s16)ebx;
            } while (ev16 != 0x20);
            if (ev16 != 0x20) {
                FUN_00402bad(1, 1);
                Video_Play(DSTR(0x436594), D32(0x436ee4), D32(0x436ee8), FUN_00418fe9);
            }
            FUN_00418c2f(1, 0);
            FUN_00418f58(2);
            if (D32(0x43b43c) == 0 && FUN_00402e72(0x847, P(0x4364c8), D32(0x4364d0), 1, 1) == 0) {
                FUN_00402e72(0x848, P(0x4364c8), D32(0x4364d0), 1, 1);
            }
            FUN_00418f58(1);
            {
                void* ang = P(0x4364c8);
                int spr = D32(0x4364d0);
                u32 u = (u32)Rand();
                u = u & 0x80000003;
                if ((s32)u < 0) {
                    u = (u - 1 | 0xfffffffc) + 1;
                }
                FUN_00402e72(u + 0x880, ang, spr, 1, 1);
            }
            FUN_00418e1c();
            FUN_00418f58(2);
            FUN_00402e72(D32(0x43b43c) + 0x849, P(0x4364c8), D32(0x4364d0), 1, 1);
        } else {
            FUN_00418c2f(1, 1);
            FUN_00418f58(2);
            FUN_00402e72((D32(0x43b43c) * 3 - D32(0x43b454)) + 0x85c, P(0x4364c8), D32(0x4364d0), 1, 1);
        }
        FUN_00418f58(1);
        FUN_00418eb9();
        FUN_00418f58(2);
        {
            void* ang = P(0x4364c8);
            int spr = D32(0x4364d0);
            r = Rand();
            FUN_00402e72(r % 5 + 0x884, ang, spr, 1, 1);
        }
        Hotspot_Remove(0x20);
        Timer_Set(6, 10000);
        do {
            if (!Timer_Expired(6)) {
                ev = (u32)Event_Poll();
            } else {
                ev = FUN_00418f58(2);
                Timer_Set(6, 10000);
            }
            r = FUN_0040394c((u16)ev);
            if (r == 0) {
                if ((u16)ev == 0x2e0) {
                    FUN_00418e69();
                } else {
                    int i = (int)(ev & 0xffff) - 0x2e1;
                    if (-1 < i && i < 4) {
                        FUN_00402bad(0, 1);
                        Anim_StartWithSprite(D32(0x4364b8 + i * 4), 0, 0, 0, D32(0x4364dc + i * 4));
                        Timer_Set(0, 100);
                        while ((done = Anim_IsPlaying(D32(0x4364b8 + i * 4))) != false) {
                            Anim_UpdateAll();
                            Spr_Update(1);
                            while (!Timer_Expired(0)) {
                                u32 e = (u32)Event_Poll();
                                FUN_0040394c((u16)e);
                            }
                            Timer_Set(0, 100);
                        }
                        D32(0x43b444 + i * 4) = 1;
                        done = true;
                    }
                }
                Spr_Update(1);
            }
        } while (!done);
        FUN_00418ffe();
    } else {
        FUN_00418c2f(1, 1);
        void* ang = P(0x4364c8);
        int spr = D32(0x4364d0);
        r = Rand();
        FUN_00402e72(r % 3 + 0x856, ang, spr, 1, 1);
    }
    FUN_00418a70();
    FUN_00419175(1);
    FUN_00418b39();
}

// 0x418a61
void FUN_00418a61() {
    FUN_00418a70();
    FUN_00418b39();
}

// 0x418a70
void FUN_00418a70() {
    FUN_00402d81();
    Spr_Delete(D32(0x4364d0), 0);
    Spr_Delete(D32(0x4364d8), 0);
    Spr_Delete(D32(0x4364d4), 0);
    Hotspot_Remove(0x2e0);
    Res_Free(P(0x4364b4));
    P(0x4364b4) = nullptr;
    int i = 0;
    do {
        Anim_Stop(D32(0x4364b8 + i * 4));
        Spr_Delete(D32(0x4364dc + i * 4), 0);
        Anim_Free(D32(0x4364b8 + i * 4));
        Hotspot_Remove((s16)i + 0x2e1);
        i++;
    } while (i < 4);
    Hotspot_Remove(0x20);
    Res_Free(P(0x4364c8));
    P(0x4364c8) = nullptr;
    Res_Free(P(0x4364cc));
    P(0x4364cc) = nullptr;
}

// 0x418b39
void FUN_00418b39() {
    Spr_DeleteAll(0, 0);
    Hotspot_Clear();
}

// 0x418b4f
void FUN_00418b4f() {
    FUN_00402c78(0x7f4);
    FUN_00402b7e(0x808);
    P(0x4364c8) = Res_Load(0x4c1);
    D32(0x4364d0) = Spr_Create(nullptr, D32(0x4364ec), D32(0x4364f0), 9);
    P(0x4364cc) = Res_Load(0x4c2);
    D32(0x4364d4) = Spr_Create(nullptr, D32(0x4364f4), D32(0x4364f8), 8);
    D32(0x43b454) = 0;
    int i = 0;
    do {
        if (D32(0x43b444 + i * 4) == 0) {
            D32(0x43b454) = D32(0x43b454) + 1;
        }
        i++;
    } while (i < 4);
    if (DU32(0x43b43c) < 0xd) {
        P(0x4364b4) = Res_Load(D32(0x43b43c) + 0x514);
        i = 0;
        do {
            D32(0x4364b8 + i * 4) = i + 0x4c3;
            Anim_Load(D32(0x4364b8 + i * 4));
            i++;
        } while (i < 4);
    }
    D32(0x43b458) = 0;
}

// 0x418c2f: draws the background (0x4b1 if param_1 == 0, else 0x4b0 with
// the object sprite (param_2 1) or the overlay 0x4b2 (param_2 2)).
void FUN_00418c2f(int param_1, int param_2) {
    void* img;
    Gfx_FillRect(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h, 0x24);
    if (param_1 == 0) {
        img = Res_Load(0x4b1);
        Img_Draw(D32(0x436ee4), D32(0x436ee8), img, 0);
        Res_Free(img);
        Gfx_SaveBg(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    } else {
        img = Res_Load(0x4b0);
        Img_Draw(D32(0x436ee4), D32(0x436ee8), img, 0);
        Res_Free(img);
        if (param_2 == 1) {
            Gfx_SaveBg(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
            D32(0x4364d8) = Spr_Create(P(0x4364b4), D32(0x4364fc), D32(0x436500), 10);
            Hotspot_Set(0x2e0, D32(0x436504), D32(0x436508), D32(0x43650c), D32(0x436510));
        } else if (param_2 == 2) {
            img = Res_Load(0x4b2);
            Img_Draw(D32(0x436524), D32(0x436528), img, 0);
            Res_Free(img);
            Gfx_SaveBg(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
        } else {
            Gfx_SaveBg(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
        }
    }
    Spr_InvalidateAll();
    Spr_Update(0);
    Screen_FlushAll();
    FUN_00418dd8();
}

// 0x418dd8
void FUN_00418dd8() {
    void* rgb;
    if (DU32(0x43b43c) < 0xd) {
        rgb = Res_Load(D32(0x43b43c) + 0x4b4);
    } else {
        rgb = Res_Load(0x4b3);
    }
    Pal_FadeTo((const u8*)rgb, 1);
    Res_Free(rgb);
}

// 0x418e1c
void FUN_00418e1c() {
    FUN_00402bad(2, 1);
    Anim_PlayFrames(D32(0x43b43c) + 0x521, nullptr, 0x42);
    Hotspot_Set(0x2e0, D32(0x436504), D32(0x436508), D32(0x43650c), D32(0x436510));
}

// 0x418e69
void FUN_00418e69() {
    void* kwav = Res_Load(D32(0x43b43c) + 0x168b);
    int h = Sound_Play(kwav, 0, 1);
    Anim_PlayFrames(D32(0x43b43c) + 0x52e, nullptr, 0x42);
    Sound_Stop(h);
    Res_Free(kwav);
}

// 0x418eb9: shows the remaining items one by one (clickable 0x2e1..0x2e4).
void FUN_00418eb9() {
    Point pt;
    Rect r;
    int i = 0;
    do {
        if (D32(0x43b444 + i * 4) == 0) {
            const void* img = Ang_FirstImage(Anim_GetRes(D32(0x4364b8 + i * 4)), &pt);
            D32(0x4364dc + i * 4) = Spr_Create(img, pt.x, pt.y, 0x14);
            Img_FullRect(img, &r);
            r.x = r.x + pt.x;
            r.y = r.y + pt.y;
            Hotspot_Set((s16)i + 0x2e1, r.x, r.y, r.w, r.h);
            Spr_Update(1);
            Time_Delay(500);
        }
        i++;
    } while (i < 4);
}

// 0x418f58: blink of the second character (frame 0 for 100 ms, then frame
// param_1). Returns the last event not consumed by the toolbar.
u32 FUN_00418f58(int param_1) {
    u32 ev;
    u32 ev2;
    u32 result;
    Spr_SetImage(D32(0x4364d4), M13_Sub(P(0x4364cc), 0));
    Spr_Update(1);
    Timer_Set(0, 100);
    result = 0;
    while (!Timer_Expired(0)) {
        ev = (u32)Event_Poll();
        if ((s16)ev != 0) {
            ev2 = (u32)Event_Poll();
            if (FUN_0040394c((u16)ev2) == 0) {
                result = ev;
            }
        }
    }
    Spr_SetImage(D32(0x4364d4), M13_Sub(P(0x4364cc), param_1));
    Spr_Update(1);
    return result;
}

// 0x418fe9: video abort callback (space bar).
int FUN_00418fe9(int ev) {
    if ((s16)ev == 0x20) {
        return 1;
    }
    return 0;
}

// 0x418ffe: picks the next three activities (0x43b424/0x43b42c/0x43b434,
// values from 0x43652c in 0x43b428/0x43b430/0x43b438), favouring the least
// played (count) and lowest scoring (ratio) ones.
void FUN_00418ffe() {
    float ratio[10];
    int count[10];
    float minRatio = 0.0f;
    int minCount = 0;
    int tries;
    int pick;
    bool more;

    int i = 0;
    do {
        if (i == 9) {
            count[9] = 0x7fffffff;
            ratio[9] = 1.0f;
        } else {
            count[i] = D32(0x439620 + D32(0x4395f8 + i * 4) * 8 + i * 0x30);
            if (count[i] < 1) {
                ratio[i] = 0.0f;
            } else {
                // fild qword (unsigned) / fild dword, stored as float
                ratio[i] = (float)((double)DU32(0x439624 + D32(0x4395f8 + i * 4) * 8 + i * 0x30) /
                                   (double)count[i]);
            }
        }
        if (i == 0) {
            minRatio = ratio[0];
            minCount = count[0];
        } else {
            if (count[i] < minCount) {
                minCount = count[i];
            }
            if (ratio[i] < minRatio) {
                minRatio = ratio[i];
            }
        }
        i++;
    } while (i < 10);
    D32(0x43b414) = 1;
    D32(0x43b418) = 3;
    D32(0x43b41c) = 0;
    FUN_004186a3();
    D32(0x43b420) = 0;
    tries = 0;
    for (;;) {
        pick = FUN_00403dd5(10, (int*)DPTR(0x43657c), 4);
        if (minCount < count[pick]) {
            more = tries < 0x14;
            tries = tries + 1;
            if (more) continue;
        }
        if (pick != 9) break;
    }
    D32(0x43b428) = D32(0x43652c + pick * 4);
    tries = 0;
    D32(0x43b424) = pick;
    do {
        do {
            pick = FUN_00403dd5(10, (int*)DPTR(0x43657c), 4);
        } while (pick == D32(0x43b424));
    } while ((minRatio < ratio[pick] && (more = tries < 0x14, tries = tries + 1, more)) || pick == 9);
    D32(0x43b430) = D32(0x43652c + pick * 4);
    D32(0x43b42c) = pick;
    do {
        do {
            pick = FUN_00403dd5(10, (int*)DPTR(0x43657c), 4);
        } while (pick == D32(0x43b424));
    } while (pick == D32(0x43b42c) || pick == 9);
    D32(0x43b434) = pick;
    D32(0x43b438) = D32(0x43652c + pick * 4);
}
