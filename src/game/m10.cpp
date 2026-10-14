// Module 10 (0x40be0d..0x40cf8c): a 28-card matching activity. Cards are
// dealt in pairs (groups of 4 images share a "family" = image index >> 2);
// the player selects two matching cards to remove them. A magnifier mode
// (hotspot 0x48f) shows a card's text. Ends with Movies\mumfade.smk.
//
// Data segment layout used by this module:
//   0x439de4 int        level + 1 (written only)
//   0x439de8 int        round (0..2)
//   0x439dec int        round finished flag
//   0x439df0 int        stuck flag (>= 0x1e: no more pairs, re-deal)
//   0x439df4 int        selected card count / deal counter
//   0x439df8 int        remaining cards
//   0x439dfc int        rotating offset into the shuffled card set list
//   0x439e00 int[0x4b]  shuffled card sets
//   0x439f2c ptr[0x1c]  loaded card images (P)
//   0x439f9c ptr        card back 'ANG' (P), 0x439fa0 magnifier 'ANG' (P),
//   0x439fa4 ptr        text panel 'ANG' (P), 0x439fa8 cursor 'ANG' (P)
//   0x439fac int        magnifier / panel sprite
//   0x439fb0 int[0x1c]  card back sprites (nonzero = card still present)
//   0x43a020 ptr[0x1c]  saved card back images (P)
//   0x43a090 u8[0x1c]   card selected flag (also used as back frame)
//   0x43a0ac u8[0x1c]   deal order (slot by deal position)
//   0x43a0c8 u8[0x1c]   image index per slot
//   0x43a0e4 u8         family of the current selection
//   0x43a0e8 int        current card index
//   0x43a0ec u8[0x1c]   card set id per image
//   0x43a108 u8[0x1c]   card set id per slot
//   0x43a124 ptr[0x1c]  card face image per slot (P)
//   0x43a194 int[0x1c]  card face sprites
//   0x43a204 int        mode (0 play, 1 magnifier, 2 opening text, 3 text)
//   0x43a208 int        frame delay before the text appears
#include "game/common.h"

// LAB_0040bdf8: Video_Play abort callback (space aborts).
static int LAB_0040bdf8(int ev) {
    if ((u16)ev == 0x20) return 1;
    return 0;
}

// 0x40be0d: deal the 28 slots in random pairs.
void FUN_0040be0d() {
    u8 bVar1;
    u32 uVar2;
    int iVar3;
    int iVar4;
    int iVar5;
    u8 abStack_3c[56];

    iVar5 = 0;
    do {
        D32(0x439fb0 + iVar5 * 4) = 1;
        abStack_3c[iVar5] = (u8)iVar5;
        iVar5 = iVar5 + 1;
    } while (iVar5 < 0x1c);
    if (D32(0x439de8) != 0) {
        uVar2 = (u32)Rand();
        uVar2 = uVar2 & 0x80000001;
        if ((int)uVar2 < 0) {
            uVar2 = ((uVar2 - 1) | 0xfffffffe) + 1;
        }
        if (uVar2 == 0) goto LAB_0040be8c;
    }
    iVar5 = 0;
    do {
        iVar3 = Rand();
        iVar3 = (iVar3 % 0xe) * 2;
        bVar1 = abStack_3c[iVar5];
        abStack_3c[iVar5] = abStack_3c[iVar3];
        abStack_3c[iVar3] = bVar1;
        bVar1 = abStack_3c[iVar5 + 1];
        abStack_3c[iVar5 + 1] = abStack_3c[iVar3 + 1];
        abStack_3c[iVar3 + 1] = bVar1;
        iVar5 = iVar5 + 2;
    } while (iVar5 < 0x1c);
LAB_0040be8c:
    D32(0x439df4) = 0;
    do {
        iVar3 = 0;
        iVar5 = 0;
        do {
            if (D32(0x439fb0 + iVar5 * 4) != 0) {
                iVar4 = FUN_0040c3d5(iVar5);
                if (iVar4 != 0) {
                    abStack_3c[iVar3 + 0x1c] = (u8)iVar5;
                    iVar3 = iVar3 + 1;
                }
            }
            iVar5 = iVar5 + 1;
        } while (iVar5 < 0x1c);
        if (iVar3 == 0) {
            iVar5 = 0;
        } else {
            iVar5 = Rand();
            iVar5 = iVar5 % iVar3;
        }
        do {
            if (iVar3 == 0) {
                iVar4 = 0;
            } else {
                iVar4 = Rand();
                iVar4 = iVar4 % iVar3;
            }
        } while (iVar4 == iVar5);
        D8(0x43a0c8 + abStack_3c[iVar5 + 0x1c]) = abStack_3c[D32(0x439df4)];
        D8(0x43a0ac + D32(0x439df4)) = abStack_3c[iVar5 + 0x1c];
        D32(0x439df4) = D32(0x439df4) + 1;
        D32(0x439fb0 + abStack_3c[iVar5 + 0x1c] * 4) = 0;
        D8(0x43a0c8 + abStack_3c[iVar4 + 0x1c]) = abStack_3c[D32(0x439df4)];
        D8(0x43a0ac + D32(0x439df4)) = abStack_3c[iVar4 + 0x1c];
        D32(0x439df4) = D32(0x439df4) + 1;
        D32(0x439fb0 + abStack_3c[iVar4 + 0x1c] * 4) = 0;
    } while (D32(0x439df4) < 0x1c);
}

