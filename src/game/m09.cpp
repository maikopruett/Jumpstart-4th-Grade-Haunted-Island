// Module 9 (0x40f7bc..0x411524): a 6x6 isometric planting grid. The player
// picks one of up to two plant kinds (hotspots 0x524+), places them on the
// grid cells (0x530 + col * 6 + row), adjusts two levers (0x521/0x522) and
// runs up to three growth trials (0x520); the harvest must reach the
// per-plant targets of the puzzle loaded by FUN_004102cb.
//
// Data segment layout used by this module:
//   0x435ad0..0x435adc ptr  sprite sheets (offset tables, P)
//   0x435ae0 ptr[2]         lever animation sheets (P)
//   0x435ae8, 0x435af4, 0x435af8, 0x435afc, 0x435b00 ptr  misc sheets (P)
//   0x435aec ptr[n]         plant growth sheets, n = D32(0x43a710) <= 2 (P)
//   0x435b04..0x435b24      sprite ids; 0x435b28 int[36] cell sprites
//   0x435bb8 int            toggle sprite; 0x435bbc int[n] plant anim sprites
//   0x435bc4 int            puzzle index; 0x435bc8/0x435bcc flags
//   0x43a70c int            round (0..2)
//   0x43a710 0x8c bytes     puzzle record: +0 count, +4 lever 1 weights,
//                           +0x10 lever 2 weights, +0x1c neighbour weights,
//                           (+4/+0x10/+0x1c: per plant stride 0x40, neighbour
//                           weights [kind * 5 + count]), +0x84 int[n] targets
//   0x43a79c trials, 0x43a7a0 trial number (0 = editing), 0x43a7a4/0x43a7a8
//   lever states, 0x43a7ac selected plant
//   0x43a7b0 / 0x43a8d0 / 0x43a9f0  6x6 cells {int plant, int stage},
//                           cell (col, row) at + col * 8 + row * 0x30
//   0x43ab10 int[2] harvest; 0x43ab18 int; 0x43ab1c tool (0 none, 1 plant,
//   2 remove); 0x43ab20 Point[6][6] cell positions (same indexing)
//   0x43ac40 int[4] shuffled plant images; 0x43ac50 int[n] anim ids
#include "game/common.h"

// Image idx of a resource that starts with a table of s32 offsets.
static inline const void* SubImg(void* base, int idx) {
    return (u8*)base + RdS32((u8*)base + idx * 4);
}

// 0x40f7bc: module 9 entry.
void FUN_0040f7bc() {
    bool bVar1;

    FUN_0040fe5d();
    D32(0x43a70c) = 0;
    do {
        FUN_0040f7e9();
        D32(0x43a70c) = D32(0x43a70c) + 1;
        bVar1 = FUN_0040fc9a();
    } while (!bVar1);
    FUN_004040f5(1);
}

