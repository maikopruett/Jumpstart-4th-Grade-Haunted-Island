// Module 17: yearbook / progress report screens (0x41a8c8..0x41ba50).
//
// Globals:
//   D32(0x43b49c)  current yearbook page (-1 cover, 0..12 pages, 13 report)
//   D32(0x43b4a0)  page to return to from the report
//   D32(0x43b4a4)  report screen active
//   DU32(0x43b4a8) largest per-subject question total (bar scale)
//   D32(0x43b4ac + i * 8) / D32(0x43b4b0 + i * 8)  subject button positions
//   D32(0x43b504)  print button hover state
//   P(0x436664), P(0x436668), P(0x436678), P(0x43667c), P(0x43666c)  resources
//   D32(0x436670), D32(0x436674)  sprites
//   Subject table at 0x4366d8, 11 entries of 0x5c bytes:
//     +0x00 name[64], +0x40 subject index, +0x44 min level, +0x48 max level,
//     +0x4c clamped level, +0x50 total questions, +0x54 correct, +0x58 hover.
#include "game/common.h"

enum : u32 { SUBJ_TAB = 0x4366d8, SUBJ_SIZE = 0x5c };

static inline s32u& Subj(int i, u32 off) { return D32(SUBJ_TAB + i * SUBJ_SIZE + off); }

// Sub-image at offset-table entry `off` of the button bank P(0x43667c) (offset table at its start).
static inline const void* M17_BankImg(u32 off) {
    u8* res = (u8*)P(0x43667c);
    return res + RdS32(res + off);
}

static void M17_LoadRec(ProgressRec* rec, const void* src) { memcpy(rec, src, sizeof(ProgressRec)); }

// 0x41a8c8: module 17 entry.
void FUN_0041a8c8() {
    int uVar1;
    int iVar2;
    u16 uVar3;

    FUN_0041a9ce();
    FUN_0041aaa7(1);
    do {
        do {
            Spr_Update(1);
            FUN_0041adeb();
            uVar1 = Event_Poll();
            uVar3 = (u16)uVar1;
            iVar2 = FUN_0040394c(uVar3);
        } while (iVar2 != 0);
        if (uVar3 == 0x2f0) {
            FUN_0041b069();
        } else if (uVar3 == 0x2f1) {
            FUN_0041ae6b();
        } else if (uVar3 == 0x2f2) {
            FUN_0041af05();
        } else if (uVar3 == 0x2f3) {
            FUN_0041afdf();
        }
        if (D32(0x43b49c) == 0xd) {
            FUN_0041b113();
        }
    } while (true);
}

// 0x41a93d: module 17 cleanup.
void FUN_0041a93d() {
    Spr_Delete(D32(0x436670), 0);
    Spr_Delete(D32(0x436674), 0);
    Hotspot_Clear();
    Res_Free(P(0x436664));
    P(0x436664) = nullptr;
    Res_Free(P(0x436668));
    P(0x436668) = nullptr;
    Res_Free(P(0x436678));
    P(0x436678) = nullptr;
    Res_Free(P(0x43667c));
    P(0x43667c) = nullptr;
    Res_Free(P(0x43666c));
    P(0x43666c) = nullptr;
    FUN_0041b38a();
}

// 0x41a9ce: loads the module's resources and picks the start page.
void FUN_0041a9ce() {
    FUN_00402c78(0x7f7);
    FUN_00402b7e(0x80b);
    P(0x436664) = Res_Load(0x53b);
    P(0x436668) = Res_Load(0x53c);
    P(0x436678) = Res_Load(0x53d);
    P(0x43667c) = Res_Load(0x53e);
    D32(0x436670) = Spr_Create(nullptr, D32(0x4366a0), D32(0x4366a4), 0x14);
    D32(0x436674) = Spr_Create(nullptr, D32(0x436b7c), D32(0x436b80), 0x14);
    if ((DU32(0x43b440) == 0) && (D32(0x43b414) == 0)) {
        D32(0x43b49c) = -1;
    } else {
        D32(0x43b49c) = D32(0x43b43c);
        if (0xc < D32(0x43b49c)) {
            D32(0x43b49c) = 0xc;
        }
    }
    D32(0x43b4a4) = 0;
}

