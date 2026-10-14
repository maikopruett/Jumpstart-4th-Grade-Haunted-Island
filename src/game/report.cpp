// Module 18 (FUN_0041bab2) and the Ms. Grunkle question / multiple choice
// screens (0x41ba50..0x41c62c).
//
// Globals:
//   D32(0x43b514)  mode (param of FUN_0041bab2: 1 or 2)
//   D32(0x43b50c)  last answer was right (mode 2)
//   D16(0x43b510) / D16(0x43b512)  question counters for mode 2 / mode 1
//   D32(0x43b518)  current question index
//   P(0x436d1c)    question text resource, P(0x436d20) question string,
//   P(0x436d24 + i * 4) answer strings (max 6)
//   D32(0x43b51c)  answer count, D32(0x43b520 + i * 4) shuffled order,
//   D32(0x43b538 + i * 4) hover flags
//   0x43b550 question rect, 0x43b560 + i * 16 answer rects
//   P(0x43b5c0) / P(0x43b5c4) question / answer fonts
#include "game/common.h"

static inline const char* Rep_Answer(int i) {
    return (const char*)P(0x436d24 + D32(0x43b520 + i * 4) * 4);
}

static inline void Rep_SetRect(u32 va, int x, int y, int w, int h) {
    D32(va) = x;
    D32(va + 4) = y;
    D32(va + 8) = w;
    D32(va + 12) = h;
}

// 0x41ba50: returns 2 (go to module 18) with a 10% chance at certain locations.
int FUN_0041ba50() {
    int iVar1;
    int iVar2;
    s16 local_48[34];

    for (iVar2 = 0; iVar2 < 0x22; iVar2++) {
        local_48[iVar2] = D16(0x436d60 + iVar2 * 2);
    }
    iVar2 = 0;
    while (((local_48[iVar2] != D32(0x432154) || (iVar1 = Rand(), 9 < iVar1 % 100)) || (D32(0x43b50c) != 0))) {
        iVar2 = iVar2 + 1;
        if (0x21 < iVar2) {
            return 0;
        }
    }
    return 2;
}

// 0x41bab2: module 18 entry.
void FUN_0041bab2(int param_1) {
    u32 uVar1;

    D32(0x436d18) = 1;
    D32(0x43b514) = param_1;
    FUN_004032e3();
    Cursor_Set(2);
    FUN_0041baf8();
    uVar1 = FUN_0041bf85();
    if (uVar1 == 0) {
        FUN_0041bd4e();
    } else {
        FUN_0041bcbe();
    }
    D32(0x436d18) = 0;
}

// 0x41baf8: intro (mode 2): animation 0x615, then a talking line.
void FUN_0041baf8() {
    bool bVar1;
    void* pvVar2;
    int iVar3;
    int iVar5;
    int iVar6;
    int uVar4;

    if (D32(0x43b514) == 2) {
        Anim_Load(0x615);
        D32(0x436d4c) = Spr_Create(nullptr, D32(0x436ee4), D32(0x436ee8), 0xc);
        pvVar2 = Res_Load(0x160d);
        iVar3 = Sound_Play(pvVar2, 0, 1);
        Anim_StartWithSprite(0x615, 0, 0, 0xc, D32(0x436d4c));
        Timer_Set(2, 0);
        while (true) {
            bVar1 = Anim_IsPlaying(0x615);
            if (!bVar1) break;
            uVar4 = Event_Poll();
            FUN_0040394c((u16)uVar4);
            bVar1 = Timer_Expired(2);
            if (bVar1) {
                Timer_Set(2, 100);
                Anim_UpdateAll();
                Spr_Update(1);
            }
        }
        Sound_Stop(iVar3);
        Res_Free(pvVar2);
        Hotspot_Set(0x20, D32(0x43188d), D32(0x431891), D32(0x431895), D32(0x431899));
        P(0x436d54) = Res_Load(0x617);
        D32(0x436d50) = Spr_Create(nullptr, D32(0x436d58), D32(0x436d5c), 0xd);
        if (D32(0x43b508) == 0) {
            FUN_00402e72(0x160e, P(0x436d54), D32(0x436d50), 1, 1);
            D32(0x43b508) = 1;
        } else {
            iVar6 = 1;
            iVar5 = 1;
            pvVar2 = P(0x436d54);
            iVar3 = D32(0x436d50);
            uVar4 = Rand() % 4;
            FUN_00402e72(uVar4 + 0x160f, pvVar2, iVar3, iVar5, iVar6);
        }
        Hotspot_Remove(0x20);
        Spr_Delete(D32(0x436d4c), 0);
        Anim_Free(0x615);
        Spr_Delete(D32(0x436d50), 0);
        Res_Free(P(0x436d54));
        P(0x436d54) = nullptr;
    }
}