// 0x40f7e9: play one round.
void FUN_0040f7e9() {
    bool bVar2;
    u32 uVar3;
    int iVar4;
    int iVar5;
    int iVar6;
    u16 uVar7;
    int local_10;
    int local_c;

    bVar2 = false;
    FUN_0040ff83();
    FUN_00410389();
    if (D32(0x43a70c) == 0) {
        FUN_0040658a();
    }
    for (;;) {
        // LAB_0040fc5a
        if (bVar2) {
            FUN_00403e2c(0x15e);
            FUN_00419330(1);
            FUN_00404068(1);
            D32(0x435bc8) = 1;
            FUN_0040fd90();
            Fx_CloseBlack();
            return;
        }
        uVar3 = (u32)Event_Poll();
        uVar7 = (u16)uVar3;
        FUN_0040394c(uVar7);
        FUN_004107cf();
        if ((uVar7 == 0x521) && (D32(0x43a7a0) == 0)) {
            FUN_004109c5();
            continue;
        }
        if ((uVar7 == 0x522) && (D32(0x43a7a0) == 0)) {
            FUN_00410b5f();
            continue;
        }
        if (uVar7 == 0x523) {
            if (D32(0x43ab1c) == 2) {
                D32(0x43ab1c) = 0;
                Spr_SetImage(D32(0x435bb8), P(0x435af8));
                Spr_Update(1);
            } else {
                D32(0x43ab1c) = 2;
                Spr_SetImage(D32(0x435bb8), nullptr);
                Spr_Update(1);
            }
            continue;
        }
        if ((uVar7 >= 0x524) && ((uVar3 & 0xffff) < (u32)D32(0x43a710) + 0x524U) && (D32(0x43a7a0) == 0)) {
            iVar4 = (int)(uVar3 & 0xffff) - 0x524;
            if ((D32(0x43ab1c) == 1) && (iVar4 == D32(0x43a7ac))) {
                D32(0x43ab1c) = 0;
            } else {
                FUN_00402bad(2, 1);
                if (D32(0x43ab1c) == 2) {
                    Spr_SetImage(D32(0x435bb8), P(0x435af8));
                }
                FUN_0041130a(iVar4);
                D32(0x43ab1c) = 1;
                D32(0x43a7ac) = iVar4;
            }
            continue;
        }
        if (((0x52f < uVar7) && (uVar7 < 0x554)) && (D32(0x43a7a0) == 0)) {
            // click on a grid cell
            if (D32(0x43ab1c) == 0) {
                FUN_00402bad(10, 1);
                FUN_00402e72(0x15a0, nullptr, 0, 1, 1);
                continue;
            }
            iVar5 = (int)((uVar3 & 0xffff) - 0x530) / 6;
            iVar6 = (int)((uVar3 & 0xffff) - 0x530) % 6;
            local_10 = D32(0x43a7b0 + iVar5 * 8 + iVar6 * 0x30);
            local_c = D32(0x43a7b4 + iVar6 * 0x30 + iVar5 * 8);
            if (D32(0x43ab1c) == 1) {
                if (local_c != 0) {
                    FUN_00402bad(10, 1);
                    continue;
                }
                FUN_00402bad(3, 1);
                local_10 = D32(0x43a7ac);
                local_c = 1;
            } else {
                if (local_c == 0) {
                    FUN_00402bad(10, 1);
                    continue;
                }
                FUN_00402bad(4, 1);
                local_c = 0;
            }
            // LAB_0040fa3d
            iVar4 = iVar6 * 0x30;
            D32(0x43a7b0 + iVar5 * 8 + iVar4) = local_10;
            D32(0x43a7b4 + iVar4 + iVar5 * 8) = local_c;
            D32(0x43a8d0 + iVar6 * 0x30 + iVar5 * 8) = D32(0x43a7b0 + iVar5 * 8 + iVar4);
            D32(0x43a8d4 + iVar6 * 0x30 + iVar5 * 8) = D32(0x43a7b4 + iVar5 * 8 + iVar4);
            FUN_00410d55();
            continue;
        }
        if (uVar7 == 0x520) {
            D32(0x435bcc) = 1;
            Spr_SetImage(D32(0x435b08), SubImg(P(0x435ad0), 1));
            Spr_Update(1);
            if (D32(0x43ab1c) == 2) {
                Spr_SetImage(D32(0x435bb8), P(0x435af8));
                Spr_Update(1);
            }
            D32(0x43ab1c) = 0;
            if (D32(0x43a7a0) == 0) {
                FUN_00402bad(5, 2);
                D32(0x43a79c) = D32(0x43a79c) + 1;
                FUN_004111ec();
                FUN_004112c3();
                for (D32(0x43a7a0) = 1; D32(0x43a7a0) < 4; D32(0x43a7a0) = D32(0x43a7a0) + 1) {
                    FUN_00402bad(6, 1);
                    iVar4 = FUN_00410e99();
                    FUN_004113ab();
                    FUN_004104d5(1);
                    if (iVar4 == 0) break;
                }
                FUN_00411235();
                FUN_004112f5();
                iVar4 = FUN_0041141d();
                if (iVar4 == 0) {
                    FUN_00402bad(9, 4);
                    FUN_00402e72(0x15a2, nullptr, 0, 1, 1);
                    Spr_SetImage(D32(0x435b0c), SubImg(P(0x435ad0), 3));
                    Spr_Update(1);
                } else {
                    FUN_00402bad(8, 4);
                    FUN_00402e72(0x15a1, nullptr, 0, 1, 1);
                    bVar2 = true;
                }
            } else {
                FUN_00402bad(7, 2);
                D32(0x43a7a0) = 0;
                memset(DPTR(0x43ab10), 0, 8);
                memmove(DPTR(0x43a8d0), DPTR(0x43a7b0), 0x120);
                Spr_SetImage(D32(0x435b0c), SubImg(P(0x435ad0), 2));
                FUN_00410d55();
                FUN_004104d5(1);
            }
            Spr_SetImage(D32(0x435b08), SubImg(P(0x435ad0), 0));
            Spr_Update(1);
        } else if (uVar7 == 0xc) {
            FUN_00403f95();
        }
    }
}

// 0x40fc9a
bool FUN_0040fc9a() { return 2 < D32(0x43a70c); }

// 0x40fcac: module 9 cleanup.
void FUN_0040fcac() {
    int iVar1;

    FUN_0040fd90();
    Hotspot_Clear();
    Res_Free(P(0x435ad0));
    P(0x435ad0) = nullptr;
    Res_Free(P(0x435ad4));
    P(0x435ad4) = nullptr;
    Res_Free(P(0x435ad8));
    P(0x435ad8) = nullptr;
    Res_Free(P(0x435adc));
    P(0x435adc) = nullptr;
    iVar1 = 0;
    do {
        Res_Free(P(0x435ae0 + iVar1 * 4));
        P(0x435ae0 + iVar1 * 4) = nullptr;
        iVar1 = iVar1 + 1;
    } while (iVar1 < 2);
    Res_Free(P(0x435ae8));
    P(0x435ae8) = nullptr;
    Res_Free(P(0x435af4));
    P(0x435af4) = nullptr;
    Res_Free(P(0x435afc));
    P(0x435afc) = nullptr;
    Res_Free(P(0x435af8));
    P(0x435af8) = nullptr;
    Res_Free(P(0x435b00));
    P(0x435b00) = nullptr;
    FUN_00405901();
}