// 0x41aaa7: draws the current page (fade in when param_1 != 0).
void FUN_0041aaa7(int param_1) {
    void* rgb;
    u32 uVar1;
    void* img;
    int iVar2;

    rgb = Res_Load(D32(0x43b49c) + 0x540);
    if (param_1 == 0) {
        Pal_Set((const u8*)rgb, 1);
    }
    if (D32(0x43b49c) < 0) {
        Img_Draw(D32(0x436ee4), D32(0x436ee8), P(0x436664), 0);
        Spr_SetImage(D32(0x436670), nullptr);
        Spr_SetImage(D32(0x436674), nullptr);
        Spr_Update(0);
    } else if (D32(0x43b49c) < 0xd) {
        Img_Draw(D32(0x436ee4), D32(0x436ee8), P(0x436668), 0);
        Spr_SetImage(D32(0x436674), nullptr);
        Res_Free(P(0x43666c));
        P(0x43666c) = nullptr;
        if (DU32(0x43b49c) < DU32(0x43b43c)) {
            P(0x43666c) = Res_Load(D32(0x43b49c) + 0x55b);
        } else {
            P(0x43666c) = Res_Load(D32(0x43b49c) + 0x54e);
        }
        Spr_SetImage(D32(0x436670), P(0x43666c));
        iVar2 = 0;
        do {
            uVar1 = DU32(0x43b49c) * 4 + iVar2;
            if (uVar1 < DU32(0x43b440)) {
                img = Res_Load(uVar1 + 0x59c);
            } else {
                img = Res_Load(iVar2 + 0x5d0);
            }
            Img_Draw(D32(0x4366b8 + iVar2 * 8), D32(0x4366bc + iVar2 * 8), img, 0);
            Res_Free(img);
            iVar2 = iVar2 + 1;
        } while (iVar2 < 4);
    } else {
        Img_Draw(D32(0x436ee4), D32(0x436ee8), P(0x436678), 0);
        Spr_SetImage(D32(0x436670), nullptr);
        Spr_Update(0);
        Spr_SetImage(D32(0x436674), M17_BankImg(0x58));
    }
    Gfx_SaveBg(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    FUN_004032e3();
    Spr_Update(0);
    Screen_FlushAll();
    if (param_1 != 0) {
        Pal_FadeTo((const u8*)rgb, 1);
    }
    Res_Free(rgb);
    FUN_0041acc3();
}

// 0x41acc3: sets the page navigation hotspots.
void FUN_0041acc3() {
    if ((D32(0x43b49c) < 0) || (0xc < D32(0x43b49c))) {
        Hotspot_Remove(0x2f0);
        Hotspot_Remove(0x2f3);
    } else {
        Hotspot_Set(0x2f0, D32(0x4366a8), D32(0x4366ac), D32(0x4366b0), D32(0x4366b4));
        Hotspot_Set(0x2f3, D32(0x436ba4), D32(0x436ba8), D32(0x436bac), D32(0x436bb0));
    }
    if (D32(0x43b49c) < 1) {
        Hotspot_Remove(0x2f1);
    } else {
        Hotspot_Set(0x2f1, D32(0x436680), D32(0x436684), D32(0x436688), D32(0x43668c));
    }
    if (0xc < D32(0x43b49c)) {
        Hotspot_Remove(0x2f2);
        return;
    }
    Hotspot_Set(0x2f2, D32(0x436690), D32(0x436694), D32(0x436698), D32(0x43669c));
    if (-1 < D32(0x43b49c)) {
        Hotspot_Remove(0x2f3);
        Hotspot_Set(0x2f3, D32(0x436ba4), D32(0x436ba8), D32(0x436bac), D32(0x436bb0));
    }
}

// 0x41adeb: cursor shape for the hotspot under the mouse.
void FUN_0041adeb() {
    int iVar1;
    s16 sVar2;

    iVar1 = g_mouseY;
    sVar2 = Hotspot_HitTest(g_mouseX, g_mouseY);
    if (0x1ad < iVar1) {
        Cursor_Set(2);
        return;
    }
    if (sVar2 != 0x2f0) {
        if (sVar2 == 0x2f1) {
            Cursor_Set(3);
            return;
        }
        if (sVar2 == 0x2f2) {
            Cursor_Set(5);
            return;
        }
        if (sVar2 != 0x2f3) {
            Cursor_Set(2);
            return;
        }
    }
    Cursor_Set(4);
}

// 0x41ae6b: previous page (or back from the report).
void FUN_0041ae6b() {
    FUN_00402bad(0, 1);
    Spr_SetImage(D32(0x436670), nullptr);
    Spr_SetImage(D32(0x436674), nullptr);
    Spr_Update(0);
    if (D32(0x43b49c) < 0xd) {
        Video_Play(DSTR(0x436c0c), D32(0x436ee4), D32(0x436ee8), FUN_0041b054); // Movies\M\Ybpgl.smk
        D32(0x43b49c) = D32(0x43b49c) + -1;
    } else {
        Video_Play(DSTR(0x436c1f), D32(0x436ee4), D32(0x436ee8), FUN_0041b054); // Movies\M\Ybprl.smk
        D32(0x43b49c) = D32(0x43b4a0);
    }
    FUN_0041aaa7(0);
}

// 0x41af05: next page.
void FUN_0041af05() {
    if (D32(0x43b49c) < 0) {
        FUN_00402bad(1, 1);
    } else {
        FUN_00402bad(0, 1);
    }
    Spr_SetImage(D32(0x436670), nullptr);
    Spr_SetImage(D32(0x436674), nullptr);
    Spr_Update(0);
    if (D32(0x43b49c) < 0) {
        Video_Play(DSTR(0x436c32), D32(0x436ee4), D32(0x436ee8), FUN_0041b054); // Movies\M\Ybcovr.smk
    } else if (D32(0x43b49c) < 0xc) {
        Video_Play(DSTR(0x436c46), D32(0x436ee4), D32(0x436ee8), FUN_0041b054); // Movies\M\Ybpgr.smk
    } else {
        D32(0x43b4a0) = D32(0x43b49c);
        Video_Play(DSTR(0x436c59), D32(0x436ee4), D32(0x436ee8), FUN_0041b054); // Movies\M\Ybprr.smk
    }
    D32(0x43b49c) = D32(0x43b49c) + 1;
    FUN_0041aaa7(0);
}

// 0x41afdf: jump to the progress report.
void FUN_0041afdf() {
    FUN_00402bad(0, 1);
    Spr_SetImage(D32(0x436670), nullptr);
    Spr_SetImage(D32(0x436674), nullptr);
    Spr_Update(0);
    Video_Play(DSTR(0x436c6c), D32(0x436ee4), D32(0x436ee8), FUN_0041b054); // Movies\M\Ybprr.smk
    D32(0x43b4a0) = D32(0x43b49c);
    D32(0x43b49c) = 0xd;
    FUN_0041aaa7(0);
}

// 0x41b054: video abort callback (event 0x20).
int FUN_0041b054(int ev) {
    if ((s16)ev == 0x20) {
        return 1;
    }
    return 0;
}
REGISTER_CODE(0x41b054, FUN_0041b054);

// 0x41b069: plays the current page's animation with its sound.
void FUN_0041b069() {
    void* pvVar1;
    int iVar2;

    if (DU32(0x43b49c) < DU32(0x43b43c)) {
        pvVar1 = Res_Load(D32(0x43b49c) + 0x1698);
        iVar2 = Sound_Play(pvVar1, 0, 1);
        Anim_PlayFrames(D32(0x43b49c) + 0x575, nullptr, 0x42);
        Sound_Stop(iVar2);
        Res_Free(pvVar1);
    } else {
        pvVar1 = Res_Load(D32(0x43b49c) + 0x168b);
        iVar2 = Sound_Play(pvVar1, 0, 1);
        Anim_PlayFrames(D32(0x43b49c) + 0x568, nullptr, 0x42);
        Sound_Stop(iVar2);
        Res_Free(pvVar1);
    }
}

// 0x41b113: progress report screen loop.
void FUN_0041b113() {
    bool bVar1;
    int iVar2;
    int iVar3;
    int uVar4;
    int iVar5;
    u16 uVar6;
    int iVar7;
    ProgressRec rec;

    bVar1 = false;
    D32(0x43b4a4) = 1;
    Cursor_Set(2);
    M17_LoadRec(&rec, DPTR(0x4395f0)); // pushed by value (0x86 dwords)
    FUN_0041b3b1(rec);
    FUN_0041b48c(DPTR(0x439810));
    while (iVar2 = g_mouseY, iVar5 = g_mouseX, !bVar1) {
        FUN_0041adeb();
        iVar7 = 0;
        do {
            iVar3 = Pt_InRect(iVar5, iVar2, D32(0x436acc + iVar7 * 16), D32(0x436ad0 + iVar7 * 16),
                              D32(0x436ad4 + iVar7 * 16), D32(0x436ad8 + iVar7 * 16));
            if (iVar3 == 0) {
                if (Subj(iVar7, 0x58) != 0) {
                    Subj(iVar7, 0x58) = 0;
                    Img_Draw(D32(0x43b4ac + iVar7 * 8), D32(0x43b4b0 + iVar7 * 8), M17_BankImg(iVar7 * 8), 0);
                    Screen_Flush(D32(0x436acc + iVar7 * 16), D32(0x436ad0 + iVar7 * 16),
                                 D32(0x436ad4 + iVar7 * 16), D32(0x436ad8 + iVar7 * 16));
                }
            } else if (Subj(iVar7, 0x58) == 0) {
                Subj(iVar7, 0x58) = 1;
                Img_Draw(D32(0x43b4ac + iVar7 * 8), D32(0x43b4b0 + iVar7 * 8), M17_BankImg(4 + iVar7 * 8), 0);
                Screen_Flush(D32(0x436acc + iVar7 * 16), D32(0x436ad0 + iVar7 * 16),
                             D32(0x436ad4 + iVar7 * 16), D32(0x436ad8 + iVar7 * 16));
            }
            iVar7 = iVar7 + 1;
        } while (iVar7 < 0xb);
        iVar5 = Pt_InRect(iVar5, iVar2, D32(0x436b84), D32(0x436b88), D32(0x436b8c), D32(0x436b90));
        if (iVar5 == 0) {
            if (D32(0x43b504) != 0) {
                D32(0x43b504) = 0;
                Spr_SetImage(D32(0x436674), M17_BankImg(0x58));
                Spr_Update(1);
            }
        } else if (D32(0x43b504) == 0) {
            D32(0x43b504) = 1;
            Spr_SetImage(D32(0x436674), M17_BankImg(0x5c));
            Spr_Update(1);
        }
        uVar4 = Event_Poll();
        uVar6 = (u16)uVar4;
        FUN_0040394c(uVar6);
        if (uVar6 == 0x2f1) {
            bVar1 = true;
        } else if (uVar6 == 0x2cf) {
            FUN_0041b811();
        } else {
            uVar6 = (u16)(uVar6 - 0x2d0);
            if (uVar6 < 0xb) {
                D32(0x43211c) = Subj(uVar6, 0x4c);
                Module_Jump(Subj(uVar6, 0x40) + 3);
            }
        }
    }
    FUN_0041b38a();
    D32(0x43b4a4) = 0;
    FUN_0041ae6b();
}

// 0x41b38a: removes the report hotspots.
void FUN_0041b38a() {
    int iVar1;

    iVar1 = 0;
    do {
        Hotspot_Remove((s16)iVar1 + 0x2d0);
        iVar1 = iVar1 + 1;
    } while (iVar1 < 0xb);
    Hotspot_Remove(0x2f1);
}

// 0x41b3b1: computes per-subject totals from a progress block (by value).
void FUN_0041b3b1(const ProgressRec& rec) {
    int iVar1;
    int iVar2;
    u32 uVar3;

    DU32(0x43b4a8) = 0;
    iVar2 = 0;
    do {
        iVar1 = Subj(iVar2, 0x40);
        uVar3 = rec.level[iVar1];
        if ((u32)Subj(iVar2, 0x48) < uVar3) {
            Subj(iVar2, 0x4c) = Subj(iVar2, 0x48);
        } else if (uVar3 < (u32)Subj(iVar2, 0x44)) {
            Subj(iVar2, 0x4c) = Subj(iVar2, 0x44);
        } else {
            Subj(iVar2, 0x4c) = uVar3;
        }
        Subj(iVar2, 0x54) = 0;
        Subj(iVar2, 0x50) = 0;
        for (uVar3 = Subj(iVar2, 0x44); uVar3 <= (u32)Subj(iVar2, 0x48); uVar3 = uVar3 + 1) {
            Subj(iVar2, 0x50) = Subj(iVar2, 0x50) + rec.stats[iVar1][uVar3].total;
            Subj(iVar2, 0x54) = Subj(iVar2, 0x54) + rec.stats[iVar1][uVar3].correct;
        }
        if (DU32(0x43b4a8) < (u32)Subj(iVar2, 0x50)) {
            DU32(0x43b4a8) = Subj(iVar2, 0x50);
        }
        Subj(iVar2, 0x58) = 0;
        iVar2 = iVar2 + 1;
    } while (iVar2 < 0xb);
}

// 0x41b48c: draws the progress report for kid `rec` (points at the name).
void FUN_0041b48c(const void* rec) {
    int iVar1;
    Rect local_ac;
    char local_9c[80];
    Rect local_4c;
    // Original buffer is 16 bytes but overflows harmlessly into the following
    // locals (up to 0x30 bytes); sized generously here.
    char local_3c[0x80];
    Rect local_2c;
    char local_1c[16];
    int local_c;
    int local_8;

    local_c = D32(0x436bb4);
    local_8 = D32(0x436bb8);
    Rect_Set(&local_4c, D32(0x436bbc), D32(0x436bc0), D32(0x436bc4), D32(0x436bc8));
    Rect_Set(&local_ac, D32(0x436bcc), D32(0x436bd0), D32(0x436bd4), D32(0x436bd8));
    snprintf(local_3c, sizeof(local_3c), DSTR(0x436c7f), (const char*)rec); // "%s's\rProgress Report"
    Text_DrawBoxFont(local_4c.x, local_4c.y, local_4c.w, local_4c.h, local_3c, P(0x4318b0), 0x24);
    FUN_0041b734(local_9c);
    Text_DrawBoxFont(local_ac.x, local_ac.y, local_ac.w, local_ac.h, local_9c, P(0x4318a8), 0x24);
    iVar1 = 0;
    do {
        Hotspot_Set((s16)iVar1 + 0x2d0, D32(0x436acc + iVar1 * 16), D32(0x436ad0 + iVar1 * 16),
                    D32(0x436ad4 + iVar1 * 16), D32(0x436ad8 + iVar1 * 16));
        D32(0x43b4ac + iVar1 * 8) = D32(0x436acc + iVar1 * 16);
        D32(0x43b4b0 + iVar1 * 8) = D32(0x436ad0 + iVar1 * 16);
        Img_Draw(D32(0x43b4ac + iVar1 * 8), D32(0x43b4b0 + iVar1 * 8), M17_BankImg(iVar1 * 8), 1);
        if (Subj(iVar1, 0x50) != 0) {
            FUN_0041b78a(local_c, local_8, Subj(iVar1, 0x50), 1);
            FUN_0041b78a(local_c, local_8, Subj(iVar1, 0x54), 0);
            sprintf(local_1c, DSTR(0x436c94), Subj(iVar1, 0x50)); // "%d"
            Rect_Set(&local_2c, local_c + 0xa9, local_8, 0x2e, 0x18);
            Text_DrawBoxFont(local_2c.x, local_2c.y, local_2c.w, local_2c.h, local_1c, P(0x4318a8), 0x24);
            sprintf(local_1c, DSTR(0x436c97), // "%d %%"
                    (int)((u32)(Subj(iVar1, 0x54) * 100) / (u32)Subj(iVar1, 0x50)));
            Rect_Set(&local_2c, local_c + 0xd9, local_8, 0x2e, 0x18);
            Text_DrawBoxFont(local_2c.x, local_2c.y, local_2c.w, local_2c.h, local_1c, P(0x4318a8), 0x24);
        }
        local_8 = local_8 + 0x19;
        iVar1 = iVar1 + 1;
    } while (iVar1 < 0xb);
    Hotspot_Set(0x2f1, D32(0x436680), D32(0x436684), D32(0x436688), D32(0x43668c));
    Hotspot_Set(0x2cf, D32(0x436b84), D32(0x436b88), D32(0x436b8c), D32(0x436b90));
    Spr_InvalidateAll();
    Spr_Update(0);
    Screen_FlushAll();
}

// 0x41b734: formats today's date ("Month D, YYYY").
void FUN_0041b734(char* param_1) {
    const char* local_38[12];
    time_t local_8;
    struct tm* puVar1;

    for (int i = 0; i < 12; i++) {
        local_38[i] = (const char*)P(0x436bdc + i * 4); // "January".. "December"
    }
    local_8 = time(nullptr);
    puVar1 = localtime(&local_8); // FUN_0042e520
    sprintf(param_1, DSTR(0x436cf3), local_38[puVar1->tm_mon], puVar1->tm_mday, puVar1->tm_year + 0x76c);
}

// 0x41b78a: draws one bar of the report (param_4: 1 = total, 0 = correct).
void FUN_0041b78a(int param_1, int param_2, int param_3, int param_4) {
    u32 w;
    Rect local_14;
    int uVar1;

    w = (u32)(param_3 * 0xa8) / DU32(0x43b4a8);
    if ((param_3 != 0) && ((int)w < 4)) {
        w = 4;
    }
    if (0 < (int)w) {
        if (param_4 == 0) {
            Rect_Set(&local_14, param_1, param_2, w, 0xc);
        } else {
            Rect_Set(&local_14, param_1, param_2 + 0xc, w, 0xc);
        }
        uVar1 = 0x23;
        if (param_4 == 0) {
            uVar1 = 0x4f;
        }
        Gfx_FillRect(local_14.x, local_14.y, local_14.w, local_14.h, (u8)uVar1);
    }
}

// 0x41b811: print button. "Teacher" as kid name prints every kid's report.
void FUN_0041b811() {
    int iVar1;

    Spr_SetImage(D32(0x436674), nullptr);
    Spr_Update(1);
    iVar1 = strcmp(DSTR(0x439810), DSTR(0x436cfd)); // "Teacher"
    if (iVar1 != 0) {
        iVar1 = strcmp(DSTR(0x439810), DSTR(0x436d05)); // "TEACHER"
        if (iVar1 != 0) {
            iVar1 = strcmp(DSTR(0x439810), DSTR(0x436d0d)); // "teacher"
            if (iVar1 != 0) {
                FUN_0041b8a1();
                goto LAB_0041b87a;
            }
        }
    }
    FUN_0041b912();
LAB_0041b87a:
    Spr_SetImage(D32(0x436674), M17_BankImg(0x5c));
    Spr_Update(1);
}

// 0x41b8a1: captures the report area and prints it.
void FUN_0041b8a1() {
    int n;
    void* img;
    void* p;

    n = Img_RawSize(D32(0x436b94), D32(0x436b98), D32(0x436b9c), D32(0x436ba0));
    img = Mem_Alloc(n);
    Img_Capture(D32(0x436b94), D32(0x436b98), D32(0x436b9c), D32(0x436ba0), img);
    p = Img_ToDIBAlloc(img);
    Mem_Free(img);
    FUN_004017b3(p);
    Mem_Free(p);
}

// 0x41b912: prints the reports of all kids, then redraws the current one.
void FUN_0041b912() {
    int iVar2;
    ProgressRec rec;

    FUN_00405901();
    FUN_00405ac3();
    for (iVar2 = 0; iVar2 < D32(0x4341d4); iVar2 = iVar2 + 1) {
        u8* kid = (u8*)P(0x4341d8) + iVar2 * 0x329;
        M17_LoadRec(&rec, kid + 0x11); // pushed by value from kid record + 0x11
        FUN_0041b3b1(rec);
        Gfx_RestoreBg(D32(0x436b94), D32(0x436b98), D32(0x436b9c), D32(0x436ba0));
        FUN_0041b48c(kid);
        FUN_0041b8a1();
    }
    Mem_Free(P(0x4341d8));
    P(0x4341d8) = nullptr;
    M17_LoadRec(&rec, DPTR(0x4395f0));
    FUN_0041b3b1(rec);
    Gfx_RestoreBg(D32(0x436b94), D32(0x436b98), D32(0x436b9c), D32(0x436ba0));
    FUN_0041b48c(DPTR(0x439810));
}