// 0x40bf68: 1 if two selectable cards of the same family remain.
int FUN_0040bf68() {
    int iVar1;
    int iVar2;
    int iVar3;
    u8 abStack_20[28];

    iVar3 = 0;
    iVar2 = 0;
    do {
        if ((D32(0x439fb0 + iVar2 * 4) != 0) && (iVar1 = FUN_0040c3d5(iVar2), iVar1 != 0)) {
            abStack_20[iVar3] = (u8)iVar2;
            iVar3 = iVar3 + 1;
        }
        iVar2 = iVar2 + 1;
    } while (iVar2 < 0x1c);
    iVar2 = 0;
    do {
        if (iVar3 + -1 <= iVar2) {
            return 0;
        }
        iVar1 = 0;
        if (0 < iVar3) {
            do {
                if ((iVar2 != iVar1) &&
                    ((int)(u32)D8(0x43a0c8 + abStack_20[iVar2]) >> 2 ==
                     (int)(u32)D8(0x43a0c8 + abStack_20[iVar1]) >> 2)) {
                    return 1;
                }
                iVar1 = iVar1 + 1;
            } while (iVar1 < iVar3);
        }
        iVar2 = iVar2 + 1;
    } while (true);
}

// 0x40bff2: draw the text of the current card.
void FUN_0040bff2() {
    char* text;
    int iVar1;

    text = (char*)Res_Load(D8(0x43a108 + D32(0x43a0e8)) + 0x2fb2);
    if (text != nullptr) {
        iVar1 = Res_LastSize();
        text[iVar1 + -1] = '\0';
        Text_DrawFont(text, 100, 0x28, 0x1a4, 0x15e, P(0x4318b0), 0x24, 0);
        Screen_Flush(100, 0x28, 0x1a4, 0x15e);
        Res_Free(text);
    }
}