// 0x40fd90: end of round: free per-round resources, update progress.
void FUN_0040fd90() {
    int uVar1;
    u32 uVar2;

    Spr_DeleteAll(0, 0);
    for (uVar2 = 0; uVar2 < (u32)D32(0x43a710); uVar2 = uVar2 + 1) {
        Res_Free(P(0x435aec + uVar2 * 4));
        P(0x435aec + uVar2 * 4) = nullptr;
        Anim_Stop(D32(0x43ac50 + uVar2 * 4));
        Anim_Free(D32(0x43ac50 + uVar2 * 4));
    }
    if ((D32(0x435bc8) == 0) && (D32(0x435bcc) != 0)) {
        uVar1 = g_module;
        g_module = 9;
        FUN_00404068(0);
        g_module = uVar1;
        if (D8(0x439805) == D8(0x439804)) {
            D8(0x439804) = D8(0x439804) + 1;
        }
        D32(0x435bc8) = 1;
    } else {
        if ((int)(u32)D8(0x439804) == D32(0x435bc4)) {
            D8(0x439804) = D8(0x439804) + 1;
        }
        if ((int)(u32)D8(0x439805) == D32(0x435bc4)) {
            D8(0x439805) = D8(0x439804);
        }
    }
}

// 0x40fe5d: module initialisation.
void FUN_0040fe5d() {
    void* pvVar1;
    int iVar2;
    int iVar3;
    int iVar4;
    Point local_c;

    FUN_00402c78(0x7f6);
    FUN_00402b7e(0x805);
    P(0x435ad0) = Res_Load(0x322);
    P(0x435ad4) = Res_Load(0x323);
    P(0x435ad8) = Res_Load(0x324);
    P(0x435adc) = Res_Load(0x325);
    iVar3 = 0;
    do {
        pvVar1 = Res_Load(iVar3 + 0x32a);
        P(0x435ae0 + iVar3 * 4) = pvVar1;
        iVar3 = iVar3 + 1;
    } while (iVar3 < 2);
    P(0x435ae8) = Res_Load(0x32c);
    P(0x435af4) = Res_Load(0x32d);
    P(0x435afc) = Res_Load(0x32e);
    P(0x435af8) = Res_Load(0x32f);
    P(0x435b00) = Res_Load(0x332);
    FUN_00403e91();
    iVar4 = 0x6d;
    Pt_Set(&local_c, 0x6d, 0xea);
    iVar3 = 0;
    do {
        iVar2 = 0;
        do {
            D32(0x43ab20 + iVar2 * 0x30 + iVar3 * 8) = local_c.x;
            D32(0x43ab24 + iVar2 * 0x30 + iVar3 * 8) = local_c.y;
            local_c.x = local_c.x + 0x36;
            iVar2 = iVar2 + 1;
        } while (iVar2 < 6);
        iVar4 = iVar4 + 0x19;
        local_c.y = local_c.y + 0x21;
        iVar3 = iVar3 + 1;
        local_c.x = iVar4;
    } while (iVar3 < 6);
}

// 0x40ff83: round setup: state, puzzle, sprites.
void FUN_0040ff83() {
    void* pvVar1;
    int iVar2;
    int iVar3;
    int iVar4;
    u32 uVar5;
    Point local_c;

    D32(0x435bc8) = 0;
    D32(0x435bcc) = 0;
    D32(0x43a7a0) = 0;
    D32(0x43a79c) = 0;
    D32(0x43a7ac) = -1;
    D32(0x43a7a4) = 0;
    D32(0x43a7a8) = 0;
    memset(DPTR(0x43a7b0), 0, 0x120);
    memmove(DPTR(0x43a8d0), DPTR(0x43a7b0), 0x120);
    memset(DPTR(0x43ab10), 0, 8);
    D32(0x43ab1c) = 0;
    FUN_004102cb();
    iVar4 = 0;
    do {
        D32(0x43ac40 + iVar4 * 4) = iVar4;
        iVar4 = iVar4 + 1;
    } while (iVar4 < 4);
    Util_Shuffle(DPTR(0x43ac40), 4, 4);
    for (uVar5 = 0; uVar5 < (u32)D32(0x43a710); uVar5 = uVar5 + 1) {
        pvVar1 = Res_Load(D32(0x43ac40 + uVar5 * 4) + 0x326);
        P(0x435aec + uVar5 * 4) = pvVar1;
        D32(0x43ac50 + uVar5 * 4) = uVar5 * 4 + D32(0x43ac40 + uVar5 * 4) + 0x333;
        Anim_Load(D32(0x43ac50 + uVar5 * 4));
    }
    D32(0x435b04) = Spr_Create(nullptr, D32(0x436ee4), D32(0x436ee8), 1000);
    Pt_Set(&local_c, D32(0x435be0), D32(0x435be4));
    D32(0x435b08) = Spr_Create(SubImg(P(0x435ad0), 0), local_c.x, local_c.y, 0x6e);
    D32(0x435b0c) = Spr_Create(SubImg(P(0x435ad0), 2), D32(0x435bf0), D32(0x435bf4), 0x6f);
    Pt_Set(&local_c, D32(0x435bf8), D32(0x435bfc));
    D32(0x435b10) = Spr_Create(SubImg(P(0x435ad4), 0), local_c.x, local_c.y, 0x71);
    Pt_Set(&local_c, D32(0x435c08), D32(0x435c0c));
    D32(0x435b14) = Spr_Create(SubImg(P(0x435ad8), 0), local_c.x, local_c.y, 0x6e);
    local_c.x = D32(0x435c18);
    local_c.y = D32(0x435c1c);
    D32(0x435b18) = Spr_Create(SubImg(P(0x435adc), 0), D32(0x435c18), D32(0x435c1c), 0x70);
    iVar4 = 0;
    do {
        iVar2 = Spr_Create(nullptr, D32(0x435c40 + iVar4 * 8), D32(0x435c44 + iVar4 * 8), 0x14);
        D32(0x435b1c + iVar4 * 4) = iVar2;
        iVar4 = iVar4 + 1;
    } while (iVar4 < 2);
    D32(0x435b24) = Spr_Create(nullptr, D32(0x435c50), D32(0x435c54), 0x71);
    iVar4 = 0;
    do {
        iVar2 = 0;
        do {
            local_c.x = D32(0x43ab20 + iVar4 * 0x30 + iVar2 * 8) + -0x2a;
            local_c.y = D32(0x43ab24 + iVar4 * 0x30 + iVar2 * 8) + -0x6e;
            iVar3 = Spr_Create(nullptr, local_c.x, local_c.y, (iVar2 * 6 + 0x78) - iVar4);
            D32(0x435b28 + (iVar4 * 6 + iVar2) * 4) = iVar3;
            iVar2 = iVar2 + 1;
        } while (iVar2 < 6);
        iVar4 = iVar4 + 1;
    } while (iVar4 < 6);
    Pt_Set(&local_c, D32(0x435c68), D32(0x435c6c));
    D32(0x435bb8) = Spr_Create(P(0x435af8), local_c.x, local_c.y, 0x6e);
    for (uVar5 = 0; uVar5 < (u32)D32(0x43a710); uVar5 = uVar5 + 1) {
        iVar4 = Spr_Create(nullptr, D32(0x436ee4), D32(0x436ee8), uVar5 + 10);
        D32(0x435bbc + uVar5 * 4) = iVar4;
    }
}