// 0x41bcbe: right answer.
void FUN_0041bcbe() {
    if (D32(0x43b514) != 1) {
        D32(0x43b50c) = 1;
        D16(0x43b510) = (s16)(D16(0x43b510) + 1);
        if (0x32 < (s16)D16(0x43b510)) {
            D16(0x43b510) = 0;
        }
        return;
    }
    D16(0x43b512) = (s16)(D16(0x43b512) + 1);
    if (0x41 < (s16)D16(0x43b512)) {
        D16(0x43b512) = 0;
    }
    FUN_004040f5(1);
    if (D32(0x439800) < 10) {
        FUN_00402e72(0x1609, nullptr, 0, 1, 1);
        return;
    }
    FUN_00402e72(0x160a, nullptr, 0, 1, 1);
}

// 0x41bd4e: wrong answer.
void FUN_0041bd4e() {
    if (D32(0x43b514) == 1) {
        int uVar1 = Rand() % 2;
        FUN_00402e72(uVar1 + 0x160b, nullptr, 0, 1, 1);
        return;
    }
    D32(0x43b50c) = 0;
}

// 0x41bd8c: outro (mode 2): talking line, then animation 0x616.
void FUN_0041bd8c() {
    bool bVar1;
    const void* pvVar2;
    int uVar3;
    int iVar4;
    int iVar5;
    int iVar6;
    Point local_c;

    if (D32(0x43b514) == 2) {
        Anim_Load(0x616);
        pvVar2 = Anim_GetRes(0x616);
        pvVar2 = Ang_FirstImage(pvVar2, &local_c);
        D32(0x436d4c) = Spr_Create(pvVar2, local_c.x, local_c.y, 0xc);
        Spr_Update(0);
        Screen_FlushAll();
        Hotspot_Set(0x20, D32(0x43188d), D32(0x431891), D32(0x431895), D32(0x431899));
        P(0x436d54) = Res_Load(0x617);
        D32(0x436d50) = Spr_Create(nullptr, D32(0x436d58), D32(0x436d5c), 0xd);
        if (D32(0x43b50c) == 0) {
            iVar6 = 1;
            iVar5 = 1;
            void* ang = P(0x436d54);
            iVar4 = D32(0x436d50);
            uVar3 = Rand() % 2;
            FUN_00402e72(uVar3 + 0x1607, ang, iVar4, iVar5, iVar6);
        } else {
            iVar6 = 1;
            iVar5 = 1;
            void* ang = P(0x436d54);
            iVar4 = D32(0x436d50);
            uVar3 = Rand() % 2;
            FUN_00402e72(uVar3 + 0x1605, ang, iVar4, iVar5, iVar6);
        }
        Hotspot_Remove(0x20);
        Spr_Delete(D32(0x436d50), 0);
        Res_Free(P(0x436d54));
        P(0x436d54) = nullptr;
        Anim_StartWithSprite(0x616, 0, 0, 0xc, D32(0x436d4c));
        Timer_Set(2, 0);
        while (bVar1 = Anim_IsPlaying(0x616), bVar1) {
            uVar3 = Event_Poll();
            FUN_0040394c((u16)uVar3);
            bVar1 = Timer_Expired(2);
            if (bVar1) {
                Timer_Set(2, 100);
                Anim_UpdateAll();
                // ebx is always 1 here (0x41bf21); the Spr_Update(1) branch is dead.
                Spr_Update(0);
                Screen_FlushAll();
            }
        }
        Spr_Delete(D32(0x436d4c), 1);
        Anim_Free(0x616);
        Spr_Update(1);
        if (D32(0x43b50c) == 0) {
            FUN_004040f5(-1);
        }
    }
}