// 0x40c07c: set up a round (deal, load images).
void FUN_0040c07c() {
    u8 bVar1;
    void* pvVar3;
    int iVar4;
    int iVar5;
    int iVar6;

    Spr_DeleteAll(0, 0);
    FUN_0040c70c();
    FUN_0040be0d();
    iVar5 = 0;
    do {
        Res_Free(P(0x439f2c + iVar5 * 4));
        P(0x439f2c + iVar5 * 4) = nullptr;
        iVar5 = iVar5 + 1;
    } while (iVar5 < 0x1c);
    iVar5 = 0;
    do {
        iVar6 = iVar5;
        if (iVar5 < 0) {
            iVar6 = iVar5 + 3;
        }
        iVar6 = (iVar6 >> 2) + D32(0x439dfc);
        if (0x4a < iVar6) {
            iVar6 = iVar6 + -0x4b;
        }
        iVar4 = 0;
        do {
            D8(0x43a0ec + iVar4 + iVar5) = D8(0x439e00 + iVar6 * 4);
            pvVar3 = Res_Load(D32(0x439e00 + iVar6 * 4) * 4 + iVar4 + 0x319c);
            P(0x439f2c + (iVar4 + iVar5) * 4) = pvVar3;
            iVar4 = iVar4 + 1;
        } while (iVar4 < 4);
        iVar5 = iVar5 + 4;
    } while (iVar5 < 0x1c);
    D32(0x439dfc) = D32(0x439dfc) + 7;
    if (0x4a < D32(0x439dfc)) {
        D32(0x439dfc) = D32(0x439dfc) - 0x4b; // 0x40c128: sub [0x439dfc],0x4b
    }
    iVar5 = 0;
    do {
        u8 slot = D8(0x43a0ac + iVar5);
        bVar1 = D8(0x43a0c8 + slot);
        P(0x43a124 + slot * 4) = P(0x439f2c + bVar1 * 4);
        D8(0x43a108 + slot) = D8(0x43a0ec + bVar1);
        iVar5 = iVar5 + 1;
    } while (iVar5 < 0x1c);
    D32(0x434f38) = Spr_Create(nullptr, D32(0x436ee4), D32(0x436ee8), 1000);
}

