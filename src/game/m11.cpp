// Module 11 of 4G.EXE (0x4147ec..0x41862c): the scrolling island-map
// mini-game (6x12 visible tile window over a 72x36 map, roaming creatures,
// collectibles, question/answer panels and the "boat" bonus sequence).
//
// Globals (data segment, see seg.h):
//   0x43b3f8 / 0x43b3fc   map scroll row / column (column wraps at 0x48)
//   0x43ae78 / 0x43ae7c   player column / row inside the visible window
//   0x43ae80              player facing (0 left, 1 right)
//   0x43b2d8              P: 6 x 12 tile images (row * 12 + col)
//   0x43adf8              P: map resource 0x2f04 (0x48 x 0x24 bytes)
//   0x43adfc              P: currently loaded text resource
//   0x43ad68              P: talk resource 0x15f0
//   0x43ad6c..0x43ada4    P: 15 'ANG' animations (0x313a..0x3148)
//   0x43adbc/c0/c4        P: animations 0x30db / 0x30d6 / 0x30da
//   0x43adc8..0x43ade0    P: 7 animations (0x30d7, 0x30d8, 0x30df..0x30e3)
//   0x43ade4..0x43adf4    P: [0] = null, [1..4] animations (0x30dc, 0x30de, 0x30d9 x2)
//   0x43b1f4..0x43b204    P: 5 off-screen 100x30 images for the counters
//   0x43b1e0 / 0x43b208   sprite ids for the panel icons / counter images
//   0x43ae8c, 0x43ae9c, 0x43aeac, 0x43aebc  creature sprite, column, row, direction bits
//   0x43aed0, 0x43af94, 0x43b058, 0x43b11c  item sprite, kind, column, row (count 0x43aecc)
//   0x43b410              state machine of the main event handler (0x417c4c)
#include "game/common.h"

// Tile image slot of the 6x12 visible map window.
static inline void*& TILE(int row, int col) { return P(0x43b2d8 + (u32)((row * 0xc + col) * 4)); }

// 0x4147ec: normalises the scroll column into [0, 0x48) and returns the
// wrapped map column of window column param_1.
int FUN_004147ec(int param_1) {
    while (0x47 < D32(0x43b3fc)) D32(0x43b3fc) = D32(0x43b3fc) - 0x48;
    while (D32(0x43b3fc) < 0) D32(0x43b3fc) = D32(0x43b3fc) + 0x48;
    int iVar1 = param_1 + D32(0x43b3fc);
    while (iVar1 < 0) iVar1 = iVar1 + 0x48;
    while (0x47 < iVar1) iVar1 = iVar1 + -0x48;
    return iVar1;
}

// 0x414827: map index (column * 0x24 + row) of window cell (param_1, param_2).
int FUN_00414827(int param_1, int param_2) {
    int iVar1 = FUN_004147ec(param_1);
    return iVar1 * 0x24 + param_2 + D32(0x43b3f8);
}

// 0x414846: copies text line param_2 of the current text resource's question
// block into param_1 (param_2 == 0 also counts the lines into 0x43b224).
void FUN_00414846(u8* param_1, int param_2) {
    char* text = (char*)P(0x43adfc);
    *param_1 = 0;
    char* pcVar1 = strstr(text, DSTR(0x436494)); // "Difficult"
    if (pcVar1 == nullptr && (pcVar1 = strstr(text, DSTR(0x43649e)), pcVar1 == nullptr)) {
        pcVar1 = text;
    }
    u8* pbVar4;
    do {
        pcVar1 = strchr(pcVar1 + 1, 0xd);
        char* pcVar2;
        if (pcVar1 != nullptr &&
            ((pcVar2 = strchr(pcVar1 + 1, 0xd), pcVar2 == pcVar1 + 1) || (pcVar2 == pcVar1 + 2))) {
            pbVar4 = (u8*)(pcVar2 + 1);
            break;
        }
        pbVar4 = nullptr;
    } while (pcVar1 != nullptr);
    if (pbVar4 != nullptr) {
        for (; (*pbVar4 < 0x21 && (*pbVar4 != 0)); pbVar4 = pbVar4 + 1) {
        }
        if (param_2 == 0) {
            D32(0x43b22c) = 0;
            D32(0x43b224) = 0;
            u8* pbVar3 = pbVar4;
            do {
                D32(0x43b224) = D32(0x43b224) + 1;
                pbVar3 = (u8*)strchr((char*)(pbVar3 + 1), 0xd);
                if (pbVar3 == nullptr) break;
                for (; (*pbVar3 < 0x21 && (*pbVar3 != 0)); pbVar3 = pbVar3 + 1) {
                }
            } while (0x21 < *pbVar3);
        }
        if (param_2 <= D32(0x43b224)) {
            int iVar5 = 0;
            if (0 < param_2) {
                do {
                    pbVar4 = (u8*)strchr((char*)(pbVar4 + 1), 0xd);
                    if (pbVar4 == nullptr) {
                        return;
                    }
                    for (; (*pbVar4 < 0x21 && (*pbVar4 != 0)); pbVar4 = pbVar4 + 1) {
                    }
                    iVar5 = iVar5 + 1;
                } while (iVar5 < param_2);
            }
            for (; 0x1f < *pbVar4; pbVar4 = pbVar4 + 1) {
                *param_1 = *pbVar4;
                param_1 = param_1 + 1;
            }
            *param_1 = 0;
        }
    }
}

// 0x414960: copies line param_2 (with the original's index remapping) after
// the marker string 0x4364a3 of the current text resource into param_1.
void FUN_00414960(u8* param_1, int param_2) {
    *param_1 = 0;
    u8* pbVar1 = (u8*)strstr((char*)P(0x43adfc), DSTR(0x4364a3));
    if (pbVar1 != nullptr) {
        int iVar3 = param_2;
        if ((1 < param_2) && (iVar3 = param_2 + 1, 4 < iVar3)) {
            iVar3 = param_2 + 2;
        }
        int iVar2 = 0;
        if (-1 < iVar3) {
            do {
                for (pbVar1 = (u8*)strchr((char*)(pbVar1 + 1), 0xd); *pbVar1 < 0x1f; pbVar1 = pbVar1 + 1) {
                }
                iVar2 = iVar2 + 1;
            } while (iVar2 <= iVar3);
        }
        for (; 0x1f < *pbVar1; pbVar1 = pbVar1 + 1) {
            *param_1 = *pbVar1;
            param_1 = param_1 + 1;
        }
        *param_1 = 0;
    }
}

// 0x4149c8: map byte under the player.
u8 FUN_004149c8() {
    int iVar1 = FUN_004147ec(D32(0x43ae78));
    return *((u8*)P(0x43adf8) + D32(0x43ae7c) + D32(0x43b3f8) + iVar1 * 0x24);
}

// 0x4149f6: redraws counter image idx with the number value.
void FUN_004149f6(int idx, int value) {
    char local_5c[80];
    int local_c = D32(0x436416);
    int local_8 = D32(0x43641a);
    sprintf(local_5c, DSTR(0x4364a8), value);
    void* img = P(0x43b1f4 + idx * 4);
    Rect r;
    Img_FullRect(img, &r);
    Gfx_SetTargetImg(img);
    u8 bVar1 = Img_Key(P(0x43b1f4 + idx * 4));
    Gfx_FillRect(r.x, r.y, r.w, r.h, bVar1);
    Text_DrawScreenFont(local_c, local_8, local_5c, P(0x4318a8), 0x33);
    Gfx_ResetTarget();
    Spr_Invalidate(D32(0x43b208 + idx * 4));
}

// 0x414a9a: draws a text panel (0 = clue for the current target, 1/2 = info
// text of the tile under the player, page 0x43b228).
void FUN_00414a9a(s16 param_1) {
    u8 local_234[512];
    int local_24[6];
    memcpy(local_24, DPTR(0x43641e), sizeof(local_24));
    int local_34 = local_24[param_1 * 2];
    int local_30 = local_24[param_1 * 2 + 1];
    int local_2c = 200;
    int local_28 = 0xf5;
    if (param_1 == 0) {
        int target = D32(0x43b280 + D32(0x43b2a4) * 4);
        D32(0x43b2a0) = D16(0x4363e2 + target * 2) * 6 + D32(0x43b290 + D32(0x43b2a4) * 4) + 0x17d4;
        P(0x43adfc) = Res_Load((u32)(int)D16(0x43639a + D32(0x43b280 + D32(0x43b2a4) * 4) * 2));
        int iVar2 = Res_LastSize();
        *((u8*)P(0x43adfc) + iVar2 + -1) = 0;
        FUN_00414960(local_234, D32(0x43b290 + D32(0x43b2a4) * 4));
        if ((D32(0x43b280 + D32(0x43b2a4) * 4) == 0x18) && (D32(0x43b290 + D32(0x43b2a4) * 4) == 5)) {
            local_30 = local_30 + -0x1e;
        }
    } else {
        u8 uVar1 = FUN_004149c8();
        P(0x43adfc) = Res_Load((u32)(int)D16(0x43639a + uVar1 * 2));
        int iVar2 = Res_LastSize();
        *((u8*)P(0x43adfc) + iVar2 + -1) = 0;
        FUN_00414846(local_234, D32(0x43b228));
    }
    g_textColumnGap = 0x32;
    Text_DrawFont((char*)local_234, local_34, local_30, local_2c, local_28, P(0x4318b0), 0x24, 0);
    if ((g_textFirstLine != 0) && (0 < g_textOverflow)) {
        local_2c = local_2c + g_textColumnGap + 200;
    }
    g_textOverflow = 0;
    if ((param_1 != 0) && (D32(0x43b228) + 1 < D32(0x43b224))) {
        int saved = g_textFirstLine;
        int local_c = 0x1ea;
        int local_8 = 0x164;
        Text_DrawScreenFont(0x1ea, 0x164, DSTR(0x4364ab), P(0x4318a8), 0x24); // "(more)"
        g_textFirstLine = saved;
        Screen_Flush(local_c, local_8, 100, 0x18);
    }
    Screen_Flush(local_34, local_30, local_2c, local_28);
    Res_Free(P(0x43adfc));
    P(0x43adfc) = nullptr;
}

// 0x414cdf: frees the tile images.
void FUN_00414cdf() {
    int iVar2 = 0;
    do {
        int iVar1 = 0;
        do {
            Res_Free(TILE(iVar2, iVar1));
            TILE(iVar2, iVar1) = nullptr;
            iVar1 = iVar1 + 1;
        } while (iVar1 < 0xc);
        iVar2 = iVar2 + 1;
    } while (iVar2 < 6);
}