// 0x41bf85: multiple choice loop; returns 1 if the right answer (index 0) was picked.
u32 FUN_0041bf85() {
    bool bVar1;
    int iVar2;
    int iVar3;
    int uVar4;
    int iVar5;
    int iVar6;
    u32 local_c = 0;

    bVar1 = false;
    FUN_0041c139();
    FUN_0041c165();
    FUN_0041c1b5();
    FUN_0041c292();
    FUN_0041c580();
    FUN_0041c540();
    for (iVar5 = 0; iVar5 < D32(0x43b51c); iVar5 = iVar5 + 1) {
        D32(0x43b538 + iVar5 * 4) = 0;
    }
    while (iVar2 = g_mouseY, iVar5 = g_mouseX, !bVar1) {
        for (iVar6 = 0; iVar6 < D32(0x43b51c); iVar6 = iVar6 + 1) {
            u32 r = 0x43b560 + iVar6 * 16;
            iVar3 = Pt_InRect(iVar5, iVar2, D32(r), D32(r + 4), D32(r + 8), D32(r + 12));
            if (iVar3 == 0) {
                if (D32(0x43b538 + iVar6 * 4) != 0) {
                    D32(0x43b538 + iVar6 * 4) = 0;
                    Text_DrawBoxFont(D32(r), D32(r + 4), D32(r + 8), D32(r + 12), Rep_Answer(iVar6), P(0x43b5c4), 0x33);
                    Screen_Flush(D32(r), D32(r + 4), D32(r + 8), D32(r + 12));
                }
            } else if (D32(0x43b538 + iVar6 * 4) == 0) {
                D32(0x43b538 + iVar6 * 4) = 1;
                Text_DrawBoxFont(D32(r), D32(r + 4), D32(r + 8), D32(r + 12), Rep_Answer(iVar6), P(0x43b5c4), 0x1c);
                Screen_Flush(D32(r), D32(r + 4), D32(r + 8), D32(r + 12));
            }
        }
        uVar4 = Event_Poll();
        FUN_0040394c((u16)uVar4);
        if ((0x63f < (u16)uVar4) && ((int)(uVar4 & 0xffff) < D32(0x43b51c) + 0x640)) {
            local_c = (u32)(D32(0x43b520 + ((uVar4 & 0xffff) - 0x640) * 4) == 0);
            bVar1 = true;
        }
    }
    FUN_0041c14d();
    return local_c;
}

// 0x41c139: selects the fonts.
void FUN_0041c139() {
    P(0x43b5c4) = P(0x4318b0);
    P(0x43b5c0) = P(0x4318b0);
}

// 0x41c14d: frees the question text.
void FUN_0041c14d() {
    Res_Free(P(0x436d1c));
    P(0x436d1c) = nullptr;
}

// 0x41c165: loads the current question text.
void FUN_0041c165() {
    u32 id;

    if (D32(0x43b514) == 1) {
        D32(0x43b518) = (int)(s16)D16(0x43b512);
        id = D32(0x43b518) + 0x680;
    } else {
        D32(0x43b518) = (int)(s16)D16(0x43b510);
        id = D32(0x43b518) * 2 + 0x61a;
    }
    P(0x436d1c) = Res_Load(id);
}

// 0x41c1b5: splits the text into question and answers (blank line separated)
// and shuffles the answer order.
void FUN_0041c1b5() {
    bool bVar1;
    bool bVar2;
    char* pcVar3;
    int iVar4;

    bVar2 = false;
    bVar1 = false;
    D32(0x43b51c) = -1;
    P(0x436d20) = P(0x436d1c);
    pcVar3 = (char*)P(0x436d1c);
    while ((*pcVar3 != '\0' && (D32(0x43b51c) == -1))) {
        if ((*pcVar3 == '\r') || (*pcVar3 == '\n')) {
            if (bVar1) {
                if ((bVar2) && (*pcVar3 == '\r')) {
                    pcVar3[-2] = '\0';
                    D32(0x43b51c) = 0;
                } else {
                    bVar2 = true;
                }
            } else {
                P(0x436d20) = (char*)P(0x436d20) + 1;
            }
        } else {
            bVar1 = true;
            bVar2 = false;
        }
        pcVar3 = pcVar3 + 1;
    }
    for (; (*pcVar3 != '\0' && ((*pcVar3 == '\r' || (*pcVar3 == '\n')))); pcVar3 = pcVar3 + 1) {
    }
    while ((*pcVar3 != '\0' && (D32(0x43b51c) < 6))) {
        // With no separator found the count is still -1 here, so the first
        // answer lands in slot 0x436d20 (the question), as in the original.
        P(0x436d24 + D32(0x43b51c) * 4) = pcVar3;
        D32(0x43b51c) = D32(0x43b51c) + 1;
        for (; ((*pcVar3 != '\0' && (*pcVar3 != '\r')) && (*pcVar3 != '\n')); pcVar3 = pcVar3 + 1) {
        }
        if (*pcVar3 != '\0') {
            *pcVar3 = '\0';
            pcVar3 = pcVar3 + 1;
        }
        for (; (*pcVar3 != '\0' && ((*pcVar3 == '\r' || (*pcVar3 == '\n')))); pcVar3 = pcVar3 + 1) {
        }
    }
    for (iVar4 = 0; iVar4 < D32(0x43b51c); iVar4 = iVar4 + 1) {
        D32(0x43b520 + iVar4 * 4) = iVar4;
    }
    Util_Shuffle(DPTR(0x43b520), (unsigned)D32(0x43b51c), 4);
}