// 0x4102cb: choose and load the puzzle record into 0x43a710.
void FUN_004102cb() {
    u32 uVar1;
    int iVar2;
    void* p;
    int local_10[3];

    local_10[0] = D32(0x435c98);
    local_10[1] = D32(0x435c9c);
    local_10[2] = D32(0x435ca0);
    uVar1 = FUN_004040a2();
    if (((int)uVar1 < 2) && ((int)(u32)D8(0x439804) < D32(0x435ac4 + uVar1 * 4))) {
        if ((int)(u32)D8(0x439805) == D32(0x435bc4)) {
            D32(0x435bc4) = (u32)D8(0x439804);
        } else {
            D32(0x435bc4) = (u32)D8(0x439805);
        }
    } else if (D32(0x435ac4 + uVar1 * 4) == 0) {
        D32(0x435bc4) = 0;
    } else {
        iVar2 = Rand();
        D32(0x435bc4) = iVar2 % D32(0x435ac4 + uVar1 * 4);
    }
    p = Res_Load(local_10[uVar1] + D32(0x435bc4));
    memcpy(DPTR(0x43a710), p, 0x23 * 4);
    Res_Free(p);
}

// 0x410389: draw the scene.
void FUN_00410389() {
    void* pvVar1;
    u32 uVar2;
    Point local_c;

    pvVar1 = Res_Load(800);
    Img_Draw(D32(0x436ee4), D32(0x436ee8), pvVar1, 0);
    Res_Free(pvVar1);
    for (uVar2 = 0; uVar2 < (u32)D32(0x43a710); uVar2 = uVar2 + 1) {
        pvVar1 = Res_Load(uVar2 + 0x330);
        Pt_Set(&local_c, D32(0x435c20 + uVar2 * 16), D32(0x435c24 + uVar2 * 16));
        Img_Draw(local_c.x, local_c.y, pvVar1, 1);
        Res_Free(pvVar1);
        Img_Draw(D32(0x435c78 + uVar2 * 8), D32(0x435c7c + uVar2 * 8),
                 SubImg(P(0x435b00), uVar2 * 4 + D32(0x43ac40 + uVar2 * 4)), 0);
    }
    FUN_004104d5(0);
    Gfx_SaveBg(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    FUN_004032e3();
    Spr_InvalidateAll();
    Spr_Update(0);
    if (D32(0x43a70c) == 0) {
        Screen_FlushAll();
        FUN_004104af();
        FUN_00410681();
    } else {
        Fx_OpenCenter();
    }
}

// 0x4104af: fade in the palette.
void FUN_004104af() {
    void* rgb;

    rgb = Res_Load(0x321);
    Pal_FadeTo((const u8*)rgb, 1);
    Mem_Free(rgb);
}

// 0x4104d5: draw the needed / yield / trials panel.
void FUN_004104d5(int param_1) {
    u32 uVar1;
    char local_5c[80];
    int local_c;
    int local_8;
    Point pt;

    Gfx_FillRect(D32(0x435c58), D32(0x435c5c), D32(0x435c60), D32(0x435c64), 0x24);
    Pt_Set(&pt, 0x206, 0x39);
    local_c = pt.x;
    local_8 = pt.y;
    Text_DrawScreenFont(local_c, local_8, DSTR(0x435dc9), P(0x4318d8), 0x33);
    local_c = 0x21a;
    for (uVar1 = 0; local_8 = local_8 + 0xc, uVar1 < (u32)D32(0x43a710); uVar1 = uVar1 + 1) {
        sprintf(local_5c, DSTR(0x435dce), D32(0x43a794 + uVar1 * 4),
                (char*)P(0x435c88 + D32(0x43ac40 + uVar1 * 4) * 4));
        Text_DrawScreenFont(local_c, local_8, local_5c, P(0x4318d8), 0x33);
    }
    local_c = 0x206;
    Text_DrawScreenFont(0x206, local_8, DSTR(0x435dd4), P(0x4318d8), 0x33);
    local_c = 0x21a;
    for (uVar1 = 0; local_8 = local_8 + 0xc, uVar1 < (u32)D32(0x43a710); uVar1 = uVar1 + 1) {
        sprintf(local_5c, DSTR(0x435dda), D32(0x43ab10 + uVar1 * 4),
                (char*)P(0x435c88 + D32(0x43ac40 + uVar1 * 4) * 4));
        Text_DrawScreenFont(local_c, local_8, local_5c, P(0x4318d8), 0x33);
    }
    local_c = 0x206;
    sprintf(local_5c, DSTR(0x435de0), D32(0x43a79c));
    Text_DrawScreenFont(local_c, local_8, local_5c, P(0x4318d8), 0x33);
    if (param_1 != 0) {
        Gfx_SaveBg(D32(0x435c58), D32(0x435c5c), D32(0x435c60), D32(0x435c64));
        Screen_Flush(D32(0x435c58), D32(0x435c5c), D32(0x435c60), D32(0x435c64));
    }
}

// 0x410681: hotspots.
void FUN_00410681() {
    int iVar1;
    u32 uVar2;
    int iVar3;
    int local_14[4];

    memcpy(local_14, DPTR(0x435ca4), sizeof(local_14));
    Hotspot_Set(0x521, D32(0x435bf8), D32(0x435bfc), D32(0x435c00), D32(0x435c04));
    Hotspot_Set(0x522, D32(0x435c08), D32(0x435c0c), D32(0x435c10), D32(0x435c14));
    for (uVar2 = 0; uVar2 < (u32)D32(0x43a710); uVar2 = uVar2 + 1) {
        Hotspot_Set((s16)uVar2 + 0x524, D32(0x435c20 + uVar2 * 16), D32(0x435c24 + uVar2 * 16),
                    D32(0x435c28 + uVar2 * 16), D32(0x435c2c + uVar2 * 16));
    }
    Hotspot_Set(0x523, D32(0x435c68), D32(0x435c6c), D32(0x435c70), D32(0x435c74));
    Hotspot_Set(0x520, D32(0x435be0), D32(0x435be4), D32(0x435be8), D32(0x435bec));
    iVar1 = 0;
    do {
        iVar3 = 0;
        do {
            local_14[0] = D32(0x43ab20 + iVar3 * 0x30 + iVar1 * 8) + -0x1b;
            local_14[1] = D32(0x43ab24 + iVar3 * 0x30 + iVar1 * 8) + -0x10;
            Hotspot_Set((s16)((s16)iVar1 * 6 + (s16)iVar3 + 0x530), local_14[0], local_14[1], local_14[2],
                        local_14[3]);
            iVar3 = iVar3 + 1;
        } while (iVar3 < 6);
        iVar1 = iVar1 + 1;
    } while (iVar1 < 6);
}

// 0x4107cf: tool cursor following the mouse.
void FUN_004107cf() {
    int iVar1;
    u32 uVar2;
    int iVar3;
    const void* img;
    Point local_14;
    int local_c;
    int local_8;

    local_c = g_mouseX;
    local_8 = g_mouseY;
    iVar1 = Pt_InRect(g_mouseX, g_mouseY, D32(0x435bd0), D32(0x435bd4), D32(0x435bd8), D32(0x435bdc));
    if (iVar1 == 0) {
        Spr_SetImage(D32(0x435b04), nullptr);
        Spr_Update(1);
        Cursor_Show(1);
    } else if (D32(0x43ab1c) == 0) {
        Spr_SetImage(D32(0x435b04), nullptr);
        Spr_Update(1);
        Cursor_Show(1);
    } else {
        iVar1 = Pt_InRect(local_c, local_8, D32(0x435be0), D32(0x435be4), D32(0x435be8), D32(0x435bec));
        if (((iVar1 == 0) &&
             (iVar1 = Pt_InRect(local_c, local_8, D32(0x435c08), D32(0x435c0c), D32(0x435c10), D32(0x435c14)),
              iVar1 == 0)) &&
            (iVar1 = Pt_InRect(local_c, local_8, D32(0x435bf8), D32(0x435bfc), D32(0x435c00), D32(0x435c04)),
             iVar1 == 0)) {
            Cursor_Show(0);
            img = P(0x435afc);
            if (D32(0x43ab1c) == 1) {
                img = SubImg(P(0x435af4), D32(0x43ac40 + D32(0x43a7ac) * 4));
            }
            Spr_SetImage(D32(0x435b04), img);
            iVar1 = Img_H(img);
            iVar1 = local_8 - iVar1;
            uVar2 = (u32)Img_W(img);
            iVar3 = (int)uVar2 >> 1;
            if (iVar3 < 0) {
                iVar3 = iVar3 + (u32)((uVar2 & 1) != 0);
            }
            Pt_Set(&local_14, local_c - iVar3, iVar1);
            Spr_SetPos(D32(0x435b04), local_14.x, local_14.y);
            Spr_Update(1);
        } else {
            Spr_SetImage(D32(0x435b04), nullptr);
            Spr_Update(1);
            Cursor_Show(1);
        }
    }
}

// 0x410999: wait param_1 ms while handling the toolbar.
void FUN_00410999(int param_1) {
    bool bVar1;
    u32 uVar2;

    Timer_Set(1, param_1);
    while (true) {
        bVar1 = Timer_Expired(1);
        if (bVar1) break;
        uVar2 = (u32)Event_Poll();
        FUN_0040394c((u16)uVar2);
    }
}

// 0x4109c5: lever 1 (states 0 -> 1 -> 2 -> 0).
void FUN_004109c5() {
    FUN_00402bad(0, 1);
    if (D32(0x43a7a4) == 0) {
        Spr_SetImage(D32(0x435b10), SubImg(P(0x435ad4), 1));
        Spr_Update(1);
        FUN_00410999(100);
        Spr_SetImage(D32(0x435b10), SubImg(P(0x435ad4), 2));
        Spr_Update(1);
        D32(0x43a7a4) = 1;
        return;
    }
    if (D32(0x43a7a4) != 1) {
        if (D32(0x43a7a4) != 2) {
            return;
        }
        Spr_SetImage(D32(0x435b10), SubImg(P(0x435ad4), 3));
        Spr_Update(1);
        FUN_00410999(100);
        Spr_SetImage(D32(0x435b10), SubImg(P(0x435ad4), 2));
        Spr_Update(1);
        FUN_00410999(100);
        Spr_SetImage(D32(0x435b10), SubImg(P(0x435ad4), 1));
        Spr_Update(1);
        FUN_00410999(100);
        Spr_SetImage(D32(0x435b10), SubImg(P(0x435ad4), 0));
        Spr_Update(1);
        D32(0x43a7a4) = 0;
        return;
    }
    Spr_SetImage(D32(0x435b10), SubImg(P(0x435ad4), 3));
    Spr_Update(1);
    FUN_00410999(100);
    Spr_SetImage(D32(0x435b10), SubImg(P(0x435ad4), 4));
    Spr_Update(1);
    D32(0x43a7a4) = 2;
}

// 0x410b5f: lever 2 (states 0 -> 1 -> 2 -> 0).
void FUN_00410b5f() {
    FUN_00402bad(1, 1);
    if (D32(0x43a7a8) == 0) {
        Spr_SetImage(D32(0x435b14), SubImg(P(0x435ad8), 1));
        Spr_Update(1);
        FUN_00410999(100);
        Spr_SetImage(D32(0x435b14), SubImg(P(0x435ad8), 2));
        Spr_Update(1);
        FUN_00410999(100);
        Spr_SetImage(D32(0x435b14), SubImg(P(0x435ad8), 3));
        Spr_SetImage(D32(0x435b18), SubImg(P(0x435adc), 1));
        Spr_Update(1);
        D32(0x43a7a8) = 1;
        return;
    }
    if (D32(0x43a7a8) != 1) {
        if (D32(0x43a7a8) != 2) {
            return;
        }
        Spr_SetImage(D32(0x435b14), SubImg(P(0x435ad8), 7));
        Spr_Update(1);
        FUN_00410999(100);
        Spr_SetImage(D32(0x435b14), SubImg(P(0x435ad8), 0));
        Spr_SetImage(D32(0x435b18), SubImg(P(0x435adc), 0));
        Spr_Update(1);
        D32(0x43a7a8) = 0;
        return;
    }
    Spr_SetImage(D32(0x435b14), SubImg(P(0x435ad8), 4));
    Spr_Update(1);
    FUN_00410999(100);
    Spr_SetImage(D32(0x435b14), SubImg(P(0x435ad8), 5));
    Spr_Update(1);
    FUN_00410999(100);
    Spr_SetImage(D32(0x435b14), SubImg(P(0x435ad8), 6));
    Spr_SetImage(D32(0x435b18), SubImg(P(0x435adc), 2));
    Spr_Update(1);
    D32(0x43a7a8) = 2;
}

// 0x410d55: draw the plants of grid 0x43a8d0.
void FUN_00410d55() {
    int iVar1;
    int iVar2;
    int iVar3;

    iVar3 = 0;
    do {
        iVar2 = 0;
        do {
            iVar1 = D32(0x43a8d0 + iVar2 * 0x30 + iVar3 * 8);
            int spr = D32(0x435b28 + (iVar2 * 6 + iVar3) * 4);
            switch (D32(0x43a8d4 + iVar2 * 0x30 + iVar3 * 8)) {
            case 0:
                Spr_SetImage(spr, nullptr);
                break;
            case 1:
                Spr_SetImage(spr, SubImg(P(0x435aec + iVar1 * 4), 0));
                break;
            case 2:
                Spr_SetImage(spr, SubImg(P(0x435aec + iVar1 * 4), 4));
                break;
            case 3:
                Spr_SetImage(spr, SubImg(P(0x435aec + iVar1 * 4), 7));
                break;
            case 4:
                Spr_SetImage(spr, SubImg(P(0x435aec + iVar1 * 4), 11));
            }
            iVar2 = iVar2 + 1;
        } while (iVar2 < 6);
        iVar3 = iVar3 + 1;
    } while (iVar3 < 6);
    Spr_Update(1);
}

// 0x410e99: one growth step; returns 1 if any plant is alive.
int FUN_00410e99() {
    int iVar1;
    u32 uVar2;
    int iVar3;
    int local_20;
    int local_18;
    int local_14[4]; // [0..1] neighbour counts, [2] row, [3] col (as in the original frame)

    local_20 = 0;
    local_14[3] = 0;
    do {
        local_14[2] = 0;
        do {
            iVar1 = D32(0x43a8d0 + local_14[2] * 0x30 + local_14[3] * 8);
            local_18 = D32(0x43a8d4 + local_14[2] * 0x30 + local_14[3] * 8);
            FUN_00411001(local_14[3], local_14[2], local_14);
            iVar3 = D32(0x43a714 + iVar1 * 0x40 + D32(0x43a7a4) * 4) +
                    D32(0x43a720 + D32(0x43a7a8) * 4 + iVar1 * 0x40);
            for (uVar2 = 0; uVar2 < (u32)D32(0x43a710); uVar2 = uVar2 + 1) {
                iVar3 = iVar3 + D32(0x43a72c + local_14[uVar2] * 4 + uVar2 * 0x14 + iVar1 * 0x40);
            }
            if (iVar3 < 0) {
                local_18 = 0;
            } else if (0x18 < iVar3) {
                if (iVar3 < 0x32) {
                    if (((D32(0x43a7a0) == 1) && (local_18 != 0)) && (local_18 < 4)) {
                        local_18 = local_18 + 1;
                    }
                } else if (iVar3 < 100) {
                    if ((((D32(0x43a7a0) == 1) || (D32(0x43a7a0) == 3)) && (local_18 != 0)) && (local_18 < 4)) {
                        local_18 = local_18 + 1;
                    }
                } else if ((local_18 != 0) && (local_18 < 4)) {
                    local_18 = local_18 + 1;
                }
            }
            if (local_18 != 0) {
                local_20 = 1;
            }
            D32(0x43a9f0 + local_14[3] * 8 + local_14[2] * 0x30) = iVar1;
            D32(0x43a9f4 + local_14[2] * 0x30 + local_14[3] * 8) = local_18;
            local_14[2] = local_14[2] + 1;
        } while (local_14[2] < 6);
        local_14[3] = local_14[3] + 1;
    } while (local_14[3] < 6);
    FUN_004110da();
    memmove(DPTR(0x43a8d0), DPTR(0x43a9f0), 0x120);
    return local_20;
}

// 0x411001: count the living neighbours of cell (param_1 col, param_2 row)
// per plant kind into param_3[].
void FUN_00411001(int param_1, int param_2, int* param_3) {
    int* piVar1;
    u32 uVar2;

    for (uVar2 = 0; uVar2 < (u32)D32(0x43a710); uVar2 = uVar2 + 1) {
        param_3[uVar2] = 0;
    }
    if (0 < param_1) {
        if (D32(0x43a8cc + param_2 * 0x30 + param_1 * 8) != 0) {
            piVar1 = param_3 + D32(0x43a8c8 + param_1 * 8 + param_2 * 0x30);
            *piVar1 = *piVar1 + 1;
        }
    }
    if (param_1 < 5) {
        if (D32(0x43a8dc + param_2 * 0x30 + param_1 * 8) != 0) {
            piVar1 = param_3 + D32(0x43a8d8 + param_2 * 0x30 + param_1 * 8);
            *piVar1 = *piVar1 + 1;
        }
    }
    if (0 < param_2) {
        if (D32(0x43a8a4 + param_2 * 0x30 + param_1 * 8) != 0) {
            piVar1 = param_3 + D32(0x43a8a0 + param_1 * 8 + param_2 * 0x30);
            *piVar1 = *piVar1 + 1;
        }
    }
    if (param_2 < 5) {
        if (D32(0x43a904 + param_2 * 0x30 + param_1 * 8) != 0) {
            piVar1 = param_3 + D32(0x43a900 + param_2 * 0x30 + param_1 * 8);
            *piVar1 = *piVar1 + 1;
        }
    }
}

// 0x4110da: animate the transition from grid 0x43a8d0 to 0x43a9f0 (7 frames).
void FUN_004110da() {
    int iVar1;
    int iVar2;
    int local_e0[25];
    int local_78;
    int local_74[28];

    memcpy(local_74, DPTR(0x435cb4), sizeof(local_74));
    memcpy(local_e0, DPTR(0x435d24), sizeof(local_e0));
    local_78 = 0;
    do {
        iVar1 = 0;
        do {
            iVar2 = 0;
            do {
                int t = local_e0[D32(0x43a8d4 + iVar2 * 0x30 + iVar1 * 8) * 5 +
                                 D32(0x43a9f4 + iVar2 * 0x30 + iVar1 * 8)];
                if (-1 < t) {
                    int spr = D32(0x435b28 + (iVar2 * 6 + iVar1) * 4);
                    if (local_74[t * 7 + local_78] < 0) {
                        Spr_SetImage(spr, nullptr);
                    } else {
                        Spr_SetImage(spr, SubImg(P(0x435aec + D32(0x43a8d0 + iVar2 * 0x30 + iVar1 * 8) * 4),
                                                 local_74[t * 7 + local_78]));
                    }
                }
                iVar2 = iVar2 + 1;
            } while (iVar2 < 6);
            iVar1 = iVar1 + 1;
        } while (iVar1 < 6);
        FUN_00411262();
        Spr_Update(1);
        FUN_00410999(100);
        local_78 = local_78 + 1;
    } while (local_78 < 7);
}

// 0x4111ec: show the lever-2 effect sprites.
void FUN_004111ec() {
    int iVar1;

    D32(0x43ab18) = D32(0x43a7a8);
    for (iVar1 = 0; iVar1 < D32(0x43ab18); iVar1 = iVar1 + 1) {
        Spr_SetImage(D32(0x435b1c + iVar1 * 4), SubImg(P(0x435ae0 + iVar1 * 4), 0));
    }
    Timer_Set(2, 100);
}

// 0x411235
void FUN_00411235() {
    int iVar1;

    for (iVar1 = 0; iVar1 < D32(0x43ab18); iVar1 = iVar1 + 1) {
        Spr_SetImage(D32(0x435b1c + iVar1 * 4), nullptr);
    }
    Spr_Update(1);
}

// 0x411262: animate the lever-2 effect sprites.
void FUN_00411262() {
    bool bVar1;
    int iVar2;

    bVar1 = Timer_Expired(2);
    if (bVar1) {
        for (iVar2 = 0; iVar2 < D32(0x43ab18); iVar2 = iVar2 + 1) {
            Spr_SetImage(D32(0x435b1c + iVar2 * 4), SubImg(P(0x435ae0 + iVar2 * 4), D32(0x435d88)));
        }
        D32(0x435d88) = D32(0x435d88) + 1;
        if (3 < D32(0x435d88)) {
            D32(0x435d88) = 1;
        }
    }
}

// 0x4112c3: show the lever-1 effect sprite.
void FUN_004112c3() {
    if (0 < D32(0x43a7a4)) {
        Spr_SetImage(D32(0x435b24), SubImg(P(0x435ae8), D32(0x43a7a4) - 1));
    }
}

// 0x4112f5
void FUN_004112f5() { Spr_SetImage(D32(0x435b24), nullptr); }

// 0x41130a: play the pick animation of plant param_1.
void FUN_0041130a(int param_1) {
    bool bVar1;
    u32 uVar2;

    Cursor_Show(0);
    Spr_SetImage(D32(0x435b04), nullptr);
    Anim_StartWithSprite(D32(0x43ac50 + param_1 * 4), 0, 0, param_1 + 10, D32(0x435bbc + param_1 * 4));
    Timer_Set(2, 0);
    while (true) {
        bVar1 = Anim_IsPlaying(D32(0x43ac50 + param_1 * 4));
        if (!bVar1) break;
        uVar2 = (u32)Event_Poll();
        FUN_0040394c((u16)uVar2);
        bVar1 = Timer_Expired(2);
        if (bVar1) {
            Timer_Set(2, 0x32);
            Anim_UpdateAll();
            Spr_Update(1);
        }
    }
    Spr_SetImage(D32(0x435bbc + param_1 * 4), nullptr);
}

// 0x4113ab: compute the harvest per plant kind.
void FUN_004113ab() {
    u32 uVar1;
    int iVar2;
    int iVar3;
    int local_20[5];

    memcpy(local_20, DPTR(0x435d8c), sizeof(local_20));
    for (uVar1 = 0; uVar1 < (u32)D32(0x43a710); uVar1 = uVar1 + 1) {
        D32(0x43ab10 + uVar1 * 4) = 0;
    }
    iVar3 = 0;
    do {
        iVar2 = 0;
        do {
            int v = D32(0x43a8d0 + iVar2 * 0x30 + iVar3 * 8);
            D32(0x43ab10 + v * 4) = D32(0x43ab10 + v * 4) + local_20[D32(0x43a8d4 + iVar2 * 0x30 + iVar3 * 8)];
            iVar2 = iVar2 + 1;
        } while (iVar2 < 6);
        iVar3 = iVar3 + 1;
    } while (iVar3 < 6);
}

// 0x41141d: 1 if every harvest reached its target.
int FUN_0041141d() {
    u32 uVar1;

    uVar1 = 0;
    while (true) {
        if ((u32)D32(0x43a710) <= uVar1) {
            return 1;
        }
        if ((u32)D32(0x43ab10 + uVar1 * 4) < (u32)D32(0x43a794 + uVar1 * 4)) break;
        uVar1 = uVar1 + 1;
    }
    return 0;
}

// 0x411449: Ms. Grunkle's introduction (param_1: first visit).
int FUN_00411449(int param_1) {
    int iVar1;

    if (param_1 == 0) {
        iVar1 = FUN_00402ec2(D32(0x435da0) + 0x1598, 0, 1);
        D32(0x435da0) = D32(0x435da0) + 1;
        if (2 < D32(0x435da0)) {
            D32(0x435da0) = 0;
        }
    } else {
        iVar1 = FUN_00402ec2(0x16a8, 0, 1);
        if (iVar1 == 0) {
            iVar1 = FUN_00402ec2(0x16a9, 0, 1);
        }
    }
    return iVar1;
}