// 0x414d21: loads the tile images of the visible window.
void FUN_00414d21() {
    int iVar4 = 0;
    do {
        int iVar3 = 0;
        do {
            int iVar1 = FUN_00414827(iVar3, iVar4);
            TILE(iVar4, iVar3) = Res_Load(iVar1 + 0x1b59);
            iVar3 = iVar3 + 1;
        } while (iVar3 < 0xc);
        iVar4 = iVar4 + 1;
    } while (iVar4 < 6);
}

// 0x414d66: scrolls the map window by one row (param_1) and/or one column
// (param_2), shifting the back buffer and the tile slots and loading the new
// edge tiles.
void FUN_00414d66(int param_1, int param_2) {
    int local_c;
    int local_8;
    Gfx_RestoreBg(D32(0x43ae78) * 0x32 + 0x14, D32(0x43ae7c) * 0x32 + 0x5a, 0x32, 0x32);
    if (param_1 != 0) {
        if (param_1 < 0) {
            Gfx_CopyRect(0x14, D32(0x431850) + 0x5a, 600, 0xfa, 0x14, 0x8c);
            Gfx_SaveBg(0x14, 0x8c, 600, 0xfa);
            local_8 = 0x5a;
            local_c = 0x14;
            int iVar4 = 5;
            do {
                int iVar3 = 0;
                do {
                    if (iVar4 == 5) {
                        Res_Free(TILE(5, iVar3));
                        TILE(5, iVar3) = nullptr;
                    }
                    if (iVar4 == 0) {
                        int iVar1 = FUN_00414827(iVar3, 0);
                        TILE(0, iVar3) = Res_Load(iVar1 + 0x1b59);
                        Img_Draw(local_c, 0x5a, TILE(0, iVar3), 0);
                        local_c = local_c + 0x32;
                    } else {
                        TILE(iVar4, iVar3) = TILE(iVar4 - 1, iVar3);
                    }
                    iVar3 = iVar3 + 1;
                } while (iVar3 < 0xc);
                iVar4 = iVar4 + -1;
            } while (-1 < iVar4);
        } else {
            Gfx_CopyRect(0x14, D32(0x431850) + 0x8c, 600, 0xfa, 0x14, 0x5a);
            Gfx_SaveBg(0x14, 0x5a, 600, 0xfa);
            local_8 = 0x154;
            local_c = 0x14;
            int iVar4 = 0;
            do {
                int iVar3 = 0;
                do {
                    if (iVar4 == 0) {
                        Res_Free(TILE(0, iVar3));
                        TILE(0, iVar3) = nullptr;
                    }
                    if (iVar4 < 5) {
                        TILE(iVar4, iVar3) = TILE(iVar4 + 1, iVar3);
                    } else {
                        int iVar1 = FUN_00414827(iVar3, iVar4);
                        TILE(iVar4, iVar3) = Res_Load(iVar1 + 0x1b59);
                        Img_Draw(local_c, 0x154, TILE(iVar4, iVar3), 0);
                        local_c = local_c + 0x32;
                    }
                    iVar3 = iVar3 + 1;
                } while (iVar3 < 0xc);
                iVar4 = iVar4 + 1;
            } while (iVar4 < 6);
        }
        Gfx_SaveBg(0x14, local_8, 600, 0x32);
    }
    if (param_2 != 0) {
        if (param_2 < 0) {
            local_8 = 0x5a;
            Gfx_CopyRect(0x14, D32(0x431850) + 0x5a, 0x226, 300, 0x46, 0x5a);
            Gfx_SaveBg(0x46, 0x5a, 0x226, 300);
            local_c = 0x14;
            int iVar4 = 0;
            do {
                int iVar3 = 0xb;
                do {
                    if (iVar3 == 0xb) {
                        Res_Free(TILE(iVar4, 0xb));
                        TILE(iVar4, 0xb) = nullptr;
                    }
                    if (iVar3 == 0) {
                        int iVar1 = FUN_00414827(0, iVar4);
                        TILE(iVar4, 0) = Res_Load(iVar1 + 0x1b59);
                        Img_Draw(0x14, local_8, TILE(iVar4, 0), 0);
                        local_8 = local_8 + 0x32;
                    } else {
                        TILE(iVar4, iVar3) = TILE(iVar4, iVar3 - 1);
                    }
                    iVar3 = iVar3 + -1;
                } while (-1 < iVar3);
                iVar4 = iVar4 + 1;
            } while (iVar4 < 6);
        } else {
            local_8 = 0x5a;
            Gfx_CopyRect(0x46, D32(0x431850) + 0x5a, 0x226, 300, 0x14, 0x5a);
            Gfx_SaveBg(0x14, 0x5a, 0x226, 300);
            local_c = 0x23a;
            int iVar4 = 0;
            do {
                int iVar3 = 0;
                do {
                    if (iVar3 == 0) {
                        Res_Free(TILE(iVar4, 0));
                        TILE(iVar4, 0) = nullptr;
                    }
                    if (iVar3 < 0xb) {
                        TILE(iVar4, iVar3) = TILE(iVar4, iVar3 + 1);
                    } else {
                        int iVar1 = FUN_00414827(iVar3, iVar4);
                        TILE(iVar4, iVar3) = Res_Load(iVar1 + 0x1b59);
                        Img_Draw(0x23a, local_8, TILE(iVar4, iVar3), 0);
                        local_8 = local_8 + 0x32;
                    }
                    iVar3 = iVar3 + 1;
                } while (iVar3 < 0xc);
                iVar4 = iVar4 + 1;
            } while (iVar4 < 6);
        }
        Gfx_SaveBg(local_c, 0x5a, 0x32, 300);
    }
}

// 0x4152e3: draws the whole tile window and saves it as background.
void FUN_004152e3() {
    D32(0x43b400) = 0x14;
    D32(0x43b404) = 0x5a;
    D32(0x43b408) = 600;
    D32(0x43b40c) = 300;
    int local_8 = 0x5a;
    int iVar2 = 0;
    do {
        int local_c = 0x14;
        int iVar1 = 0;
        do {
            Img_Draw(local_c, local_8, TILE(iVar2, iVar1), 0);
            local_c = local_c + 0x32;
            iVar1 = iVar1 + 1;
        } while (iVar1 < 0xc);
        local_8 = local_8 + 0x32;
        iVar2 = iVar2 + 1;
    } while (iVar2 < 6);
    Gfx_SaveBg(D32(0x43b400), D32(0x43b404), D32(0x43b408), D32(0x43b40c));
}

// 0x415382: picks up the item under the player; returns its kind or -1.
int FUN_00415382() {
    int iVar2 = FUN_004147ec(D32(0x43ae78));
    int iVar3 = 0;
    while (true) {
        if (D32(0x43aecc) <= iVar3) {
            return -1;
        }
        if ((iVar2 == D32(0x43b058 + iVar3 * 4)) && (D32(0x43ae7c) + D32(0x43b3f8) == D32(0x43b11c + iVar3 * 4)))
            break;
        iVar3 = iVar3 + 1;
    }
    int uVar1 = D32(0x43af94 + iVar3 * 4);
    Spr_Delete(D32(0x43aed0 + iVar3 * 4), 1);
    D32(0x43aecc) = D32(0x43aecc) + -1;
    for (; iVar3 < D32(0x43aecc); iVar3 = iVar3 + 1) {
        D32(0x43af94 + iVar3 * 4) = D32(0x43af98 + iVar3 * 4);
        D32(0x43b058 + iVar3 * 4) = D32(0x43b05c + iVar3 * 4);
        D32(0x43b11c + iVar3 * 4) = D32(0x43b120 + iVar3 * 4);
        D32(0x43aed0 + iVar3 * 4) = D32(0x43aed4 + iVar3 * 4);
    }
    D32(0x43aed0 + iVar3 * 4) = 0;
    return uVar1;
}

// 0x41542d: places an item of kind param_1 on a random free map cell away
// from the player.
void FUN_0041542d(int param_1) {
    while (true) {
        int iVar1 = Rand();
        iVar1 = iVar1 % 0x48;
        int iVar2 = Rand();
        iVar2 = iVar2 % 0x24;
        if (((D32(0x43ae78) + D32(0x43b3f8) + -8 <= iVar1) && (iVar1 <= D32(0x43ae78) + D32(0x43b3f8) + 8)) &&
            (D32(0x43ae7c) + D32(0x43b3fc) + -8 <= iVar2)) {
            if (iVar2 <= D32(0x43ae7c) + D32(0x43b3fc) + 8) continue;
        }
        if (*((u8*)P(0x43adf8) + iVar1 * 0x24 + iVar2) < 0x20) {
            int iVar3;
            for (iVar3 = 0; (iVar3 < D32(0x43aecc) &&
                             ((iVar1 != D32(0x43b058 + iVar3 * 4)) || (iVar2 != D32(0x43b11c + iVar3 * 4))));
                 iVar3 = iVar3 + 1) {
            }
            if (D32(0x43aecc) <= iVar3) {
                D32(0x43af94 + D32(0x43aecc) * 4) = param_1;
                D32(0x43b058 + D32(0x43aecc) * 4) = iVar1;
                D32(0x43b11c + D32(0x43aecc) * 4) = iVar2;
                D32(0x43aecc) = D32(0x43aecc) + 1;
                return;
            }
        }
    }
}