// 0x40c198: draw the background and the cards, fade in.
void FUN_0040c198() {
    void* pvVar1;

    pvVar1 = Res_Load(0x2fa8);
    Img_Draw(D32(0x436ee4), D32(0x436ee8), pvVar1, 0);
    Res_Free(pvVar1);
    Gfx_SaveBg(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    FUN_004032e3();
    FUN_0040c5c8();
    Spr_Update(0);
    Screen_FlushAll();
    pvVar1 = Res_Load(0x2fa9);
    Pal_FadeTo((const u8*)pvVar1, 1);
    Res_Free(pvVar1);
    Timer_Set(0, 0x32);
    Timer_Set(1, 200);
}

// 0x40c297: free the module's resources.
void FUN_0040c297() {
    int iVar1;

    iVar1 = 0;
    do {
        Res_Free(P(0x439f2c + iVar1 * 4));
        P(0x439f2c + iVar1 * 4) = nullptr;
        iVar1 = iVar1 + 1;
    } while (iVar1 < 0x1c);
    Res_Free(P(0x439f9c));
    P(0x439f9c) = nullptr;
    Res_Free(P(0x439fa4));
    P(0x439fa4) = nullptr;
    Res_Free(P(0x439fa0));
    P(0x439fa0) = nullptr;
    Res_Free(P(0x439fa8));
    P(0x439fa8) = nullptr;
}

// 0x40c30a: module initialisation.
void FUN_0040c30a() {
    int uVar1;
    u32 uVar2;
    int iVar3;
    int iVar4;

    FUN_00403e91();
    uVar2 = FUN_004040cb();
    D32(0x439de4) = uVar2 + 1;
    FUN_00402c78(0x7f0);
    P(0x439fa8) = Res_Load(0x2fac);
    P(0x439fa0) = Res_Load(0x2fad);
    P(0x439fa4) = Res_Load(0x2fb0);
    P(0x439f9c) = Res_Load(0x2faf);
    iVar4 = 0;
    do {
        P(0x43a124 + iVar4 * 4) = nullptr;
        iVar4 = iVar4 + 1;
    } while (iVar4 < 0x1c);
    iVar4 = 0;
    do {
        P(0x439f2c + iVar4 * 4) = nullptr;
        iVar4 = iVar4 + 1;
    } while (iVar4 < 0x1c);
    iVar4 = 0;
    do {
        D32(0x439e00 + iVar4 * 4) = iVar4;
        iVar4 = iVar4 + 1;
    } while (iVar4 < 0x4b);
    D32(0x439dfc) = 0;
    iVar4 = 0;
    do {
        iVar3 = Rand();
        uVar1 = D32(0x439e00 + (iVar3 % 0x4b) * 4);
        D32(0x439e00 + (iVar3 % 0x4b) * 4) = D32(0x439e00 + iVar4 * 4);
        D32(0x439e00 + iVar4 * 4) = uVar1;
        iVar4 = iVar4 + 1;
    } while (iVar4 < 0x4b);
    FUN_00402b7e(0x80c);
}

// 0x40c3d5: 1 if card param_1 may be picked (not covered by another card).
int FUN_0040c3d5(int param_1) {
    int uVar1;
    int iVar2;

    if (param_1 < 0x16) {
        iVar2 = 0;
        do {
            if ((D32(0x439fb0 + D32(0x4350dc + iVar2 * 12) * 4) != 0) &&
                ((param_1 == D32(0x4350e0 + iVar2 * 12)) || (param_1 == D32(0x4350e4 + iVar2 * 12)))) {
                return 0;
            }
            iVar2 = iVar2 + 1;
        } while (iVar2 < 0x14);
        if (((param_1 == 4) && (D32(0x439fe0) == 0)) && (D32(0x439fe4) == 0)) {
            uVar1 = 1;
        } else if (((param_1 == 7) && (D32(0x43a000) == 0)) && (D32(0x43a004) == 0)) {
            uVar1 = 1;
        } else if ((D32(0x439fb0 + D32(0x43502c + (0x15 - param_1) * 8) * 4) == 0) ||
                   (D32(0x439fb0 + D32(0x435030 + (0x15 - param_1) * 8) * 4) == 0)) {
            uVar1 = 1;
        } else {
            uVar1 = 0;
        }
    } else {
        uVar1 = 1;
    }
    return uVar1;
}

// 0x40c485: hide the cards (entering magnifier mode).
void FUN_0040c485(int param_1) {
    int iVar1;
    const void* local_8 = nullptr; // in_ECX in the original; always overwritten

    iVar1 = 0;
    do {
        if (D32(0x439fb0 + iVar1 * 4) != 0) {
            Spr_SetImage(D32(0x43a194 + iVar1 * 4), nullptr);
            Spr_GetImage(D32(0x439fb0 + iVar1 * 4), &local_8);
            P(0x43a020 + iVar1 * 4) = (void*)local_8;
            Spr_SetImage(D32(0x439fb0 + iVar1 * 4), nullptr);
        }
        iVar1 = iVar1 + 1;
    } while (iVar1 < 0x1c);
    Gfx_SaveBg(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    if (param_1 == 0) {
        local_8 = Ang_FrameImage(P(0x439fa0), 0);
        Spr_SetImage(D32(0x439fac), local_8);
    }
}

// 0x40c524: redraw the background and show the cards again.
void FUN_0040c524() {
    void* img;
    int iVar1;

    img = Res_Load(0x2fa8);
    Img_Draw(D32(0x436ee4), D32(0x436ee8), img, 0);
    Res_Free(img);
    Gfx_SaveBg(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    FUN_004032e3();
    iVar1 = 0;
    do {
        if (D32(0x439fb0 + iVar1 * 4) != 0) {
            Spr_SetImage(D32(0x43a194 + iVar1 * 4), P(0x43a124 + iVar1 * 4));
            Spr_SetImage(D32(0x439fb0 + iVar1 * 4), P(0x43a020 + iVar1 * 4));
        }
        iVar1 = iVar1 + 1;
    } while (iVar1 < 0x1c);
    Spr_SetImage(D32(0x439fac), nullptr);
}

// 0x40c5c8: create card sprites and hotspots.
void FUN_0040c5c8() {
    int x;
    int y;
    const void* img;
    int iVar1;
    int h;
    int iVar2;

    img = Ang_FrameImage(P(0x439f9c), 0);
    iVar2 = 0;
    do {
        D8(0x43a090 + iVar2) = 0;
        x = D32(0x434f4c + iVar2 * 8);
        y = D32(0x434f50 + iVar2 * 8);
        iVar1 = Spr_Create(img, x, y, iVar2 * 2 + 10);
        D32(0x439fb0 + iVar2 * 4) = iVar1;
        iVar1 = Spr_Create(P(0x43a124 + iVar2 * 4), x, y, iVar2 * 2 + 0xb);
        D32(0x43a194 + iVar2 * 4) = iVar1;
        iVar1 = Img_W(img);
        h = Img_H(img);
        Hotspot_Set((s16)iVar2 + 0x490, x, y, iVar1, h);
        iVar2 = iVar2 + 1;
    } while (iVar2 < 0x1c);
    D32(0x439df8) = 0x1c;
    D32(0x439df0) = 0;
}

// 0x40c6a2: set card back frame (0 normal, 1 selected).
void FUN_0040c6a2(int param_1, int param_2) {
    int iVar1;
    int iVar2;
    const void* img;

    if (D32(0x439fb0 + param_1 * 4) != 0) {
        D8(0x43a090 + param_1) = (u8)param_2;
        iVar1 = D32(0x434f4c + param_1 * 8);
        iVar2 = D32(0x434f50 + param_1 * 8);
        img = Ang_FrameImage(P(0x439f9c), param_2);
        Spr_SetImage(D32(0x439fb0 + param_1 * 4), img);
        Spr_SetPos(D32(0x439fb0 + param_1 * 4), iVar1, iVar2);
    }
}

// 0x40c70c: (re)create the magnifier sprite and its hotspot.
void FUN_0040c70c() {
    const void* img;
    int w;
    int h;
    Point local_c;

    if (D32(0x439fac) != 0) {
        Spr_Delete(D32(0x439fac), 1);
        D32(0x439fac) = 0;
        Hotspot_Remove(0x48f);
    }
    Ang_FrameInfo(P(0x439fa0), 1, &local_c);
    img = Ang_FrameImage(P(0x439fa0), 0);
    if (D32(0x43a204) == 1) {
        D32(0x439fac) = Spr_Create(img, local_c.x, local_c.y, 1);
    } else {
        D32(0x439fac) = Spr_Create(nullptr, local_c.x, local_c.y, 1);
    }
    w = Img_W(img);
    h = Img_H(img);
    Hotspot_Set(0x48f, local_c.x, local_c.y, w, h);
}

// 0x40c7de: show the text panel (full-screen hotspot).
void FUN_0040c7de() {
    const void* img;
    Point local_c;

    if (D32(0x439fac) != 0) {
        Spr_Delete(D32(0x439fac), 1);
        D32(0x439fac) = 0;
        Hotspot_Remove(0x48f);
    }
    Ang_FrameInfo(P(0x439fa4), 0, &local_c);
    img = Ang_FrameImage(P(0x439fa4), 0);
    D32(0x439fac) = Spr_Create(img, local_c.x, local_c.y, 99);
    FUN_004032e3();
    (void)Img_W(img);
    (void)Img_H(img);
    Hotspot_Set(0x48f, g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
}

// 0x40c89f: magnifier cursor following the mouse.
void FUN_0040c89f() {
    int iVar1;
    const void* img;
    Point local_14;
    int local_c;
    int local_8;

    local_c = g_mouseX;
    local_8 = g_mouseY;
    iVar1 = Pt_InRect(g_mouseX, g_mouseY, D32(0x434f3c), D32(0x434f40), D32(0x434f44), D32(0x434f48));
    if (iVar1 == 0) {
        Spr_SetImage(D32(0x434f38), nullptr);
        Cursor_Show(1);
    } else if (D32(0x43a204) == 1) {
        Cursor_Show(0);
        img = Ang_FrameImage(P(0x439fa8), 0);
        Spr_SetImage(D32(0x434f38), img);
        Pt_Set(&local_14, local_c + -0x14, local_8 + -0x14);
        Spr_SetPos(D32(0x434f38), local_14.x, local_14.y);
    } else {
        Spr_SetImage(D32(0x434f38), nullptr);
        Cursor_Show(1);
    }
}

// 0x40c977: event handler.
void FUN_0040c977(u16 param_1) {
    u32 uVar1;
    int iVar2;
    int iVar3;

    if (((D32(0x43a204) == 2) || (D32(0x43a204) == 3)) || (iVar2 = FUN_0040394c(param_1), iVar2 == 0)) {
        if (D32(0x43a204) == 1) {
            if (((param_1 == 0x48f) || (param_1 == 0xd)) || (param_1 == 0x20)) {
                D32(0x43a204) = 0;
                FUN_0040c524();
            } else if ((0x48f < param_1) && (param_1 < 0x4ad)) {
                D32(0x43a0e8) = param_1 - 0x490;
                D32(0x43a204) = 2;
                FUN_0040c7de();
                D32(0x43a208) = 1;
                FUN_00402bad(1, 2);
            }
        } else if (D32(0x43a204) == 2) {
            if ((D32(0x43a208) != 0) && (D32(0x43a208) = D32(0x43a208) + -1, D32(0x43a208) == 0)) {
                FUN_0040bff2();
                D32(0x43a204) = 3;
            }
        } else if (D32(0x43a204) == 3) {
            if (((param_1 == 0x48f) || (param_1 == 0xd)) || (param_1 == 0x20)) {
                D32(0x43a204) = 1;
                FUN_0040c70c();
            }
        } else if (D32(0x439df8) == 0) {
            FUN_00402bad(5, 2);
            if (D32(0x439de8) == 2) {
                Video_Play(DSTR(0x4351d4), D32(0x436ee4), D32(0x436ee8), LAB_0040bdf8);
                Time_Delay(400);
            }
            D32(0x439dec) = 1;
        } else if (D32(0x439df0) < 0x1e) {
            if (D32(0x439df4) == 2) {
                iVar2 = 0;
                do {
                    if (((D32(0x439fb0 + iVar2 * 4) != 0) && (D8(0x43a090 + iVar2) != 0)) &&
                        ((u32)D8(0x43a0e4) != (u32)((int)(u32)D8(0x43a0c8 + iVar2) >> 2))) {
                        D8(0x43a0e4) = 99;
                        FUN_00402bad(3, 2);
                        FUN_00404068(0);
                        break;
                    }
                    iVar2 = iVar2 + 1;
                } while (iVar2 < 0x1c);
                iVar2 = 0;
                do {
                    if ((D32(0x439fb0 + iVar2 * 4) != 0) && (D8(0x43a090 + iVar2) != 0)) {
                        D8(0x43a090 + iVar2) = 0;
                        D32(0x439df4) = D32(0x439df4) + -1;
                        if ((u32)D8(0x43a0e4) == (u32)((int)(u32)D8(0x43a0c8 + iVar2) >> 2)) {
                            Spr_Delete(D32(0x43a194 + iVar2 * 4), 1);
                            D32(0x43a194 + iVar2 * 4) = 0;
                            Spr_Delete(D32(0x439fb0 + iVar2 * 4), 1);
                            D32(0x439fb0 + iVar2 * 4) = 0;
                            Hotspot_Remove((s16)iVar2 + 0x490);
                            D32(0x439df8) = D32(0x439df8) + -1;
                            if (D32(0x439df4) == 0) {
                                FUN_00404068(1);
                                FUN_00402bad(4, 2);
                            }
                        } else {
                            FUN_0040c6a2(iVar2, (u32)D8(0x43a090 + iVar2));
                        }
                    }
                    iVar2 = iVar2 + 1;
                } while (iVar2 < 0x1c);
                if ((D32(0x439df8) != 0) && (iVar2 = FUN_0040bf68(), iVar2 == 0)) {
                    D32(0x439df0) = 99;
                }
            }
            if (param_1 == 0x48f) {
                D32(0x43a204) = 1;
                FUN_0040c485(0);
            } else if ((0x48f < param_1) && (param_1 < 0x4ad)) {
                uVar1 = (u32)param_1;
                iVar2 = uVar1 - 0x490;
                // 0x439c00 + id == 0x43a090 + card; 0x439c38 + id == 0x43a0c8 + card
                if (D8(0x43a090 + iVar2) == 1) {
                    D8(0x43a090 + iVar2) = 0;
                    D32(0x439df4) = D32(0x439df4) + -1;
                    FUN_00402bad(0, 1);
                } else {
                    iVar3 = FUN_0040c3d5(iVar2);
                    if (iVar3 == 0) {
                        FUN_00402bad(2, 1);
                    } else {
                        D8(0x43a090 + iVar2) = (u8)(1 - D8(0x43a090 + iVar2));
                        D32(0x439df4) = D32(0x439df4) + 1;
                        D8(0x43a0e4) = (u8)((int)(u32)D8(0x43a0c8 + iVar2) >> 2);
                        D32(0x43a0e8) = iVar2;
                        FUN_00402bad(0, 1);
                    }
                }
                FUN_0040c6a2(iVar2, (u32)D8(0x43a090 + iVar2));
            }
        } else {
            FUN_00402e72(0x15c7, nullptr, 0, 1, 1);
            iVar2 = 0;
            do {
                if (D32(0x439fb0 + iVar2 * 4) != 0) {
                    Spr_Delete(D32(0x43a194 + iVar2 * 4), 1);
                    D32(0x43a194 + iVar2 * 4) = 0;
                    Spr_Delete(D32(0x439fb0 + iVar2 * 4), 1);
                    D32(0x439fb0 + iVar2 * 4) = 0;
                    Hotspot_Remove((s16)iVar2 + 0x490);
                }
                iVar2 = iVar2 + 1;
            } while (iVar2 < 0x1c);
            FUN_0040c5c8();
        }
    }
}

// 0x40cd60
bool FUN_0040cd60() { return 2 < D32(0x439de8); }

// 0x40cd72
void FUN_0040cd72() { Hotspot_Clear(); }

// 0x40cd7c
void FUN_0040cd7c() {
    if (D32(0x439de8) < 2) {
        Fx_CloseBlack();
    }
}

// 0x40cd8f: play one round.
void FUN_0040cd8f() {
    bool bVar1;
    u32 uVar2;

    D32(0x43a204) = 0;
    FUN_0040c07c();
    FUN_0040c198();
    D32(0x439df4) = 0;
    Spr_Update(1);
    if (D32(0x439de8) == 0) {
        FUN_0040c485(1);
        FUN_0040658a();
        FUN_0040c524();
    }
    D32(0x439dec) = 0;
    Timer_Set(0, 0x32);
    Timer_Set(1, 200);
    Event_Flush();
    do {
        while (true) {
            bVar1 = Timer_Expired(0);
            if (bVar1) break;
            uVar2 = (u32)Event_Poll();
            FUN_0040c977((u16)uVar2);
            FUN_0040c89f();
            if (D32(0x43a204) != 3) {
                Spr_Update(1);
            }
        }
        Timer_Set(0, 0x32);
    } while (D32(0x439dec) == 0);
    if (D32(0x439df8) == 0) {
        FUN_00403e2c(0x15e);
        FUN_00419330(1);
        FUN_0040cd7c();
    }
    FUN_0040cd72();
}

// 0x40ce63: module 10 entry.
void FUN_0040ce63() {
    bool bVar1;

    FUN_0040c30a();
    D32(0x439de8) = 0;
    do {
        FUN_0040cd8f();
        D32(0x439de8) = D32(0x439de8) + 1;
        bVar1 = FUN_0040cd60();
    } while (!bVar1);
    FUN_004040f5(1);
}

// 0x40ce90: module 10 cleanup.
void FUN_0040ce90() {
    FUN_0040cd72();
    FUN_0040c297();
    Spr_DeleteAll(0, 0);
    FUN_00405901();
}

// 0x40ceb0: Ms. Grunkle's introduction (param_1: first visit).
void FUN_0040ceb0(int param_1) {
    if (param_1 == 0) {
        FUN_00402ec2(D32(0x4351cc) + 0x15c0, 0, 1);
        D32(0x4351cc) = D32(0x4351cc) + 1;
        if (2 < D32(0x4351cc)) {
            D32(0x4351cc) = 0;
        }
    } else {
        FUN_00402ec2(0x15bf, 0, 1);
    }
}