// 0x41c292: draws the question screen.
void FUN_0041c292() {
    void* pvVar1;
    int iVar2;

    Gfx_FillRect(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h, 0x24);
    pvVar1 = Res_Load(0x618);
    Img_Draw(D32(0x436ee4), D32(0x436ee8), pvVar1, 0);
    Res_Free(pvVar1);
    FUN_0041c3cc();
    Text_DrawBoxFont(D32(0x43b550), D32(0x43b554), D32(0x43b558), D32(0x43b55c), (const char*)P(0x436d20),
                     P(0x43b5c0), 0x33);
    for (iVar2 = 0; iVar2 < D32(0x43b51c); iVar2 = iVar2 + 1) {
        u32 r = 0x43b560 + iVar2 * 16;
        Text_DrawBoxFont(D32(r), D32(r + 4), D32(r + 8), D32(r + 12), Rep_Answer(iVar2), P(0x43b5c4), 0x33);
    }
    Spr_DeleteAll(0, 0);
    Gfx_SaveBg(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    FUN_004032e3();
    Spr_InvalidateAll();
    Spr_Update(0);
    Screen_FlushAll();
    if (D32(0x43b514) == 1) {
        pvVar1 = Res_Load(0x619);
        Pal_FadeTo((const u8*)pvVar1, 1);
        Res_Free(pvVar1);
    }
}

// 0x41c3cc: lays out the question and answer rectangles.
void FUN_0041c3cc() {
    int iVar1;
    int iVar2;
    int local_10;
    u32 local_c;
    int local_8;

    local_c = D32(0x436d44);
    local_8 = Text_MeasureHeightFont((const char*)P(0x436d20), P(0x43b5c0), D32(0x436d44));
    Rep_SetRect(0x43b550, (u32)(g_screenW - local_c) >> 1, 0, local_c, local_8);
    iVar2 = local_8 + 0x19;
    for (iVar1 = 0; iVar1 < D32(0x43b51c); iVar1 = iVar1 + 1) {
        Text_MeasureFont(Rep_Answer(iVar1), P(0x43b5c4), &local_c, &local_10);
        if (D32(0x436d44) < (int)local_c) {
            local_c = D32(0x436d44);
            local_10 = Text_MeasureHeightFont(Rep_Answer(iVar1), P(0x43b5c4), D32(0x436d44));
            Rep_SetRect(0x43b560 + iVar1 * 16, (u32)(g_screenW - local_c) >> 1, 0, local_c, local_10);
        } else {
            Rep_SetRect(0x43b560 + iVar1 * 16, (u32)(g_screenW - local_c) >> 1, 0, local_c, local_10);
        }
        iVar2 = iVar2 + local_10 + 10;
    }
    iVar1 = (D32(0x436d48) - iVar2) / 2; // sar + adc: rounds toward zero
    D32(0x43b554) = iVar1 + D32(0x436d40);
    D32(0x43b564) = D32(0x43b554) + local_8 + 0x19;
    for (iVar1 = 1; iVar1 < D32(0x43b51c); iVar1 = iVar1 + 1) {
        D32(0x43b564 + iVar1 * 16) = D32(0x43b554 + iVar1 * 16) + D32(0x43b55c + iVar1 * 16) + 10;
    }
}

// 0x41c540: answer hotspots 0x640 + i.
void FUN_0041c540() {
    int iVar1;

    for (iVar1 = 0; iVar1 < D32(0x43b51c); iVar1 = iVar1 + 1) {
        u32 r = 0x43b560 + iVar1 * 16;
        Hotspot_Set((s16)iVar1 + 0x640, D32(r), D32(r + 4), D32(r + 8), D32(r + 12));
    }
}

// 0x41c580: reads the question aloud.
void FUN_0041c580() {
    u32 uVar1;

    if (D32(0x43b514) == 1) {
        uVar1 = D32(0x43b518) + 0x1646;
    } else {
        uVar1 = D32(0x43b518) + 0x1613;
    }
    FUN_00402e72(uVar1, nullptr, 0, 1, 1);
}