// 0x4154f4: resets the game state, picks the targets, creatures and items.
void FUN_004154f4() {
    int local_68[25];
    Spr_DeleteAll(0, 0);
    D32(0x436370) = 0;
    D32(0x43b2b4) = 0;
    D32(0x43b2b0) = 0;
    D32(0x43b2c0) = 3;
    D32(0x43b2a4) = 0;
    D32(0x43b2a8) = 0x7d;
    D32(0x43b2ac) = 10;
    D32(0x43ae74) = 0;
    D32(0x43ae68) = 0;
    D32(0x43ae40) = 0;
    D32(0x43ae30) = 0;
    D32(0x43ae00) = 0;
    D32(0x43ae04) = 0;
    D32(0x43ae08) = 0;
    D32(0x43ae54) = 0;
    D32(0x43ae3c) = 0;
    D32(0x43b3f8) = 10;
    D32(0x43b3fc) = 0xb;
    if (D32(0x43b3f8) < 0) D32(0x43b3f8) = 0;
    if (D32(0x43b3fc) < 0) D32(0x43b3fc) = 0;
    D32(0x43ae78) = 0x10 - D32(0x43b3f8);
    D32(0x43ae7c) = 0xe - D32(0x43b3fc);
    D32(0x43ae80) = 0;
    int iVar6 = 0;
    do {
        local_68[iVar6] = iVar6 + 1;
        iVar6 = iVar6 + 1;
    } while (iVar6 < 0x19);
    iVar6 = 0;
    do {
        int iVar2 = Rand();
        int iVar4 = local_68[iVar6];
        local_68[iVar6] = local_68[iVar2 % 0x19];
        local_68[iVar2 % 0x19] = iVar4;
        iVar6 = iVar6 + 1;
    } while (iVar6 < 5);
    iVar6 = 0;
    do {
        D32(0x43b280 + iVar6 * 4) = local_68[iVar6];
        int uVar3 = Rand() % 2;
        D32(0x43b290 + iVar6 * 4) = uVar3 + D32(0x43b2bc) * 2;
        if ((D16(0x43639c + D32(0x43b280 + iVar6 * 4) * 2) == 0x2ef5) && (D32(0x43b290 + iVar6 * 4) == 5)) {
            D32(0x43b290 + iVar6 * 4) = 4;
        }
        iVar6 = iVar6 + 1;
    } while (iVar6 < 4);
    u8 uVar1 = FUN_004149c8();
    if ((int)uVar1 == D32(0x43b280)) {
        D32(0x43b280) = local_68[4];
    }
    D32(0x43ae88) = 4;
    if (D32(0x43b2b8) < 2) {
        D32(0x43ae88) = 2;
    }
    for (iVar6 = 0; iVar6 < D32(0x43ae88); iVar6 = iVar6 + 1) {
        do {
            int iVar4 = Rand();
            D32(0x43ae9c + iVar6 * 4) = iVar4 % 0x48;
            if (D32(0x43ae9c + iVar6 * 4) < D32(0x43b3fc)) break;
        } while (D32(0x43ae9c + iVar6 * 4) < D32(0x43b3fc) + 0xc);
        int iVar4 = Rand();
        D32(0x43aeac + iVar6 * 4) = iVar4 % 6 + D32(0x43b3f8);
        int uVar3 = Rand() % 2;
        int uVar5 = Rand() % 2;
        D32(0x43aebc + iVar6 * 4) = uVar3 | uVar5 * 2;
        FUN_00416593(iVar6);
    }
    D32(0x43aecc) = 0;
    iVar6 = 0;
    do {
        FUN_0041542d(1);
        iVar6 = iVar6 + 1;
    } while (iVar6 < 0x24);
    iVar6 = 0;
    do {
        FUN_0041542d(0);
        iVar6 = iVar6 + 1;
    } while (iVar6 < 10);
    FUN_00414d21();
}

// 0x415772: builds the map screen and fades it in.
void FUN_00415772() {
    Gfx_FillRect(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h, 0x24);
    Gfx_SaveBg(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    FUN_004032e3();
    FUN_004152e3();
    FUN_00416c27();
    Spr_Update(0);
    Screen_FlushAll();
    void* rgb = Res_Load(0x30d5);
    Pal_FadeTo((const u8*)rgb, 1);
    Res_Free(rgb);
    Timer_Set(0, 0x32);
    Timer_Set(1, 200);
    D32(0x43ae54) = 0;
}

// 0x415818: switches to the creature encounter screen.
void FUN_00415818() {
    FUN_0041673b();
    for (int iVar4 = 0; iVar4 < D32(0x43ae88); iVar4 = iVar4 + 1) {
        Spr_Delete(D32(0x43ae8c + iVar4 * 4), 1);
        D32(0x43ae8c + iVar4 * 4) = 0;
    }
    for (int iVar4 = 0; iVar4 < D32(0x43aecc); iVar4 = iVar4 + 1) {
        Spr_Delete(D32(0x43aed0 + iVar4 * 4), 1);
        D32(0x43aed0 + iVar4 * 4) = 0;
    }
    if (D32(0x43ae74) != 0) {
        Spr_Delete(D32(0x43ae74), 1);
        D32(0x43ae74) = 0;
    }
    if (D32(0x43ae68) != 0) {
        Spr_Delete(D32(0x43ae68), 1);
        D32(0x43ae68) = 0;
    }
    if (D32(0x43ae5c) != 0) {
        Spr_Delete(D32(0x43ae5c), 1);
        D32(0x43ae5c) = 0;
    }
    void* pvVar1 = Res_Load(0x3138);
    Img_Draw(D32(0x436ee4), D32(0x436ee8), pvVar1, 0);
    Res_Free(pvVar1);
    Gfx_SaveBg(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    FUN_004032e3();
    if (D32(0x43b2c0) < 1) {
        D32(0x43b2c0) = 3;
    }
    D32(0x43b2c4) = 0;
    D32(0x43b2c8) = 0;
    Point pt;
    Pt_Set(&pt, 0, 0);
    int iVar4 = 0;
    do {
        const void* img;
        if (iVar4 < D32(0x43b2c0)) {
            img = Ang_FrameImage(P(0x43adbc), 1);
        } else {
            img = Ang_FrameImage(P(0x43add8), 0);
        }
        int iVar2 = Img_W(img);
        int iVar3 = Spr_Create(img, pt.x, pt.y, 0xc);
        D32(0x43aed0 + iVar4 * 4) = iVar3;
        pt.x = pt.x + iVar2;
        iVar4 = iVar4 + 1;
    } while (iVar4 < 3);
    const void* img = Ang_FrameImage(P(0x43ada4), 0);
    iVar4 = Img_W(img);
    Pt_Set(&pt, 0xa0, 0);
    int iVar2 = 0;
    do {
        int iVar3 = Spr_Create(img, pt.x, pt.y, 0xc);
        D32(0x43aedc + iVar2 * 4) = iVar3;
        pt.x = pt.x + iVar4;
        iVar2 = iVar2 + 1;
    } while (iVar2 < 3);
    FUN_004167a7(2);
    FUN_004149f6(2, D32(0x43b2a4));
    FUN_004149f6(3, D32(0x43b2a8));
    FUN_004149f6(4, D32(0x43b2ac));
    Spr_Update(0);
    pvVar1 = Res_Load(0x3139);
    Pal_Set((const u8*)pvVar1, 1);
    Res_Free(pvVar1);
    Screen_FlushAll();
    Timer_Set(0, 0x32);
    Timer_Set(1, 200);
}

// 0x415a86: frees the module's resources.
void FUN_00415a86() {
    Spr_DeleteAll(0, 0);
    Hotspot_Clear();
    FUN_00402d81();
    Res_Free(P(0x43ad68));
    P(0x43ad68) = nullptr;
    int iVar1 = 0;
    do {
        Mem_Free(P(0x43b1f4 + iVar1 * 4));
        P(0x43b1f4 + iVar1 * 4) = nullptr;
        iVar1 = iVar1 + 1;
    } while (iVar1 < 5);
    iVar1 = 0;
    do {
        Res_Free(P(0x43ad6c + iVar1 * 4));
        P(0x43ad6c + iVar1 * 4) = nullptr;
        iVar1 = iVar1 + 1;
    } while (iVar1 < 0xf);
    iVar1 = 0;
    do {
        Res_Free(P(0x43adc8 + iVar1 * 4));
        P(0x43adc8 + iVar1 * 4) = nullptr;
        iVar1 = iVar1 + 1;
    } while (iVar1 < 7);
    iVar1 = 1;
    do {
        Res_Free(P(0x43ade4 + iVar1 * 4));
        P(0x43ade4 + iVar1 * 4) = nullptr;
        iVar1 = iVar1 + 1;
    } while (iVar1 < 5);
    Res_Free(P(0x43adbc));
    P(0x43adbc) = nullptr;
    Res_Free(P(0x43adc4));
    P(0x43adc4) = nullptr;
    Res_Free(P(0x43adc0));
    P(0x43adc0) = nullptr;
    Res_Free(P(0x43adf8));
    P(0x43adf8) = nullptr;
    Cursor_Show(1);
    FUN_00405901();
}

// 0x415b8b: loads the module's resources.
void FUN_00415b8b() {
    FUN_00403e91();
    D32(0x43b2b8) = FUN_004040cb();
    if (2 < D32(0x43b2b8)) {
        D32(0x43b2b8) = 2;
    }
    D32(0x43b2bc) = FUN_004040a2();
    if (2 < D32(0x43b2bc)) {
        D32(0x43b2bc) = 2;
    }
    FUN_00402c78(0x7f1);
    P(0x43ad6c) = Res_Load(0x313b);
    P(0x43ad70) = Res_Load(0x313c);
    P(0x43ad74) = Res_Load(0x313d);
    P(0x43ad78) = Res_Load(0x313e);
    P(0x43ad7c) = Res_Load(0x313a);
    P(0x43ad80) = Res_Load(0x313f);
    P(0x43ad84) = Res_Load(0x3140);
    P(0x43ad88) = Res_Load(0x3141);
    P(0x43ad8c) = Res_Load(0x3143);
    P(0x43ad90) = Res_Load(0x3144);
    P(0x43ad94) = Res_Load(0x3145);
    P(0x43ad98) = Res_Load(0x3146);
    P(0x43ad9c) = Res_Load(0x3147);
    P(0x43ada0) = Res_Load(0x3142);
    P(0x43ada4) = Res_Load(0x3148);
    P(0x43ade4) = nullptr;
    P(0x43ade8) = Res_Load(0x30dc);
    P(0x43adec) = Res_Load(0x30de);
    P(0x43adf0) = Res_Load(0x30d9);
    P(0x43adf4) = Res_Load(0x30d9);
    P(0x43adc8) = Res_Load(0x30d7);
    P(0x43adcc) = Res_Load(0x30d8);
    P(0x43add0) = Res_Load(0x30df);
    P(0x43add4) = Res_Load(0x30e0);
    P(0x43add8) = Res_Load(0x30e1);
    P(0x43addc) = Res_Load(0x30e2);
    P(0x43ade0) = Res_Load(0x30e3);
    P(0x43adc0) = Res_Load(0x30d6);
    P(0x43adc4) = Res_Load(0x30da);
    P(0x43adbc) = Res_Load(0x30db);
    P(0x43adf8) = Res_Load(0x2f04);
    int iVar2 = 0;
    do {
        P(0x43b1f4 + iVar2 * 4) = Img_Create(100, 0x1e);
        iVar2 = iVar2 + 1;
    } while (iVar2 < 5);
    FUN_00402b7e(0x177a);
    P(0x43ad68) = Res_Load(0x15f0);
}

// 0x415dd7: sets frame param_1 of the creature sprite 0x43ae40.
void FUN_00415dd7(int param_1) {
    if (D32(0x43ae40) != 0) {
        int iVar1 = D32(0x43ae44);
        if (D32(0x43ae58) != 0) {
            iVar1 = D32(0x436384 + D32(0x43ae44) * 4);
        }
        D32(0x43ae48) = param_1;
        Point pt;
        Ang_FrameInfo(P(0x43ad6c + iVar1 * 4), param_1, &pt);
        pt.x = pt.x + D32(0x43ae4c);
        pt.y = pt.y + 100;
        const void* img = Ang_FrameImage(P(0x43ad6c + iVar1 * 4), param_1);
        Spr_SetImage(D32(0x43ae40), img);
        Spr_SetPos(D32(0x43ae40), pt.x, pt.y);
        iVar1 = Img_W(img);
        if (iVar1 + pt.x < D32(0x436374)) {
            D32(0x43ae4c) = D32(0x43184c);
        }
    }
}

// 0x415e88: sets frame param_1 of projectile sprite param_2 (0x43ae00[]).
void FUN_00415e88(int param_1, int param_2) {
    if (D32(0x43ae00 + param_2 * 4) != 0) {
        D32(0x43ae0c + param_2 * 4) = param_1;
        Point pt;
        Ang_FrameInfo(P(0x43ad6c + D32(0x436378 + param_2 * 4) * 4), param_1, &pt);
        if (param_2 < 2) {
            pt.x = pt.x + D32(0x43ae18 + param_2 * 4);
            pt.y = pt.y + D32(0x43ae24 + param_2 * 4);
        }
        const void* img = Ang_FrameImage(P(0x43ad6c + D32(0x436378 + param_2 * 4) * 4), param_1);
        Spr_SetImage(D32(0x43ae00 + param_2 * 4), img);
        Spr_SetPos(D32(0x43ae00 + param_2 * 4), pt.x, pt.y);
    }
}

// 0x415f1d: sets frame param_1 of the cannon sprite 0x43ae30.
void FUN_00415f1d(int param_1) {
    if (D32(0x43ae30) != 0) {
        D32(0x43ae34) = param_1;
        Point pt;
        Ang_FrameInfo(P(0x43ad78), param_1, &pt);
        const void* img = Ang_FrameImage(P(0x43ad78), param_1);
        Spr_SetImage(D32(0x43ae30), img);
        Spr_SetPos(D32(0x43ae30), pt.x, pt.y);
    }
}

// 0x415f88: sets frame param_2 of animation param_1 on the player sprite.
void FUN_00415f88(int param_1, int param_2) {
    if (D32(0x43ae74) != 0) {
        Point pt;
        Ang_FrameInfo(P(0x43adc8 + param_1 * 4), param_2 + 1, &pt);
        pt.x = pt.x + D32(0x43ae78) * 0x32 + 0x14;
        pt.y = pt.y + D32(0x43ae7c) * 0x32 + 0x34;
        const void* img = Ang_FrameImage(P(0x43adc8 + param_1 * 4), param_2);
        Spr_SetImage(D32(0x43ae74), img);
        Spr_SetPos(D32(0x43ae74), pt.x, pt.y);
        Spr_Invalidate(D32(0x43ae74));
    }
}

// 0x416015: sets frame param_1 of the info panel sprite 0x43ae5c.
void FUN_00416015(int param_1) {
    if (D32(0x43ae5c) != 0) {
        D32(0x43ae60) = param_1;
        Point pt;
        Ang_FrameInfo(P(0x43adc0), param_1, &pt);
        pt.y = pt.y + -10;
        const void* img = Ang_FrameImage(P(0x43adc0), param_1);
        Spr_SetImage(D32(0x43ae5c), img);
        Spr_SetPos(D32(0x43ae5c), pt.x, pt.y);
        Spr_Invalidate(D32(0x43ae5c));
        int iVar1 = Img_W(img);
        int h = Img_H(img);
        int y = pt.y;
        if (param_1 == 9) {
            Hotspot_Set(0x480, g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
        } else {
            Hotspot_Set(0x480, pt.x, pt.y, iVar1, h);
        }
        iVar1 = iVar1 / 2;
        Hotspot_Set(0x492, pt.x, y, iVar1, h);
        Hotspot_Set(0x494, pt.x + iVar1, y, iVar1, h);
    }
}

// 0x416143: sets frame param_1 of the clue panel sprite 0x43ae68.
void FUN_00416143(int param_1) {
    if (D32(0x43ae68) != 0) {
        D32(0x43ae6c) = param_1;
        Point pt;
        Ang_FrameInfo(P(0x43adc4), param_1, &pt);
        pt.x = pt.x + -0x28;
        pt.y = pt.y + -0x28;
        const void* img = Ang_FrameImage(P(0x43adc4), param_1);
        Spr_SetImage(D32(0x43ae68), img);
        Spr_SetPos(D32(0x43ae68), pt.x, pt.y);
        int w = Img_W(img);
        int h = Img_H(img);
        int y = pt.y;
        int x = pt.x;
        if (param_1 == 9) {
            Hotspot_Set(0x480, g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
        } else {
            Hotspot_Set(0x480, pt.x, pt.y, w, h);
        }
        Hotspot_Set(0x490, x, y, w, h);
    }
}

// 0x416238: repositions item sprite param_1 for the current scroll.
void FUN_00416238(int param_1) {
    if (D32(0x43aed0 + param_1 * 4) != 0) {
        int iVar3 = FUN_004147ec(0);
        int iVar4 = D32(0x43b058 + param_1 * 4);
        int iVar1 = D32(0x43b11c + param_1 * 4);
        if ((0x3c < iVar3) && (iVar4 < 0xc)) {
            iVar4 = iVar4 + 0x48;
        }
        if ((((iVar4 < iVar3) || (iVar1 < D32(0x43b3f8))) || (iVar3 + 0xc <= iVar4)) ||
            (D32(0x43b3f8) + 6 <= iVar1)) {
            Spr_SetImage(D32(0x43aed0 + param_1 * 4), nullptr);
        } else {
            int iVar2 = D32(0x43af94 + param_1 * 4);
            Point pt;
            Ang_FrameInfo(P(0x43adc8 + iVar2 * 4), 0, &pt);
            pt.x = pt.x + (iVar4 - iVar3) * 0x32 + 0x1e;
            pt.y = pt.y + (iVar1 - D32(0x43b3f8)) * 0x32 + 0x74 + (iVar2 == 2 ? -0x12 : 0);
            const void* img = Ang_FrameImage(P(0x43adc8 + iVar2 * 4), 0);
            Spr_SetImage(D32(0x43aed0 + param_1 * 4), img);
            Spr_SetPos(D32(0x43aed0 + param_1 * 4), pt.x, pt.y);
        }
    }
}

// 0x416345: repositions creature sprite param_1 for the current scroll.
void FUN_00416345(int param_1) {
    if (D32(0x43ae8c + param_1 * 4) != 0) {
        int iVar2 = FUN_004147ec(0);
        int iVar3 = D32(0x43ae9c + param_1 * 4);
        int iVar1 = D32(0x43aeac + param_1 * 4);
        if ((0x3c < iVar2) && (iVar3 < 0xc)) {
            iVar3 = iVar3 + 0x48;
        }
        if ((((iVar3 < iVar2) || (iVar1 < D32(0x43b3f8))) || (iVar2 + 0xc <= iVar3)) ||
            (D32(0x43b3f8) + 6 <= iVar1)) {
            Spr_SetImage(D32(0x43ae8c + param_1 * 4), nullptr);
        } else {
            Point pt;
            Ang_FrameInfo(P(0x43add0), 0, &pt);
            pt.x = pt.x + (iVar3 - iVar2) * 0x32 + 0x18;
            pt.y = pt.y + (iVar1 - D32(0x43b3f8)) * 0x32 + 0x5c;
            const void* img = Ang_FrameImage(P(0x43add0), (int)((D8(0x43aebc + param_1 * 4) & 1) == 0));
            Spr_SetImage(D32(0x43ae8c + param_1 * 4), img);
            Spr_SetPos(D32(0x43ae8c + param_1 * 4), pt.x, pt.y);
        }
    }
}

// 0x416437: sets frame param_1 (facing) of the player sprite.
void FUN_00416437(int param_1) {
    if (D32(0x43ae74) != 0) {
        Point pt;
        Ang_FrameInfo(P(0x43adbc), param_1, &pt);
        pt.x = pt.x + D32(0x43ae78) * 0x32 + 0x1c;
        pt.y = pt.y + D32(0x43ae7c) * 0x32 + 0x5c;
        const void* img = Ang_FrameImage(P(0x43adbc), param_1);
        Spr_SetImage(D32(0x43ae74), img);
        Spr_SetPos(D32(0x43ae74), pt.x, pt.y);
    }
}

// 0x4164ae: deletes the encounter sprites and creates the player sprite.
void FUN_004164ae(int param_1) {
    if (D32(0x43ae00) != 0) {
        Spr_Delete(D32(0x43ae00), 1);
        D32(0x43ae00) = 0;
    }
    if (D32(0x43ae04) != 0) {
        Spr_Delete(D32(0x43ae04), 1);
        D32(0x43ae04) = 0;
    }
    if (D32(0x43ae30) != 0) {
        Spr_Delete(D32(0x43ae30), 1);
        D32(0x43ae30) = 0;
    }
    if (D32(0x43ae40) != 0) {
        Spr_Delete(D32(0x43ae40), 1);
        D32(0x43ae40) = 0;
    }
    Point pt;
    Ang_FrameInfo(P(0x43adbc), param_1, &pt);
    pt.x = pt.x + D32(0x43ae78) * 0x32 + 0x1c;
    pt.y = pt.y + D32(0x43ae7c) * 0x32 + 0x5c;
    const void* img = Ang_FrameImage(P(0x43adbc), param_1);
    D32(0x43ae74) = Spr_Create(img, pt.x, pt.y, 2);
}

// 0x416593: creates creature sprite param_1.
void FUN_00416593(int param_1) {
    int iVar2 = FUN_004147ec(0);
    int iVar3 = D32(0x43ae9c + param_1 * 4);
    int iVar1 = D32(0x43aeac + param_1 * 4);
    if ((0x3c < iVar2) && (iVar3 < 0xc)) {
        iVar3 = iVar3 + 0x48;
    }
    Point pt;
    Ang_FrameInfo(P(0x43add0), 0, &pt);
    const void* local_10;
    if ((((iVar3 < iVar2) || (iVar1 < D32(0x43b3f8))) || (iVar2 + 0xc <= iVar3)) || (D32(0x43b3f8) + 6 <= iVar1)) {
        local_10 = nullptr;
    } else {
        local_10 = Ang_FrameImage(P(0x43add0), 0);
        pt.x = pt.x + (iVar3 - iVar2) * 0x32 + 0x18;
        pt.y = pt.y + (iVar1 - D32(0x43b3f8)) * 0x32 + 0x5c;
    }
    iVar3 = Spr_Create(local_10, pt.x, pt.y, 1);
    D32(0x43ae8c + param_1 * 4) = iVar3;
}

// 0x416653: creates item sprite param_1.
void FUN_00416653(int param_1) {
    int iVar3 = FUN_004147ec(0);
    int iVar4 = D32(0x43b058 + param_1 * 4);
    int iVar1 = D32(0x43b11c + param_1 * 4);
    if ((0x3c < iVar3) && (iVar4 < 0xc)) {
        iVar4 = iVar4 + 0x48;
    }
    int iVar2 = D32(0x43af94 + param_1 * 4);
    Point pt;
    Ang_FrameInfo(P(0x43adc8 + iVar2 * 4), 0, &pt);
    const void* local_10;
    if ((((iVar4 < iVar3) || (iVar1 < D32(0x43b3f8))) || (iVar3 + 0xc <= iVar4)) || (D32(0x43b3f8) + 6 <= iVar1)) {
        local_10 = nullptr;
    } else {
        local_10 = Ang_FrameImage(P(0x43adc8 + iVar2 * 4), 0);
        pt.x = pt.x + (iVar4 - iVar3) * 0x32 + 0x1e;
        pt.y = pt.y + (iVar1 - D32(0x43b3f8)) * 0x32 + 0x74 + (iVar2 == 2 ? -0x12 : 0);
    }
    iVar4 = Spr_Create(local_10, pt.x, pt.y, 1);
    D32(0x43aed0 + param_1 * 4) = iVar4;
}

// 0x41673b: deletes the projectile and panel sprites.
void FUN_0041673b() {
    int iVar1 = 0;
    do {
        if (D32(0x43ae00 + iVar1 * 4) != 0) {
            Spr_Delete(D32(0x43ae00 + iVar1 * 4), 1);
            D32(0x43ae00 + iVar1 * 4) = 0;
        }
        iVar1 = iVar1 + 1;
    } while (iVar1 < 3);
    iVar1 = 1;
    do {
        Spr_Delete(D32(0x43b1e0 + iVar1 * 4), 0);
        D32(0x43b1e0 + iVar1 * 4) = 0;
        Spr_Delete(D32(0x43b208 + iVar1 * 4), 0);
        D32(0x43b208 + iVar1 * 4) = 0;
        iVar1 = iVar1 + 1;
    } while (iVar1 < 5);
}

// 0x4167a7: creates the panel icons and counters from index param_1 on.
void FUN_004167a7(int param_1) {
    int local_5c[20]; // 10 icon positions, then 10 counter positions
    memcpy(local_5c, DPTR(0x436436), sizeof(local_5c));
    int* aiStack_34 = local_5c + 10;
    for (; param_1 < 5; param_1 = param_1 + 1) {
        int local_c = local_5c[param_1 * 2];
        int local_8 = local_5c[param_1 * 2 + 1];
        const void* img = Ang_FrameImage(P(0x43ade4 + param_1 * 4), (int)(param_1 == 4));
        int iVar1 = Spr_Create(img, local_c, local_8, 0xc);
        D32(0x43b1e0 + param_1 * 4) = iVar1;
        if (param_1 == 1) {
            iVar1 = Img_W(img);
            int h = Img_H(img);
            Hotspot_Set(0x493, local_c, local_8, iVar1, h);
        }
        local_c = aiStack_34[param_1 * 2];
        local_8 = aiStack_34[param_1 * 2 + 1];
        if (local_c != 0) {
            iVar1 = Spr_Create(P(0x43b1f4 + param_1 * 4), local_c, local_8, 0xc);
            D32(0x43b208 + param_1 * 4) = iVar1;
        }
    }
}

// 0x41688f: creates the info panel sprite and its hotspots.
void FUN_0041688f() {
    D32(0x43ae60) = 0;
    Point pt;
    Ang_FrameInfo(P(0x43adc0), 0, &pt);
    pt.y = pt.y + -10;
    const void* img = Ang_FrameImage(P(0x43adc0), D32(0x43ae60));
    D32(0x43ae5c) = Spr_Create(img, pt.x, pt.y, 0x14);
    D32(0x43ae64) = 0;
    int w = Img_W(img);
    int h = Img_H(img);
    int y = pt.y;
    Hotspot_Set(0x480, pt.x, pt.y, w, h);
    Hotspot_Set(0x494, pt.x, y, w, h);
    Hotspot_Set(0x492, pt.x, y, w, h);
}

// 0x416961: creates the clue panel sprite and its hotspots.
void FUN_00416961() {
    D32(0x43ae6c) = 0;
    Point pt;
    Ang_FrameInfo(P(0x43adc4), 0, &pt);
    pt.x = pt.x + -0x28;
    pt.y = pt.y + -0x28;
    const void* img = Ang_FrameImage(P(0x43adc4), D32(0x43ae6c));
    D32(0x43ae68) = Spr_Create(img, pt.x, pt.y, 0x14);
    D32(0x43ae70) = 0;
    int w = Img_W(img);
    int h = Img_H(img);
    int y = pt.y;
    int x = pt.x;
    Hotspot_Set(0x480, pt.x, pt.y, w, h);
    Hotspot_Set(0x490, x, y, w, h);
}

// 0x416a1a: (re)creates the creature sprite with animation param_1, frame param_2.
void FUN_00416a1a(s16 param_1, s16 param_2) {
    if (D32(0x43ae40) != 0) {
        Spr_Delete(D32(0x43ae40), 1);
        D32(0x43ae40) = 0;
        D32(0x43ae54) = 0;
    }
    D32(0x43ae44) = (int)param_1;
    if (D32(0x43ae58) != 0) {
        param_1 = D16(0x436384 + D32(0x43ae44) * 4);
    }
    D32(0x43ae48) = (int)param_2;
    Point pt;
    Ang_FrameInfo(P(0x43ad6c + param_1 * 4), D32(0x43ae48), &pt);
    pt.x = pt.x + D32(0x43ae4c);
    pt.y = pt.y + 100;
    const void* img = Ang_FrameImage(P(0x43ad6c + param_1 * 4), (int)param_2);
    D32(0x43ae40) = Spr_Create(img, pt.x, pt.y, 0xc);
}

// 0x416ad0: (re)creates projectile sprite param_1 at frame param_2.
void FUN_00416ad0(s16 param_1, s16 param_2) {
    if (D32(0x43ae00 + param_1 * 4) != 0) {
        Spr_Delete(D32(0x43ae00 + param_1 * 4), 1);
        D32(0x43ae00 + param_1 * 4) = 0;
    }
    D32(0x43ae0c + param_1 * 4) = (int)param_2;
    Point pt;
    Ang_FrameInfo(P(0x43ad6c + D32(0x436378 + param_1 * 4) * 4), (int)param_2, &pt);
    if (param_1 < 2) {
        pt.x = pt.x + D32(0x43ae18 + param_1 * 4);
        pt.y = pt.y + D32(0x43ae24 + param_1 * 4);
    }
    const void* img = Ang_FrameImage(P(0x43ad6c + D32(0x436378 + param_1 * 4) * 4), (int)param_2);
    int iVar1 = Spr_Create(img, pt.x, pt.y, 0xf);
    D32(0x43ae00 + param_1 * 4) = iVar1;
}

// 0x416b99: (re)creates the cannon sprite with animation param_1.
void FUN_00416b99(s16 param_1) {
    if (D32(0x43ae30) != 0) {
        Spr_Delete(D32(0x43ae30), 1);
        D32(0x43ae30) = 0;
    }
    D32(0x43ae34) = 0;
    Point pt;
    Ang_FrameInfo(P(0x43ad6c + param_1 * 4), 0, &pt);
    const void* img = Ang_FrameImage(P(0x43ad6c + param_1 * 4), D32(0x43ae34));
    D32(0x43ae30) = Spr_Create(img, pt.x, pt.y, 0xc);
    D32(0x43ae3c) = 0;
}

// 0x416c27: creates all map screen sprites.
void FUN_00416c27() {
    int iVar1 = 0;
    do {
        Spr_Delete(D32(0x43aed0 + iVar1 * 4), 1);
        D32(0x43aed0 + iVar1 * 4) = 0;
        iVar1 = iVar1 + 1;
    } while (iVar1 < 6);
    FUN_0041673b();
    FUN_004164ae(0);
    for (iVar1 = 0; iVar1 < D32(0x43ae88); iVar1 = iVar1 + 1) {
        FUN_00416593(iVar1);
    }
    for (iVar1 = 0; iVar1 < D32(0x43aecc); iVar1 = iVar1 + 1) {
        FUN_00416653(iVar1);
    }
    FUN_00416961();
    FUN_0041688f();
    FUN_004167a7(1);
    FUN_004149f6(2, D32(0x43b2a4));
    FUN_004149f6(3, D32(0x43b2a8));
    FUN_004149f6(4, D32(0x43b2ac));
}

// 0x416cc6: checks whether the player stands on the current target.
void FUN_00416cc6() {
    u8 uVar1 = FUN_004149c8();
    if ((int)uVar1 != D32(0x43b280 + D32(0x43b2a4) * 4)) {
        FUN_00404068(0);
        FUN_00402bad(5, 1);
        D32(0x43b2b0) = D32(0x43b2b0) + 1;
        if (D32(0x43b2b0) == 1) {
            FUN_00402d42(0x15dd, nullptr, 0);
            return;
        }
        FUN_00402d42(Rand() % 2 + 0x15de, nullptr, 0);
        return;
    }
    FUN_00404068(1);
    FUN_00402bad(4, 1);
    D32(0x43b2a4) = D32(0x43b2a4) + 1;
    FUN_004149f6(2, D32(0x43b2a4));
    FUN_00403e2c(0xfa);
    if (D32(0x43b2a4) < 4) {
        if (D32(0x43b2a4) == 1) {
            FUN_00402e72(0x15da, nullptr, 0, 1, 1);
        } else {
            FUN_00402e72(Rand() % 2 + 0x15db, nullptr, 0, 1, 1);
        }
        D32(0x43b410) = 3;
        return;
    }
    FUN_00402e72(Rand() % 2 + 0x15db, nullptr, 0, 1, 1);
}

// 0x416df6: advances projectile sprite param_1 (0/1 shots, 2 splash).
void FUN_00416df6(int param_1) {
    if (D32(0x43ae00 + param_1 * 4) != 0) {
        if (param_1 == 2) {
            int f = D32(0x43ae14);
            int iVar1 = f + 1;
            if (iVar1 < 0) {
                iVar1 = f + 4;
            }
            if ((iVar1 >> 2) * 4 + 2 == f + 1) {
                FUN_00402bad(9, 3);
            }
            f = D32(0x43ae14);
            iVar1 = f + 1;
            if (iVar1 < 0) {
                iVar1 = f + 4;
            }
            if ((iVar1 >> 2) * 4 == f + 1) {
                Spr_Delete(D32(0x43ae08), 1);
                D32(0x43ae08) = 0;
                return;
            }
        } else if (4 < D32(0x43ae0c + param_1 * 4)) {
            Spr_Delete(D32(0x43ae00 + param_1 * 4), 1);
            D32(0x43ae00 + param_1 * 4) = 0;
            if (param_1 != 1) {
                return;
            }
            int iVar1 = Rand();
            if ((3 < iVar1 % 10) && (0 < D32(0x43b2c0))) {
                FUN_00402bad(8, 3);
                D32(0x43b2c0) = D32(0x43b2c0) + -1;
                const void* img = Ang_FrameImage(P(0x43add8), 0);
                Spr_SetImage(D32(0x43aed0 + D32(0x43b2c0) * 4), img);
                if (D32(0x43b2c0) == 0) {
                    return;
                }
                FUN_00402d42(0x15e0, nullptr, 0);
                return;
            }
            FUN_00402bad(9, 3);
            return;
        }
        FUN_00415e88(D32(0x43ae0c + param_1 * 4) + 1, param_1);
        if ((param_1 == 0) && (D32(0x43ae0c) == 3)) {
            if (D32(0x43ae58) != 0) {
                if (((D32(0x43ae44) == 1) && (D32(0x43ae4c) < D32(0x43ae18))) &&
                    (D32(0x43ae18) < D32(0x43ae4c) + 0x46)) {
                    D32(0x43ae44) = 2;
                    FUN_00402bad(8, 3);
                    return;
                }
                FUN_00402bad(9, 3);
                return;
            }
            if (((D32(0x43ae44) == 1) && (D32(0x43ae4c) + 0x1e < D32(0x43ae18))) &&
                (D32(0x43ae18) < D32(0x43ae4c) + 100)) {
                D32(0x43ae44) = 2;
                FUN_00402bad(8, 3);
                return;
            }
            FUN_00402bad(9, 3);
        }
    }
}

// 0x41700c: advances the cannon animation and fires when it is aimed.
void FUN_0041700c() {
    if (D32(0x43ae30) != 0) {
        if (D32(0x43ae38) <= D32(0x43ae34)) {
            D32(0x43ae34) = D32(0x43ae34) + -6;
            D32(0x43ae3c) = 0;
        }
        FUN_00415f1d(D32(0x43ae34) + 1);
        if (D32(0x43ae38) + -2 == D32(0x43ae34)) {
            int q = D32(0x43184c) / 5;
            int iVar2 = (q * D32(0x43ae38)) / 6 - q / 2;
            D32(0x43ae18) = iVar2 + -0x14;
            if (D32(0x43ae38) / 6 == 0) {
                D32(0x43ae18) = D32(0x43ae18) + 0x3c;
            }
            if (D32(0x43ae38) / 6 == 4) {
                D32(0x43ae18) = D32(0x43ae18) + -0x32;
            }
            D32(0x43ae24) = 0xdc;
            if (D32(0x43ae58) == 0) {
                if (((D32(0x43ae44) == 1) && (D32(0x43ae4c) + 10 < D32(0x43ae18))) &&
                    (D32(0x43ae18) < D32(0x43ae4c) + 0x50)) {
                    FUN_00416ad0(0, 0);
                } else {
                    FUN_00416ad0(2, (s16)(D32(0x43ae38) / 6 << 2));
                }
            } else if (((D32(0x43ae44) == 1) && (D32(0x43ae4c) + 0x32 < D32(0x43ae18))) &&
                       (D32(0x43ae18) < D32(0x43ae4c) + 0x78)) {
                FUN_00416ad0(0, 0);
            } else {
                FUN_00416ad0(2, (s16)(D32(0x43ae38) / 6 << 2));
            }
            FUN_00402bad(Rand() % 2 + 6, 1);
        }
    }
}

// 0x417189: advances the creature animation of the encounter.
void FUN_00417189() {
    if (D32(0x43ae40) != 0) {
        if (D32(0x43ae44) == 5) {
            D32(0x43ae48) = D32(0x43ae48) + 1;
            if (8 < D32(0x43ae48)) {
                D32(0x43b410) = 9;
                Timer_Set(1, 500);
                D32(0x43ae50) = 3;
                return;
            }
            FUN_00415dd7(D32(0x43ae48));
            return;
        }
        if ((D32(0x43ae44) == 0) || (D32(0x43ae44) == 4)) {
            if (D32(0x43b410) == 10) {
                D32(0x43ae48) = D32(0x43ae48) + 1;
                if (5 < D32(0x43ae48)) {
                    D32(0x43ae1c) = 0;
                    D32(0x43b410) = 6;
                    return;
                }
            } else {
                D32(0x43ae48) = D32(0x43ae48) + -1;
                if (D32(0x43ae48) < 0) {
                    Spr_Delete(D32(0x43ae40), 1);
                    D32(0x43ae40) = 0;
                    D32(0x43b410) = 9;
                    Timer_Set(1, 500);
                    D32(0x43ae50) = 3;
                    return;
                }
            }
            FUN_00415dd7(D32(0x43ae48));
            return;
        }
        if (D32(0x43ae44) == -1) {
            const void* img = Ang_FrameImage(P(0x43add4), 0);
            Spr_SetImage(D32(0x43aed0 + (5 - D32(0x43b21c)) * 4), img);
            D32(0x43b21c) = D32(0x43b21c) + 1;
            if (D32(0x43b21c) < 3) {
                FUN_00402d42(Rand() % 4 + 0x15e2, nullptr, 0);
                FUN_00416a1a(4, 5);
            } else {
                FUN_00416a1a(5, 0);
            }
            D32(0x43b410) = 0xb;
            Timer_Set(1, 0x1f);
            return;
        }
        D32(0x43ae48) = D32(0x43ae48) + 1;
        if (5 < D32(0x43ae48)) {
            D32(0x43ae48) = 0;
        }
        FUN_00415dd7(D32(0x43ae48));
        if ((D32(0x43ae44) == 1) && (D32(0x43ae04) == 0)) {
            if (D32(0x43ae58) == 0) {
                if (((D32(0x43ae4c) < 100) && (D32(0x43ae1c) == 0)) && (D32(0x43b2c8) < 3)) {
                    D32(0x43ae1c) = D32(0x43ae4c) + 0x32;
                    D32(0x43ae28) = 0xd2;
                    FUN_00416ad0(1, 0);
                    FUN_00402bad(Rand() % 2 + 6, 1);
                    D32(0x43b2c8) = D32(0x43b2c8) + 1;
                }
            } else if (((0x172 < D32(0x43ae4c)) && (D32(0x43ae1c) == 0)) && (D32(0x43b2c8) < 3)) {
                D32(0x43ae1c) = D32(0x43ae4c) + 0x32;
                D32(0x43ae28) = 0xd2;
                FUN_00416ad0(1, 0);
                FUN_00402bad(Rand() % 2 + 6, 1);
                D32(0x43b2c8) = D32(0x43b2c8) + 1;
            }
        }
        if (D32(0x43ae44) == 2) {
            D32(0x43ae44) = -1;
        }
    }
}

// 0x417411: opens/closes the info panel one frame at a time.
void FUN_00417411() {
    if (D32(0x43ae64) == 0) {
        D32(0x43ae60) = D32(0x43ae60) + 1;
        if (D32(0x43ae60) < 10) {
            FUN_00416015(D32(0x43ae60));
            if (D32(0x43ae60) == 9) {
                D32(0x43ae64) = 1;
                D32(0x43b410) = 2;
                return;
            }
        }
    } else {
        D32(0x43ae60) = D32(0x43ae60) + -1;
        FUN_00416015(D32(0x43ae60));
        if (D32(0x43ae60) == 0) {
            D32(0x43ae64) = 0;
            D32(0x43b410) = 0;
        }
    }
}

// 0x417483: opens/closes the clue panel one frame at a time.
void FUN_00417483() {
    if (D32(0x43ae70) == 0) {
        D32(0x43ae6c) = D32(0x43ae6c) + 1;
        if (D32(0x43ae6c) < 10) {
            FUN_00416143(D32(0x43ae6c));
            if (D32(0x43ae6c) == 9) {
                D32(0x43ae70) = 1;
                D32(0x43b410) = 4;
                FUN_00402bad(0, 1);
                return;
            }
        }
    } else {
        D32(0x43ae6c) = D32(0x43ae6c) + -1;
        FUN_00416143(D32(0x43ae6c));
        if (D32(0x43ae6c) == 0) {
            D32(0x43ae70) = 0;
            D32(0x43b410) = 0;
        }
    }
}

// 0x417501: plays the digging animation, then checks the spot.
void FUN_00417501() {
    u8 uVar1 = FUN_004149c8();
    int iVar2;
    int iVar3;
    if ((int)uVar1 == D32(0x43b280 + D32(0x43b2a4) * 4)) {
        iVar2 = 0x1f;
        iVar3 = 6;
    } else {
        iVar2 = 0x18;
        iVar3 = 5;
    }
    if (D32(0x43ae84) < iVar2) {
        D32(0x43ae84) = D32(0x43ae84) + 1;
        FUN_00415f88(iVar3, D32(0x43ae84));
        return;
    }
    D32(0x43b410) = 0;
    FUN_00416cc6();
    FUN_00416437(D32(0x43ae80));
}

// 0x417567: moves the roaming creatures.
void FUN_00417567() {
    int iVar1 = FUN_004147ec(0);
    if (iVar1 < 0x25) {
        iVar1 = iVar1 + 0x24;
    } else {
        iVar1 = iVar1 + -0x24;
    }
    // [ebp-4] in the original: one slot for the whole loop, left unchanged by
    // Spr_GetImage when the sprite does not exist.
    const void* local_8 = nullptr;
    for (int iVar5 = 0; iVar5 < D32(0x43ae88); iVar5 = iVar5 + 1) {
        Spr_GetImage(D32(0x43ae8c + iVar5 * 4), &local_8);
        if (iVar1 == D32(0x43ae9c + iVar5 * 4)) {
            int uVar2 = Rand() % 2;
            int uVar3 = Rand() % 2;
            D32(0x43aebc + iVar5 * 4) = uVar2 | uVar3 * 2;
        }
        if ((D8(0x43aebc + iVar5 * 4) & 1) == 0) {
            D32(0x43ae9c + iVar5 * 4) = D32(0x43ae9c + iVar5 * 4) + 1;
            if (0x47 < D32(0x43ae9c + iVar5 * 4)) {
                D32(0x43ae9c + iVar5 * 4) = D32(0x43ae9c + iVar5 * 4) + -0x48;
            }
        } else {
            D32(0x43ae9c + iVar5 * 4) = D32(0x43ae9c + iVar5 * 4) + -1;
            if (D32(0x43ae9c + iVar5 * 4) < 0) {
                D32(0x43ae9c + iVar5 * 4) = D32(0x43ae9c + iVar5 * 4) + 0x48;
            }
        }
        if ((iVar5 < 2) || (D32(0x43b2b8) < 2)) {
            if (local_8 == nullptr) {
                int iVar4 = Rand();
                D32(0x43aeac + iVar5 * 4) = iVar4 % 6 + D32(0x43b3f8);
            }
        } else if ((D8(0x43aebc + iVar5 * 4) & 2) == 0) {
            D32(0x43aeac + iVar5 * 4) = D32(0x43aeac + iVar5 * 4) + 1;
            if (0x23 < D32(0x43aeac + iVar5 * 4)) {
                D32(0x43aeac + iVar5 * 4) = D32(0x43aeac + iVar5 * 4) + -0x24;
            }
        } else {
            D32(0x43aeac + iVar5 * 4) = D32(0x43aeac + iVar5 * 4) + -1;
            if (D32(0x43aeac + iVar5 * 4) < 0) {
                D32(0x43aeac + iVar5 * 4) = D32(0x43aeac + iVar5 * 4) + 0x24;
            }
        }
        FUN_00416345(iVar5);
    }
}

// 0x417684: ends the encounter and returns to the map.
void FUN_00417684(s16 param_1) {
    if (param_1 == 9) {
        D32(0x43b410) = 0;
        D32(0x436370) = 0;
        Pal_FadeToBlack(1);
        FUN_00415772();
        Spr_InvalidateAll();
    }
}

// 0x4176b5: encounter keys (space fires, left/right aims the cannon).
void FUN_004176b5(s16 param_1) {
    if (param_1 == 0x20) {
        if (D32(0x43b2ac) < 1) {
            FUN_00402cd2(P(0x43ad68), nullptr, 0);
            return;
        }
        if (D32(0x43ae3c) == 0) {
            D32(0x43ae3c) = 1;
            D32(0x43ae38) = D32(0x43ae34) + 5;
            D32(0x43b2ac) = D32(0x43b2ac) + -1;
            FUN_004149f6(4, D32(0x43b2ac));
            return;
        }
    } else if (param_1 == 0x14b) {
        if ((D32(0x43ae3c) == 0) && (0 < D32(0x43ae34))) {
            D32(0x43ae34) = D32(0x43ae34) + -6;
            FUN_00415f1d(D32(0x43ae34));
            return;
        }
    } else {
        if (param_1 != 0x14d) {
            return;
        }
        if ((D32(0x43ae3c) == 0) && (D32(0x43ae34) < 0x18)) {
            D32(0x43ae34) = D32(0x43ae34) + 6;
            FUN_00415f1d(D32(0x43ae34));
        }
    }
}

// 0x417779: scrolls the map in the direction of arrow key param_1.
void FUN_00417779(s16 param_1) {
    bool bVar1 = false;
    if (param_1 == 0x148) {
        if (D32(0x43b3f8) != 0) {
            D32(0x43b3f8) = D32(0x43b3f8) + -1;
            bVar1 = true;
            FUN_00414d66(-1, 0);
        }
    } else if (param_1 == 0x14b) {
        D32(0x43b3fc) = D32(0x43b3fc) + -1;
        FUN_00414d66(0, -1);
        bVar1 = true;
    } else if (param_1 == 0x14d) {
        D32(0x43b3fc) = D32(0x43b3fc) + 1;
        FUN_00414d66(0, 1);
        bVar1 = true;
    } else if ((param_1 == 0x150) && (D32(0x43b3f8) < 0x1e)) {
        D32(0x43b3f8) = D32(0x43b3f8) + 1;
        bVar1 = true;
        FUN_00414d66(1, 0);
    }
    if (bVar1) {
        for (int iVar2 = 0; iVar2 < D32(0x43aecc); iVar2 = iVar2 + 1) {
            FUN_00416238(iVar2);
        }
        for (int iVar2 = 0; iVar2 < D32(0x43ae88); iVar2 = iVar2 + 1) {
            FUN_00416345(iVar2);
        }
        Spr_Invalidate(D32(0x43ae74));
        Dirty_Mark(D32(0x43b400), D32(0x43b404), D32(0x43b408), D32(0x43b40c));
    }
}

// 0x41786c: map screen input (state 0): movement, digging, panels.
void FUN_0041786c(u16 param_1) {
    bool bVar2 = false; // player moved inside the window
    bool bVar1 = false; // map must scroll
    if (Timer_Expired(2)) {
        if (D32(0x43b2b8) < 2) {
            int iVar5 = 0x42;
            if (D32(0x43b2b8) != 1) {
                iVar5 = 100;
            }
            Timer_Set(2, iVar5);
        } else {
            Timer_Set(2, 0x7d);
        }
        FUN_00417567();
    }
    if ((((param_1 == 0x14b) || (param_1 == 0x148)) || (param_1 == 0x14d)) || (param_1 == 0x150)) {
        if (!Timer_Expired(3)) {
            param_1 = 0;
        } else {
            Timer_Set(3, 100);
        }
    }
    switch (param_1) {
    case 0x14d:
        D32(0x43ae80) = 1;
        if (D32(0x43ae78) < 10) {
            D32(0x43ae78) = D32(0x43ae78) + 1;
            bVar2 = true;
        } else {
            bVar1 = true;
        }
        break;
    case 0x148:
        if (D32(0x43ae7c) < 2) {
            if (D32(0x43b3f8) == 0) {
                if (D32(0x43ae7c) != 0) {
                    D32(0x43ae7c) = D32(0x43ae7c) + -1;
                    bVar2 = true;
                }
            } else {
                bVar1 = true;
            }
        } else {
            D32(0x43ae7c) = D32(0x43ae7c) + -1;
            bVar2 = true;
        }
        break;
    case 0x14b:
        D32(0x43ae80) = 0;
        if (D32(0x43ae78) < 2) {
            bVar1 = true;
        } else {
            D32(0x43ae78) = D32(0x43ae78) + -1;
            bVar2 = true;
        }
        break;
    case 0x150:
        if (D32(0x43ae7c) < 4) {
            D32(0x43ae7c) = D32(0x43ae7c) + 1;
            bVar2 = true;
        } else if (D32(0x43ae7c) + D32(0x43b3f8) < 0x22) {
            bVar1 = true;
        } else if (D32(0x43ae7c) < 5) {
            D32(0x43ae7c) = D32(0x43ae7c) + 1;
            bVar2 = true;
        }
        break;
    case 0x3f:
    case 0x2f:
    case 0x490:
        // open the clue panel
        D32(0x43b410) = 3;
        Timer_Set(1, 0x32);
        break;
    case 0xd:
    case 0x492:
    case 0x494:
        // open the info panel
        D32(0x43b228) = 0;
        D32(0x43b410) = 1;
        DU16(0x43b2d4) = param_1;
        Timer_Set(1, 0x32);
        break;
    case 0x20:
    case 0x493: {
        // dig
        u8 uVar4 = FUN_004149c8();
        if ((int)uVar4 < 0x1a) {
            if (D32(0x43b2a8) != 0) {
                D32(0x43b2a8) = D32(0x43b2a8) + -1;
                FUN_004149f6(3, D32(0x43b2a8));
            }
            D32(0x43ae84) = 0;
            Timer_Set(1, 0x3e);
            D32(0x43b410) = 5;
        }
        break;
    }
    default:
        break;
    }
    if ((bVar1) || (bVar2)) {
        if (D32(0x43b2a8) < 1) {
            FUN_00402e72(Rand() % 3 + 0x15d7, nullptr, 0, 1, 1);
            D32(0x43b2d0) = 1;
            return;
        }
        FUN_00402bad(2, 1);
        if (bVar1) {
            FUN_00417779((s16)param_1);
        }
        if (bVar2) {
            FUN_00416437(D32(0x43ae80));
        }
        int iVar5 = FUN_00415382();
        if (iVar5 == 1) {
            D32(0x43b2a8) = D32(0x43b2a8) + 0xb;
            FUN_00402bad(10, 2);
        }
        D32(0x43b2a8) = D32(0x43b2a8) + -1;
        FUN_004149f6(3, D32(0x43b2a8));
        if (iVar5 == 0) {
            D32(0x43b2ac) = D32(0x43b2ac) + 1;
            FUN_004149f6(4, D32(0x43b2ac));
            FUN_00402bad(0xb, 2);
        }
        if ((D32(0x43b2a8) < 0x14) && (D32(0x43b2b4) == 0)) {
            D32(0x43b2b4) = 1;
            FUN_00402d42(Rand() % 3 + 0x15d4, nullptr, 0);
        }
    }
    int iVar5 = 0;
    while (true) {
        if (D32(0x43ae88) <= iVar5) {
            return;
        }
        int iVar6 = FUN_004147ec(D32(0x43ae78));
        if ((iVar6 == D32(0x43ae9c + iVar5 * 4)) && (D32(0x43aeac + iVar5 * 4) == D32(0x43ae7c) + D32(0x43b3f8)))
            break;
        iVar5 = iVar5 + 1;
    }
    D32(0x43b220) = iVar5;
    D32(0x43b410) = 8;
}

// 0x417c4c: per-tick event handler of the module (state machine 0x43b410).
void FUN_00417c4c(u16 param_1) {
    int iVar2 = FUN_0040394c(param_1);
    if (iVar2 != 0) return;
    if (param_1 == 0xc) {
        FUN_00403f95();
    }
    if (D32(0x43b410) == 8) {
        // met a creature: start the encounter
        D32(0x43b410) = 9;
        Fx_CloseBlack();
        FUN_00415818();
        FUN_00416b99(3);
        D32(0x436370) = 1;
        if (D32(0x436488) != 0) {
            FUN_0040658a();
            D32(0x436488) = 0;
        }
        D32(0x43b21c) = 0;
        D32(0x43ae54) = 0;
        D32(0x43ae3c) = 0;
        D32(0x43ae50) = 3;
        FUN_00402d42(Rand() % 5 + 0x15f6, nullptr, 0);
        Timer_Set(1, 500);
        Event_Flush();
        return;
    }
    if ((D32(0x43ae54) != 0) && Timer_Expired(1)) {
        if (D32(0x43ae40) != 0) {
            if (D32(0x43ae58) == 0) {
                D32(0x43ae4c) = D32(0x43ae4c) - D32(0x436374);
                if ((D32(0x43ae44) == 1) && (D32(0x43ae4c) < 4)) {
                    FUN_00416a1a(0, 5);
                    D32(0x43b410) = 0xb;
                    Timer_Set(1, 0x1f);
                    return;
                }
                FUN_00417189();
            } else {
                D32(0x43ae4c) = D32(0x43ae4c) + D32(0x436374);
                if ((D32(0x43ae44) == 1) && (0x1d6 < D32(0x43ae4c))) {
                    FUN_00416a1a(0, 5);
                    D32(0x43b410) = 0xb;
                    Timer_Set(1, 0x1f);
                    return;
                }
                FUN_00417189();
            }
        }
        Timer_Set(1, 0x3e);
    }
    if (6 < D32(0x43b410)) {
        if (Timer_Expired(2)) {
            if (D32(0x43ae3c) != 0) {
                FUN_0041700c();
            }
            FUN_00416df6(0);
            FUN_00416df6(1);
            FUN_00416df6(2);
            Timer_Set(2, 0x3e);
        }
        FUN_004176b5((s16)param_1);
        if (D32(0x43b410) == 0) {
            return;
        }
    }
    if (D32(0x43ae70) == 1) {
        FUN_00414a9a(0);
        FUN_00402e72(D32(0x43b2a0), nullptr, 0, 1, 1);
        D32(0x43ae70) = 2;
    }
    if (D32(0x43ae64) == 1) {
        g_textOverflow = 1;
        FUN_00414a9a(1);
        if (D32(0x43b22c) < 0x14) {
            D32(0x43b230 + D32(0x43b22c) * 4) = D32(0x43b228);
            D32(0x43b22c) = D32(0x43b22c) + 1;
        }
        if (g_textFirstLine == 0) {
            g_textOverflow = -1;
            D32(0x43b228) = D32(0x43b228) + 1;
            FUN_00414a9a(2);
            if (g_textFirstLine != 0) {
                D32(0x43b228) = D32(0x43b228) + -1;
            }
        }
        D32(0x43ae64) = 2;
    }
    switch (D32(0x43b410)) {
    case 0:
        if (-1 < D32(0x43b220)) {
            // back from an encounter: settle the result
            D32(0x43b220) = -1;
            if (D32(0x43b21c) < 3) {
                if (D32(0x43b2c0) == 0) {
                    int iVar2b = Rand();
                    iVar2b = (iVar2b % 7) * 2 + 2;
                    int iVar4 = Rand();
                    if (D32(0x43b2a8) < iVar2b) {
                        iVar2b = D32(0x43b2a8);
                    }
                    int iVar6 = iVar4 % 3;
                    if (D32(0x43b2ac) < iVar4 % 3) {
                        iVar6 = D32(0x43b2ac);
                    }
                    D32(0x43b2a8) = D32(0x43b2a8) - iVar2b;
                    FUN_004149f6(3, D32(0x43b2a8));
                    D32(0x43b2ac) = D32(0x43b2ac) - iVar6;
                    FUN_004149f6(4, D32(0x43b2ac));
                    if ((iVar2b == 0) || (iVar6 == 0)) {
                        if (iVar2b != 0) {
                            FUN_00402e72(Rand() % 3 + 0x15eb, nullptr, 0, 1, 1);
                        }
                        if (iVar6 != 0) {
                            if (D32(0x43b2ac) == 0) {
                                FUN_00402e72(0x15f0, nullptr, 0, 1, 1);
                            } else if (iVar6 == 1) {
                                FUN_00402e72(0x15ee, nullptr, 0, 1, 1);
                            } else {
                                FUN_00402e72(0x15ef, nullptr, 0, 1, 1);
                            }
                        }
                    } else {
                        FUN_00402e72(0x15f1, nullptr, 0, 1, 1);
                    }
                }
            } else {
                int iVar2b = Rand();
                int iVar4 = Rand();
                D32(0x43b2a8) = D32(0x43b2a8) + (iVar2b % 9) * 2 + 5;
                FUN_004149f6(3, D32(0x43b2a8));
                D32(0x43b2ac) = D32(0x43b2ac) + iVar4 % 5 + 1;
                FUN_004149f6(4, D32(0x43b2ac));
            }
        }
        FUN_0041786c(param_1);
        if (D32(0x43b2a4) == 4) {
            D32(0x43b2d0) = 1;
        }
        break;
    case 1:
        if (Timer_Expired(1)) {
            FUN_00417411();
            Timer_Set(1, 0x32);
        }
        break;
    case 2:
        if (((param_1 == 0) || (0x7f < param_1)) && ((param_1 != 0x480 && ((param_1 != 0x492 && (param_1 != 0x494)))))) {
            DU16(0x43b2d4) = param_1;
        } else if (param_1 != DU16(0x43b2d4)) {
            DU16(0x43b2d4) = param_1;
            if (param_1 == 0x492) {
                if (D32(0x43b22c) < 2) {
                    return;
                }
                D32(0x43b22c) = D32(0x43b22c) + -2;
                D32(0x43b228) = D32(0x43b230 + D32(0x43b22c) * 4);
            } else {
                D32(0x43b228) = D32(0x43b228) + 1;
            }
            if ((D32(0x43b228) < D32(0x43b224)) && (param_1 != 0x480)) {
                D32(0x43ae64) = 0;
                D32(0x43ae60) = 8;
                FUN_00417411();
            } else {
                D32(0x43b410) = 1;
            }
        }
        break;
    case 3:
        if (Timer_Expired(1)) {
            FUN_00417483();
            Timer_Set(1, 0x32);
        }
        break;
    case 4:
        if (param_1 != 0) {
            D32(0x43b410) = 3;
        }
        break;
    case 5:
        if (Timer_Expired(1)) {
            FUN_00417501();
            Timer_Set(1, 0x3e);
        }
        break;
    case 6:
        FUN_00416a1a(1, 0);
        D32(0x43ae54) = 1;
        D32(0x43b410) = 7;
        break;
    case 9:
        if (((D32(0x43b21c) < 3) && (D32(0x43b2c0) != 0)) && (D32(0x43b2c4) < 6)) {
            bool expired = Timer_Expired(1);
            if (!expired || (Timer_Set(1, 500), D32(0x43ae50) < 1)) {
                if (D32(0x43ae50) < 1) {
                    D32(0x43ae58) = Rand() % 2;
                    D32(0x43ae4c) = 4;
                    if (D32(0x43ae58) == 0) {
                        D32(0x43ae4c) = 0x1d6;
                    }
                    FUN_00416a1a(0, 0);
                    FUN_00402bad(3, 1);
                    D32(0x43b410) = 10;
                    Timer_Set(1, 0x1f);
                    D32(0x43b2c4) = D32(0x43b2c4) + 1;
                }
            } else {
                D32(0x43ae50) = D32(0x43ae50) + -1;
            }
        } else {
            if (2 < D32(0x43b21c)) {
                FUN_00402e72(Rand() % 5 + 0x15e6, nullptr, 0, 1, 1);
            }
            if (D32(0x43b2c0) == 0) {
                FUN_00402e72(0x15e1, nullptr, 0, 1, 1);
            }
            FUN_00417684(9);
        }
        break;
    case 10:
        if (Timer_Expired(1)) {
            FUN_00417189();
            Timer_Set(1, 0x1f);
        }
        break;
    case 0xb:
        if (Timer_Expired(1)) {
            FUN_00417189();
            Timer_Set(1, 100);
        }
        break;
    default:
        break;
    }
}

// 0x418396
bool FUN_00418396() { return 0 < D32(0x43b2cc); }

// 0x4183a8
void FUN_004183a8() { FUN_00414cdf(); }

// 0x4183b2
void FUN_004183b2() { Fx_CloseBlack(); }

// 0x4183bc: one game: setup, event loop until 0x43b2d0 is set, ending.
void FUN_004183bc() {
    FUN_004154f4();
    FUN_00415772();
    Spr_Update(1);
    if (D32(0x43b2cc) == 0) {
        FUN_0040658a();
    }
    D32(0x43b410) = 3;
    D32(0x43b220) = -1;
    D32(0x43b2d0) = 0;
    Timer_Set(0, 0x32);
    Timer_Set(1, 200);
    Timer_Set(2, 100);
    Timer_Set(3, 100);
    Event_Flush();
    do {
        while (!Timer_Expired(0)) {
            int uVar2 = Event_Poll();
            FUN_00417c4c((u16)uVar2);
            Spr_Update(1);
        }
        Timer_Set(0, 0x32);
    } while (D32(0x43b2d0) == 0);
    if (D32(0x43b2a4) == 4) {
        FUN_00403e2c(0xfa);
        FUN_00419330(1000);
        FUN_004183b2();
    }
    FUN_004183a8();
}

// 0x418494: module entry.
void FUN_00418494() {
    FUN_00415b8b();
    D32(0x43b2cc) = 0;
    do {
        FUN_004183bc();
        D32(0x43b2cc) = D32(0x43b2cc) + 1;
    } while (!FUN_00418396());
}

// 0x4184b9: module cleanup.
void FUN_004184b9() {
    FUN_004183a8();
    FUN_00415a86();
    Spr_DeleteAll(0, 0);
    FUN_00402b28();
    FUN_00405901();
}

// 0x4184de: help narration (called from the toolbar).
void FUN_004184de(int param_1) {
    if (D32(0x436370) == 0) {
        if (param_1 == 0) {
            FUN_00402ec2(D32(0x43648c) + 0x15c9, 0, 1);
            D32(0x43648c) = D32(0x43648c) + 1;
            if (2 < D32(0x43648c)) {
                D32(0x43648c) = 0;
            }
        } else {
            FUN_00402ec2(0x15c8, 0, 1);
        }
    } else {
        FUN_00402ec2(0x15cc, 0, 1);
    }
}
