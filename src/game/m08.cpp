// Module 8 (0x40cf8c..0x40f7bc): maze activity with long-division problems.
//
// The player walks a cursor over a 13x9 grid of cells (16 numbered tiles are
// scattered on it); picking the tile that holds the next quotient digit of
// the long division shown in the top left advances the problem. Ghosts roam
// the maze and cost a life when they catch the player.
//
// Globals (all in the emulated data segment):
//   0x43a20c        sprite showing the division image (0x4351e8)
//   0x43a214        P: life icon animation (0x3178)
//   0x43a218        P: animation, per maze variant (0x316b + variant)
//   0x43a21c[2]     P: end-of-game animations (0x319a, 0x319b)
//   0x43a224        P: animation, per maze variant (0x316f + variant)
//   0x43a228[11]    P: digit tile animations (0x3189..0x3192, [10] = 0x3193)
//   0x43a254[6]     P: player animations (0x3194..0x3199)
//   0x43a26c[4]     P: ghost animations (0x3174..0x3177)
//   0x4351e8        P: offscreen image the division is drawn into
//   0x43a704        P: current maze wall map (from table 0x435a3c)
//   0x43a2bc[]      life sprites, 0x43a2c8 extra sprite
//   0x43a2cc[16]    tile sprites; 0x43a618/0x43a658 tile x/y; 0x43a698 digits
//   0x43a30c[17]    ghost sprites; ghost tables at 0x43a3b0 (x), 0x43a3f4 (y),
//                   0x43a438/0x43a47c (pixel offsets), 0x43a4c0 (frame),
//                   0x43a504 (anim), 0x43a548 (step), 0x43a58c (dir|0x80 flip),
//                   0x43a5d0 (timer)
//   0x43a358/0x43a35c player cell, 0x43a6f8 state
#include "game/common.h"

// 0x40cf8c
void FUN_0040cf8c() {
    u32 uVar2;
    int iVar3;

    Spr_DeleteAll(0, 0);
    D32(0x43a6ec) = 3;
    iVar3 = 0;
    do {
        D32(0x43a20c + iVar3 * 4) = Spr_Create(nullptr, D32(0x436ee4), D32(0x436ee8), 0xb);
        iVar3 = iVar3 + 1;
    } while (iVar3 < 1);
    D32(0x43a354) = 0;
    D32(0x43a36c) = 0;
    do {
        uVar2 = (u32)(Rand() % 4);
    } while ((int)uVar2 == (int)D16(0x43a70a));
    D16(0x43a70a) = (s16)uVar2;
    do {
        uVar2 = (u32)(Rand() % 8);
    } while ((int)uVar2 == (int)D16(0x43a708));
    D16(0x43a708) = (s16)uVar2;
    P(0x43a704) = P(0x435a3c + (int)D16(0x43a708) * 4);
    Res_Free(P(0x43a218));
    P(0x43a218) = nullptr;
    P(0x43a218) = Res_Load((int)D16(0x43a70a) + 0x316b);
    Res_Free(P(0x43a224));
    P(0x43a224) = nullptr;
    P(0x43a224) = Res_Load((int)D16(0x43a70a) + 0x316f);
}

// 0x40d098
void FUN_0040d098() {
    void* pvVar1 = Res_Load((int)D16(0x43a708) * 2 + 0x3179);
    Img_Draw(D32(0x436ee4), D32(0x436ee8), pvVar1, 0);
    Res_Free(pvVar1);
    Gfx_SaveBg(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    FUN_004032e3();
    FUN_0040e7f6();
    FUN_0040e7c0();
    D32(0x43a6f8) = 0;
    D32(0x43a3a4) = -1;
    FUN_0040d64b();
    Spr_Update(0);
    Screen_FlushAll();
    if (D32(0x43a380) == 0) {
        pvVar1 = Res_Load(0x317a);
        Pal_FadeTo((const u8*)pvVar1, 1);
        Res_Free(pvVar1);
    }
}

// 0x40d14f
void FUN_0040d14f() {
    int iVar1;

    iVar1 = 0;
    do {
        Mem_Free(P(0x4351e8 + iVar1 * 4));
        P(0x4351e8 + iVar1 * 4) = nullptr;
        iVar1 = iVar1 + 1;
    } while (iVar1 < 1);
    iVar1 = 0;
    do {
        Res_Free(P(0x43a228 + iVar1 * 4));
        P(0x43a228 + iVar1 * 4) = nullptr;
        iVar1 = iVar1 + 1;
    } while (iVar1 < 0xb);
    iVar1 = 0;
    do {
        Res_Free(P(0x43a254 + iVar1 * 4));
        P(0x43a254 + iVar1 * 4) = nullptr;
        iVar1 = iVar1 + 1;
    } while (iVar1 < 6);
    iVar1 = 0;
    do {
        Res_Free(P(0x43a26c + iVar1 * 4));
        P(0x43a26c + iVar1 * 4) = nullptr;
        iVar1 = iVar1 + 1;
    } while (iVar1 < 4);
    iVar1 = 0;
    do {
        Res_Free(P(0x43a21c + iVar1 * 4));
        P(0x43a21c + iVar1 * 4) = nullptr;
        iVar1 = iVar1 + 1;
    } while (iVar1 < 2);
    Res_Free(P(0x43a224));
    P(0x43a224) = nullptr;
    Res_Free(P(0x43a218));
    P(0x43a218) = nullptr;
    Res_Free(P(0x43a214));
    P(0x43a214) = nullptr;
}

// 0x40d225
void FUN_0040d225() {
    int iVar3;
    int local_c;
    u32 local_8;

    FUN_00403e91();
    D32(0x43a378) = (int)(FUN_004040cb() + 1);
    D32(0x43a37c) = (int)(FUN_004040a2() + 1);
    if (5 < D32(0x43a37c)) {
        D32(0x43a37c) = 5;
    }
    FUN_00402c78(0x7ef);
    P(0x43a214) = Res_Load(0x3178);
    P(0x43a21c) = Res_Load(0x319a);
    P(0x43a220) = Res_Load(0x319b);
    iVar3 = 0;
    do {
        P(0x43a26c + iVar3 * 4) = Res_Load(iVar3 + 0x3174);
        iVar3 = iVar3 + 1;
    } while (iVar3 < 4);
    iVar3 = 0;
    do {
        P(0x43a254 + iVar3 * 4) = Res_Load(iVar3 + 0x3194);
        iVar3 = iVar3 + 1;
    } while (iVar3 < 6);
    iVar3 = 0;
    do {
        P(0x43a228 + iVar3 * 4) = Res_Load(iVar3 + 0x3189);
        iVar3 = iVar3 + 1;
    } while (iVar3 < 10);
    P(0x43a250) = Res_Load(0x3193);
    Text_MeasureFont(DSTR(0x435a84), P(0x4318c4), &local_8, &local_c);
    local_c = local_c * 7;
    P(0x4351e8) = Img_Create((int)local_8, local_c);
    FUN_00402b7e(0x80d);
}

// 0x40d344: shuffles the tile digits and picks 16 non-adjacent grid cells.
void FUN_0040d344() {
    u8 bVar1;
    int uVar2;
    u32 uVar4;
    int iVar3, iVar5, iVar6, iVar7;
    u8 abStack_78[0x70] = {};
    int local_8;
    bool exhausted = false;

    iVar7 = 0;
    do {
        D32(0x43a698 + iVar7 * 4) = iVar7 % 9 + 1;
        iVar7 = iVar7 + 1;
    } while (iVar7 < 0x12);
    iVar7 = 0;
    do {
        local_8 = Rand();
        local_8 = local_8 % 9;
        uVar2 = D32(0x43a698 + iVar7 * 4);
        D32(0x43a698 + iVar7 * 4) = D32(0x43a698 + local_8 * 4);
        D32(0x43a698 + local_8 * 4) = uVar2;
        local_8 = Rand();
        local_8 = local_8 % 9;
        uVar2 = D32(0x43a6bc + iVar7 * 4);
        D32(0x43a6bc + iVar7 * 4) = D32(0x43a6bc + local_8 * 4);
        D32(0x43a6bc + local_8 * 4) = uVar2;
        iVar7 = iVar7 + 1;
    } while (iVar7 < 9);
    iVar7 = 0;
    do {
        abStack_78[iVar7] = (u8)iVar7;
        iVar7 = iVar7 + 1;
    } while (iVar7 < 0x6f);
    iVar7 = 0;
    do {
        local_8 = Rand();
        local_8 = local_8 % 0x6f;
        bVar1 = abStack_78[iVar7];
        abStack_78[iVar7] = abStack_78[local_8];
        abStack_78[local_8] = bVar1;
        iVar7 = iVar7 + 1;
    } while (iVar7 < 0x6f);
    local_8 = 0;
    iVar7 = 0;
    do {
        do {
            // The original reads past its 111-entry stack array when the
            // random placement jams (~1e-4 chance); wrap around instead and
            // stop rejecting neighbouring cells so the loop terminates.
            if (local_8 >= 0x6f) {
                local_8 = 0;
                exhausted = true;
            }
            bVar1 = abStack_78[local_8];
            uVar4 = (u32)bVar1;
            local_8 = local_8 + 1;
            if ((bVar1 < 2) || (uVar4 == 0x54) || ((10 < bVar1) && (bVar1 < 0xd)) ||
                ((0x2a < bVar1) && (bVar1 < 0x30)) || ((0x37 < bVar1) && (bVar1 < 0x3d)) ||
                ((0x44 < bVar1) && (bVar1 < 0x4a))) {
                uVar4 = 0xffffffff;
            }
        } while (uVar4 == 0xffffffff);
        D32(0x43a618 + iVar7 * 4) = (int)uVar4 % 0xd;
        D32(0x43a658 + iVar7 * 4) = (int)uVar4 / 0xd;
        iVar3 = 0;
        if (0 < iVar7 && !exhausted) {
            do {
                iVar6 = D32(0x43a618 + iVar7 * 4) - D32(0x43a618 + iVar3 * 4);
                if (iVar6 < 0) {
                    iVar6 = -iVar6;
                }
                iVar5 = D32(0x43a658 + iVar7 * 4) - D32(0x43a658 + iVar3 * 4);
                if (iVar5 < 0) {
                    iVar5 = -iVar5;
                }
                if ((iVar6 < 2) && (iVar5 < 2)) {
                    iVar7 = iVar7 + -1;
                    break;
                }
                iVar3 = iVar3 + 1;
            } while (iVar3 < iVar7);
        }
        iVar7 = iVar7 + 1;
    } while (iVar7 <= 0xf);
}

// 0x40d4ae: builds a new division problem for difficulty param_1.
void FUN_0040d4ae(int param_1) {
    D32(0x43a6f0) = D32(0x43a6f0) + 1;
    D32(0x43a39c) = 0;
    D32(0x43a388) = (int)(1 < param_1);
    if ((u32)(param_1 - 1) < 2) {
        D32(0x43a390) = Rand() % 9 + 1;
        D32(0x43a38c) = D32(0x43a698);
        D32(0x43a394) = D32(0x43a698) * D32(0x43a390);
        D32(0x43a398) = 0;
        return;
    }
    if (param_1 == 3) {
        D32(0x43a390) = Rand() % 8 + 2;
        D32(0x43a38c) = D32(0x43a698);
        if (D32(0x43a390) == 0) {
            D32(0x43a398) = 0;
        } else {
            D32(0x43a398) = Rand() % D32(0x43a390);
        }
        D32(0x43a394) = D32(0x43a38c) * D32(0x43a390) + D32(0x43a398);
        return;
    }
    if (param_1 == 4) {
        D32(0x43a390) = Rand() % 8 + 2;
        D32(0x43a38c) = D32(0x43a698) * 10 + D32(0x43a69c);
        if (D32(0x43a390) == 0) {
            D32(0x43a398) = 0;
        } else {
            D32(0x43a398) = Rand() % D32(0x43a390);
        }
        D32(0x43a394) = D32(0x43a38c) * D32(0x43a390) + D32(0x43a398);
        return;
    }
    if (param_1 != 5) {
        return;
    }
    D32(0x43a390) = Rand() % 8 + 2;
    D32(0x43a38c) = D32(0x43a698) * 10 + D32(0x43a69c);
    if (D32(0x43a390) == 0) {
        D32(0x43a398) = 0;
    } else {
        D32(0x43a398) = Rand() % D32(0x43a390);
    }
    D32(0x43a394) = D32(0x43a38c) * D32(0x43a390) + D32(0x43a398);
}

// 0x40d64b: draws the long division (as far as solved) into image 0x4351e8.
// Ported from the disassembly; register/stack names noted.
void FUN_0040d64b() {
    const void* font = P(0x4318c4);
    char buf[80];       // ebp-0xb8
    char sDiv[16];      // ebp-0x4  divisor
    char sDvd[16];      // ebp-0x10 dividend
    char sQuo[16];      // ebp-0x1c quotient
    Rect r;             // ebp-0x68
    int lx, ly, ldx, ldy;  // ebp-0x58..-0x4c
    int y;              // ebp-0x28
    int qy;             // ebp-0x30 quotient row
    int tx;             // ebp-0x2c
    int qx = 0;         // ebp-0x34
    int prod = 0;       // ebp-0x48
    int di;             // ebp-0x44 dividend digit index
    int qi;             // ebp-0x40 quotient digit index
    int x;              // edi
    int n;              // esi
    int rem;            // ebx

    int posX = D32(0x435a5c);
    int posY = D32(0x435a60);
    int fh = Font_Height(font) + -3;
    int cw = Font_CharWidth(0x30, font);
    sprintf(sDiv, DSTR(0x435a95), D32(0x43a390));
    sprintf(sDvd, DSTR(0x435a98), D32(0x43a394));
    sprintf(sQuo, DSTR(0x435a9b), D32(0x43a38c));
    Gfx_SetTargetImg(P(0x4351e8));
    Img_FullRect(P(0x4351e8), &r);
    Gfx_FillRect(r.x, r.y, r.w, r.h, Img_Key(P(0x4351e8)));
    x = cw * 6 + r.x;
    y = r.y + 8;
    qy = y;
    // (0x40d735..0x40d81e is unreachable code)
    lx = (int)strlen(sDiv) * cw + x + 4;
    ly = y + fh + 1;
    ldx = ((int)strlen(sDvd) + 1) * cw - 4;
    ldy = 0;
    Gfx_Line(lx, ly, ldx, ldy, 0x33);
    lx = cw + x + cw / 2 + -3;
    ly = y + fh + 2;
    ldx = 1;
    ldy = 0;
    Gfx_Line(lx, ly, ldx, ldy, 0x33);
    y = y + fh + 2;
    tx = x;
    sprintf(buf, DSTR(0x435aa5), D32(0x43a390), D32(0x43a394));
    Text_DrawScreenFont(tx, y, buf, font, 0x33);
    n = 0;
    x = (((int)strlen(sDiv) + (int)strlen(sDvd) + 1) - (int)strlen(sQuo)) * cw + x;
    di = 0;
    rem = 0;
    while (rem < D32(0x43a390)) {
        rem = rem * 10 + ((u8)sDvd[di] - 0x30);
        di = di + 1;
    }
    qi = 0;
    while ((u32)qi < (u32)strlen(sQuo)) {
        if (D32(0x43a39c) < n) {
            n = -1;
            goto done;
        }
        qx = x;
        D32(0x43a3a8) = (u8)sQuo[qi] - 0x30;
        if (n < D32(0x43a39c)) {
            prod = ((u8)sQuo[qi] - 0x30) * D32(0x43a390);
            Font_DrawChar(qx, qy, (u8)sQuo[qi], font, 0x33);
            n = n + 1;
        } else {
            if (D32(0x43a3a4) < 0) {
                Font_DrawChar(qx, qy, 0x3f, font, 0x33);
                n = -1;
                goto done;
            }
            int color = 0x57;
            if (D32(0x43a3a4) == D32(0x43a3a8)) {
                color = 0x33;
            }
            Font_DrawChar(qx, qy, D32(0x43a3a4) + 0x30, font, color);
            n = n + 1;
            prod = D32(0x43a3a4) * D32(0x43a390);
        }
        x = x + cw;
        y = y + fh;
        sprintf(buf, DSTR(0x435aab), prod);
        tx = x - (int)strlen(buf) * cw;
        Text_DrawScreenFont(tx, y, buf, font, 0x33);
        sprintf(buf, DSTR(0x435aae), rem);
        ldx = (int)strlen(buf) * cw;
        lx = x - ldx;
        ly = y + fh;
        Gfx_Line(lx, ly, ldx, ldy, 0x33);
        y = y + fh + 2;
        rem = rem - prod;
        sprintf(buf, DSTR(0x435ab1), rem);
        tx = x - (int)strlen(buf) * cw;
        Text_DrawScreenFont(tx, y, buf, font, 0x33);
        if (D32(0x43a39c) < n) {
            rem = rem + prod;
            n = -1;
            goto done;
        }
        if ((u32)di < (u32)strlen(sDvd)) {
            tx = x;
            Font_DrawChar(tx, y, (u8)sDvd[di], font, 0x33);
            rem = rem * 10 + ((u8)sDvd[di] - 0x30);
            di = di + 1;
            if (rem < D32(0x43a390)) {
                rem = rem * 10 + ((u8)sDvd[di] - 0x30);
                di = di + 1;
                qi = qi + 1;
                D32(0x43a3a8) = (u8)sQuo[qi] - 0x30;
                qx = x;
                if (n < D32(0x43a39c)) {
                    Font_DrawChar(qx, qy, (u8)sQuo[qi], font, 0x33);
                    n = n + 1;
                } else {
                    if (D32(0x43a3a4) < 0) {
                        Font_DrawChar(qx, qy, 0x3f, font, 0x33);
                        n = -1;
                        goto done;
                    }
                    int color = 0x57;
                    if (D32(0x43a3a4) == D32(0x43a3a8)) {
                        color = 0x33;
                    }
                    Font_DrawChar(qx, qy, D32(0x43a3a4) + 0x30, font, color);
                    n = n + 1;
                }
            }
        }
        qi = qi + 1;
    }
done:
    if ((-1 < D32(0x43a3a4)) && (D32(0x43a3a4) != D32(0x43a3a8))) {
        n = -1;
    }
    if (n < 0) {
        if ((rem < D32(0x43a394)) && (D32(0x43a390) <= rem)) {
            // Wrong digit: show the remaining partial division on its own.
            sprintf(sDvd, DSTR(0x435ab4), rem);
            x = r.x + 10;
            y = r.y + 8;
            lx = (int)strlen(sDiv) * cw + x + 4;
            ly = y + fh + 1;
            ldx = ((int)strlen(sDvd) + 1) * cw + -4;
            ldy = 0;
            Gfx_Line(lx, ly, ldx, ldy, 0x33);
            lx = cw + x + cw / 2 + -3;
            ly = y + fh + 2;
            ldx = 1;
            ldy = 0;
            Gfx_Line(lx, ly, ldx, ldy, 0x33);
            qy = y;
            y = y + fh + 2;
            tx = x;
            sprintf(buf, DSTR(0x435ab7), D32(0x43a390), rem);
            Text_DrawScreenFont(tx, y, buf, font, 0x33);
            x = ((int)strlen(sDiv) + (int)strlen(sDvd)) * cw + x;
            qx = x;
            if (D32(0x43a3a4) < 0) {
                Font_DrawChar(qx, qy, 0x3f, font, 0x33);
            } else {
                Font_DrawChar(qx, qy, D32(0x43a3a4) + 0x30, font, 0x33);
            }
        }
    } else if (D32(0x43a398) != 0) {
        qx = qx + cw;
        sprintf(buf, DSTR(0x435abd), D32(0x43a398));
        n = n + 1;
        Text_DrawScreenFont(qx, qy, buf, font, 0x33);
    }
    if (0 < n) {
        D32(0x43a39c) = -1;
    }
    Gfx_ResetTarget();
    Spr_Invalidate(D32(0x43a20c));
    Spr_SetImage(D32(0x43a20c), P(0x4351e8));
    Spr_SetPos(D32(0x43a20c), posX, posY);
}

// 0x40dec4
void FUN_0040dec4(int param_1) {
    int y = D32(0x435a68);
    int x = D32(0x435a64);
    if (D32(0x43a2c8) != 0) {
        Spr_Delete(D32(0x43a2c8), 0);
    }
    D32(0x43a368) = param_1;
    const void* img = Ang_FrameImage(P(0x43a218), param_1);
    D32(0x43a2c8) = Spr_Create(img, x, y, 1);
}

// 0x40df2b
void FUN_0040df2b(int param_1) {
    Point p;
    if (D32(0x43a2c8) != 0) {
        D32(0x43a368) = param_1;
        Ang_FrameInfo(P(0x43a218), param_1, &p);
        p.x = p.x + 200;
        p.y = p.y + 0x91;
        const void* img = Ang_FrameImage(P(0x43a218), param_1);
        Spr_SetImage(D32(0x43a2c8), img);
        Spr_SetPos(D32(0x43a2c8), p.x, p.y);
    }
}

// 0x40df9c
void FUN_0040df9c(int param_1, int param_2) {
    Point p;
    Spr_SetImage(D32(0x43a20c), nullptr);
    if (D32(0x43a354) != 0) {
        Spr_Delete(D32(0x43a354), 0);
    }
    D32(0x43a36c) = param_2;
    D32(0x43a370) = param_1;
    Ang_FrameInfo(P(0x43a21c + param_1 * 4), param_2, &p);
    p.x = p.x + 200;
    p.y = p.y + 0x91;
    const void* img = Ang_FrameImage(P(0x43a21c + param_1 * 4), param_2);
    D32(0x43a354) = Spr_Create(img, p.x, p.y, 2);
}

// 0x40e02f
void FUN_0040e02f(int param_1, int param_2) {
    Point p;
    if (D32(0x43a354) != 0) {
        D32(0x43a370) = param_1;
        D32(0x43a36c) = param_2;
        Ang_FrameInfo(P(0x43a21c + param_1 * 4), param_2, &p);
        p.x = p.x + 200;
        p.y = p.y + 0x91;
        const void* img = Ang_FrameImage(P(0x43a21c + param_1 * 4), param_2);
        Spr_SetImage(D32(0x43a354), img);
        Spr_SetPos(D32(0x43a354), p.x, p.y);
    }
}

// 0x40e0ad: removes ghost param_1.
void FUN_0040e0ad(int param_1) {
    Spr_Delete(D32(0x43a30c + param_1 * 4), 1);
    D32(0x43a3ac) = D32(0x43a3ac) + -1;
    for (; param_1 < D32(0x43a3ac); param_1 = param_1 + 1) {
        D32(0x43a30c + param_1 * 4) = D32(0x43a310 + param_1 * 4);
        D32(0x43a5d0 + param_1 * 4) = D32(0x43a5d4 + param_1 * 4);
        D32(0x43a58c + param_1 * 4) = D32(0x43a590 + param_1 * 4);
        D32(0x43a548 + param_1 * 4) = D32(0x43a54c + param_1 * 4);
        D32(0x43a438 + param_1 * 4) = D32(0x43a43c + param_1 * 4);
        D32(0x43a47c + param_1 * 4) = D32(0x43a480 + param_1 * 4);
        D32(0x43a3b0 + param_1 * 4) = D32(0x43a3b4 + param_1 * 4);
        D32(0x43a3f4 + param_1 * 4) = D32(0x43a3f8 + param_1 * 4);
        D32(0x43a4c0 + param_1 * 4) = D32(0x43a4c4 + param_1 * 4);
        D32(0x43a504 + param_1 * 4) = D32(0x43a508 + param_1 * 4);
    }
}

// 0x40e167: removes the tile under the player.
void FUN_0040e167() {
    int iVar1;
    int iVar3;

    iVar3 = 0;
    while (true) {
        if (D32(0x43a614) <= iVar3) {
            return;
        }
        if ((D32(0x43a618 + iVar3 * 4) == D32(0x43a358)) &&
            (D32(0x43a658 + iVar3 * 4) == D32(0x43a35c)))
            break;
        iVar3 = iVar3 + 1;
    }
    if (iVar3 != 0) {
        iVar1 = D32(0x43a618 + iVar3 * 4);
        D32(0x43a618 + iVar3 * 4) = D32(0x43a618);
        D32(0x43a618) = iVar1;
        iVar1 = D32(0x43a658 + iVar3 * 4);
        D32(0x43a658 + iVar3 * 4) = D32(0x43a658);
        D32(0x43a658) = iVar1;
        iVar1 = D32(0x43a2cc + iVar3 * 4);
        D32(0x43a2cc + iVar3 * 4) = D32(0x43a2cc);
        D32(0x43a2cc) = iVar1;
    }
    Spr_Delete(D32(0x43a2cc), 1);
    D32(0x43a614) = D32(0x43a614) + -1;
    for (iVar3 = 0; iVar3 < D32(0x43a614); iVar3 = iVar3 + 1) {
        D32(0x43a2cc + iVar3 * 4) = D32(0x43a2d0 + iVar3 * 4);
        D32(0x43a618 + iVar3 * 4) = D32(0x43a61c + iVar3 * 4);
        D32(0x43a658 + iVar3 * 4) = D32(0x43a65c + iVar3 * 4);
        D32(0x43a698 + iVar3 * 4) = D32(0x43a69c + iVar3 * 4);
    }
}

// 0x40e231: index of the tile under the player, or -1.
int FUN_0040e231() {
    int iVar1 = 0;
    while (true) {
        if (D32(0x43a614) <= iVar1) {
            return -1;
        }
        if ((D32(0x43a618 + iVar1 * 4) == D32(0x43a358)) &&
            (D32(0x43a658 + iVar1 * 4) == D32(0x43a35c)))
            break;
        iVar1 = iVar1 + 1;
    }
    return iVar1;
}

// 0x40e264: creates the sprite of tile param_1.
void FUN_0040e264(int param_1, int param_2) {
    int local_c = D32(0x43a618 + param_1 * 4) * 0x31 + 0xc;
    int iVar1 = D32(0x43a658 + param_1 * 4);
    if (D32(0x43a618 + param_1 * 4) < 5) {
        local_c = D32(0x43a618 + param_1 * 4) * 0x31 + 0x11;
    }
    const void* img = Ang_FrameImage(P(0x43a250), 0);
    if (D32(0x43a378) < 3) {
        img = Ang_FrameImage(P(0x43a228 + D32(0x43a698 + param_1 * 4) * 4), param_2);
    }
    D32(0x43a2cc + param_1 * 4) = Spr_Create(img, local_c, iVar1 * 0x2d + 0xb, 10);
}

// 0x40e2f2
void FUN_0040e2f2() {
    Point p;
    if (D32(0x43a350) != 0) {
        Spr_Delete(D32(0x43a350), 0);
    }
    D32(0x43a360) = 0;
    Ang_FrameInfo(P(0x43a224), 0, &p);
    p.y = p.y + -0x1a;
    const void* img = Ang_FrameImage(P(0x43a224), 0);
    D32(0x43a350) = Spr_Create(img, p.x, p.y, 0x14);
}

// 0x40e35e
void FUN_0040e35e(int param_1) {
    Point p;
    if (D32(0x43a350) != 0) {
        D32(0x43a360) = param_1;
        Ang_FrameInfo(P(0x43a224), param_1, &p);
        p.y = p.y + -0x1a;
        const void* img = Ang_FrameImage(P(0x43a224), D32(0x43a360));
        Spr_SetImage(D32(0x43a350), img);
        Spr_SetPos(D32(0x43a350), p.x, p.y);
    }
}

// 0x40e3c7: creates the player sprite (animation param_1, frame param_2).
void FUN_0040e3c7(int param_1, int param_2) {
    Point p;   // local_14
    Point p0;  // local_c
    if (D32(0x43a350) != 0) {
        Spr_Delete(D32(0x43a350), 0);
    }
    D32(0x43a360) = param_2;
    D32(0x43a364) = param_1;
    Ang_FrameInfo(P(0x43a254 + param_1 * 4), 0, &p0);
    Ang_FrameInfo(P(0x43a254 + param_1 * 4), param_2, &p);
    p.x = p.x + ((D32(0x43a358) * 0x31 + 2) - p0.x);
    p.y = p.y + (D32(0x43a35c) * 0x2d - p0.y);
    if (D32(0x43a358) < 5) {
        p.x = p.x + 5;
    }
    const void* img = Ang_FrameImage(P(0x43a254 + param_1 * 4), param_2);
    D32(0x43a350) = Spr_Create(img, p.x, p.y, 0x14);
}

// 0x40e47f: updates the player sprite.
void FUN_0040e47f(int param_1, int param_2) {
    Point p;   // local_14
    Point p0;  // local_c
    if (D32(0x43a350) != 0) {
        D32(0x43a360) = param_2;
        D32(0x43a364) = param_1;
        Ang_FrameInfo(P(0x43a254 + param_1 * 4), 0, &p0);
        Ang_FrameInfo(P(0x43a254 + param_1 * 4), param_2, &p);
        p.x = p.x + ((D32(0x43a358) * 0x31 + 2) - p0.x);
        p.y = p.y + (D32(0x43a35c) * 0x2d - p0.y);
        if (D32(0x43a358) < 5) {
            p.x = p.x + 5;
        }
        if ((D32(0x43a6f8) == 3) && (param_2 != 0)) {
            if (D32(0x43a700) == 1) {
                p.x = p.x + param_2 * -10;
            } else if (D32(0x43a700) == 2) {
                p.y = p.y + param_2 * -10;
            } else if (D32(0x43a700) == 3) {
                p.x = p.x + param_2 * 10;
            } else if (D32(0x43a700) == 4) {
                p.y = p.y + param_2 * 10;
            }
        }
        const void* img = Ang_FrameImage(P(0x43a254 + param_1 * 4), D32(0x43a360));
        Spr_SetImage(D32(0x43a350), img);
        Spr_SetPos(D32(0x43a350), p.x, p.y);
    }
}

// 0x40e58a: creates the sprite of ghost param_1.
void FUN_0040e58a(int param_1, int param_2, int param_3) {
    Point p;
    D32(0x43a4c0 + param_1 * 4) = param_3;
    D32(0x43a504 + param_1 * 4) = param_2;
    if ((D8(0x43a58c + param_1 * 4) & 0x80) != 0) {
        param_2 = param_2 + 1;
    }
    Ang_FrameInfo(P(0x43a26c + param_2 * 4), param_3, &p);
    p.x = p.x + D32(0x43a3b0 + param_1 * 4) * 0x31 + -0x20;
    p.y = p.y + D32(0x43a3f4 + param_1 * 4) * 0x2d + -5;
    if (D32(0x43a3b0 + param_1 * 4) < 5) {
        p.x = p.x + 5;
    }
    if ((D8(0x43a58c + param_1 * 4) & 0x80) != 0) {
        p.x = p.x + 10;
    }
    const void* img = Ang_FrameImage(P(0x43a26c + param_2 * 4), param_3);
    D32(0x43a30c + param_1 * 4) = Spr_Create(img, p.x, p.y, 0x1e);
}

// 0x40e637: updates the sprite of ghost param_1.
void FUN_0040e637(int param_1, int param_2, int param_3) {
    Point p;
    if (D32(0x43a30c + param_1 * 4) != 0) {
        D32(0x43a4c0 + param_1 * 4) = param_3;
        D32(0x43a504 + param_1 * 4) = param_2;
        if ((D8(0x43a58c + param_1 * 4) & 0x80) != 0) {
            param_2 = param_2 + 1;
        }
        Ang_FrameInfo(P(0x43a26c + param_2 * 4), param_3, &p);
        p.x = p.x + D32(0x43a3b0 + param_1 * 4) * 0x31 + D32(0x43a438 + param_1 * 4) + -0x20;
        p.y = p.y + D32(0x43a3f4 + param_1 * 4) * 0x2d + D32(0x43a47c + param_1 * 4) + -5;
        if (D32(0x43a504 + param_1 * 4) == 1) {
            p.y = p.y + 8;
            p.x = p.x + 4;
        }
        if (D32(0x43a3b0 + param_1 * 4) < 5) {
            p.x = p.x + 5;
        }
        if ((D8(0x43a58c + param_1 * 4) & 0x80) != 0) {
            p.x = p.x + 10;
        }
        const void* img = Ang_FrameImage(P(0x43a26c + param_2 * 4), param_3);
        Spr_SetImage(D32(0x43a30c + param_1 * 4), img);
        Spr_SetPos(D32(0x43a30c + param_1 * 4), p.x, p.y);
    }
}

// 0x40e71d: spawns a ghost at cell (param_1, param_2).
void FUN_0040e71d(int param_1, int param_2) {
    if (D32(0x43a3ac) < 0x11) {
        int n = D32(0x43a3ac);
        D32(0x43a5d0 + n * 4) = -30;
        D32(0x43a58c + n * 4) = 1;
        D32(0x43a548 + n * 4) = 0;
        D32(0x43a438 + n * 4) = 0;
        D32(0x43a47c + n * 4) = 0;
        D32(0x43a3b0 + n * 4) = param_1;
        D32(0x43a3f4 + n * 4) = param_2;
        FUN_0040e58a(D32(0x43a3ac), 1, 0);
        D32(0x43a3ac) = D32(0x43a3ac) + 1;
        FUN_00402bad(5, 2);
    }
}

// 0x40e7c0: places the player on the start cell.
void FUN_0040e7c0() {
    D32(0x43a374) = -1;
    D32(0x43a6fc) = 0;
    D32(0x43a358) = 6;
    D32(0x43a35c) = 6;
    FUN_0040e3c7(4, 0);
}

// 0x40e7f6: sets up lives, tiles and the first problem.
void FUN_0040e7f6() {
    Point p;
    int iVar1;
    int iVar3;

    D32(0x43a6f0) = 0;
    const void* img = Ang_FrameImage(P(0x43a214), 0);
    iVar1 = Img_W(img);
    Pt_Set(&p, 0, 0);
    for (iVar3 = 0; iVar3 < D32(0x43a6ec); iVar3 = iVar3 + 1) {
        D32(0x43a2bc + iVar3 * 4) = Spr_Create(img, p.x, p.y, 0xc);
        p.x = p.x + iVar1;
    }
    D32(0x43a6ec) = D32(0x43a6ec) + -1;
    Spr_Delete(D32(0x43a2bc + D32(0x43a6ec) * 4), 0);
    FUN_0040d344();
    FUN_0040dec4(0);
    FUN_0040d4ae(D32(0x43a37c));
    D32(0x43a3ac) = 0;
    D32(0x43a614) = 0;
    iVar1 = 0;
    do {
        FUN_0040e264(iVar1, 5);
        D32(0x43a614) = D32(0x43a614) + 1;
        iVar1 = iVar1 + 1;
    } while (iVar1 < 0x10);
    D32(0x43a350) = 0;
    D32(0x43a6f8) = 1;
}

// 0x40e8cf
void FUN_0040e8cf() {
    if (D32(0x43a368) < 9) {
        D32(0x43a368) = D32(0x43a368) + 1;
        FUN_0040df2b(D32(0x43a368));
    }
}

// 0x40e8ef
void FUN_0040e8ef() {
    if (D32(0x43a36c) < (int)((u32)(D32(0x43a370) == 1) * -7 + 0x13)) {
        D32(0x43a36c) = D32(0x43a36c) + 1;
        FUN_0040e02f(D32(0x43a370), D32(0x43a36c));
    }
}

// 0x40e931: advances the animation of ghost param_1.
void FUN_0040e931(int param_1) {
    D32(0x43a4c0 + param_1 * 4) = D32(0x43a4c0 + param_1 * 4) + 1;
    if (2 < D32(0x43a4c0 + param_1 * 4)) {
        D32(0x43a4c0 + param_1 * 4) = 0;
        if (D32(0x43a504 + param_1 * 4) == 1) {
            D32(0x43a504 + param_1 * 4) = 2;
        }
    }
    FUN_0040e637(param_1, D32(0x43a504 + param_1 * 4), D32(0x43a4c0 + param_1 * 4));
}

// 0x40e97f: wrong-answer animation of the tile under the player. param_1 is
// really whatever was in eax (the caller's Timer_Expired result); it is only
// used as the image when no tile is selected.
void FUN_0040e97f(int param_1) {
    const void* img = nullptr;  // original: (void*)eax, not a valid image
    (void)param_1;
    if (-1 < D32(0x43a374)) {
        D32(0x43a6e8) = D32(0x43a6e8) + -1;
        if (D32(0x43a6e8) < 0) {
            img = Ang_FrameImage(P(0x43a228 + D32(0x43a698 + D32(0x43a374) * 4) * 4), 5);
            D32(0x43a6f8) = D32(0x43a6f8) + 1;
        } else {
            img = Ang_FrameImage(P(0x43a228 + D32(0x43a698 + D32(0x43a374) * 4) * 4),
                                 D32(0x43a6e8));
        }
    }
    Spr_SetImage(D32(0x43a2cc + D32(0x43a374) * 4), img);
}

// 0x40e9f9: wall test of the maze map for moving from cell (x, y) in direction
// param_3 (1 left, 2 up, 3 right, 4 down); 0 = free. Zero-extended byte.
int FUN_0040e9f9(int param_1, int param_2, int param_3) {
    const u8* map = (const u8*)P(0x43a704);
    int iVar1 = param_2 * 0x1c + param_1;
    if (param_3 == 1) {
        return map[0xe + iVar1];
    }
    if (param_3 == 2) {
        return map[iVar1];
    }
    if (param_3 == 3) {
        return map[0xf + iVar1];
    }
    if (param_3 != 4) {
        return 1;
    }
    return map[0x1c + iVar1];
}

// 0x40ea51: moves ghost param_1 one step.
void FUN_0040ea51(int param_1) {
    int iVar3;
    u32 uVar2;
    u32 uVar4;
    int iVar5;
    int local_14, local_10, local_c, local_8;

    uVar4 = DU32(0x43a58c + param_1 * 4) & 0x7f;
    local_8 = D32(0x43a3b0 + param_1 * 4);
    local_c = D32(0x43a3f4 + param_1 * 4);
    local_10 = 0;
    local_14 = 0;
    if (uVar4 != 0) {
        if (FUN_0040e9f9(local_8, local_c, (int)uVar4) == 0) {
            iVar5 = D32(0x43a548 + param_1 * 4) + 1;
            if (uVar4 == 1) {
                if (iVar5 < 10) {
                    local_10 = iVar5 * -5;
                } else {
                    local_8 = local_8 + -1;
                    iVar5 = 0;
                    iVar3 = Rand();
                    if (iVar3 % 3 == 1) {
                        if (FUN_0040e9f9(local_8, local_c, 2) == 0) {
                            uVar4 = 2;
                        }
                    } else if ((iVar3 % 3 == 2) && (FUN_0040e9f9(local_8, local_c, 4) == 0)) {
                        uVar4 = 4;
                    }
                }
            } else if (uVar4 == 2) {
                if (iVar5 < 9) {
                    local_14 = iVar5 * -5;
                } else {
                    local_c = local_c + -1;
                    iVar5 = 0;
                    iVar3 = Rand();
                    if (iVar3 % 3 == 1) {
                        if (FUN_0040e9f9(local_8, local_c, 1) == 0) {
                            uVar4 = 1;
                        }
                    } else if ((iVar3 % 3 == 2) && (FUN_0040e9f9(local_8, local_c, 3) == 0)) {
                        uVar4 = 3;
                    }
                }
            } else if (uVar4 == 3) {
                if (iVar5 < 10) {
                    local_10 = iVar5 * 5;
                } else {
                    local_8 = local_8 + 1;
                    iVar5 = 0;
                    iVar3 = Rand();
                    if (iVar3 % 3 == 1) {
                        if (FUN_0040e9f9(local_8, local_c, 2) == 0) {
                            uVar4 = 2;
                        }
                    } else if ((iVar3 % 3 == 2) && (FUN_0040e9f9(local_8, local_c, 4) == 0)) {
                        uVar4 = 4;
                    }
                }
            } else if (uVar4 == 4) {
                if (iVar5 < 9) {
                    local_14 = iVar5 * 5;
                } else {
                    local_c = local_c + 1;
                    iVar5 = 0;
                    iVar3 = Rand();
                    if (iVar3 % 3 == 1) {
                        if (FUN_0040e9f9(local_8, local_c, 1) == 0) {
                            uVar4 = 1;
                        }
                    } else if ((iVar3 % 3 == 2) && (FUN_0040e9f9(local_8, local_c, 3) == 0)) {
                        uVar4 = 3;
                    }
                }
            }
            D32(0x43a548 + param_1 * 4) = iVar5;
            D32(0x43a3b0 + param_1 * 4) = local_8;
            D32(0x43a3f4 + param_1 * 4) = local_c;
            uVar2 = uVar4;
        } else {
            // Blocked: pick another direction (not back where it came from
            // for the first 4 tries).
            uVar2 = (u32)(Rand() % 4);
            iVar5 = 0;
            do {
                uVar2 = uVar2 + 1;
                if (4 < (int)uVar2) {
                    uVar2 = 1;
                }
            } while (((uVar4 == uVar2) ||
                      ((iVar5 < 4 && ((s32)uVar2 == D32(0x435a68 + uVar4 * 4))) ||
                       (FUN_0040e9f9(local_8, local_c, (int)uVar2) != 0))) &&
                     (iVar5 = iVar5 + 1, iVar5 < 8));
        }
        DU32(0x43a58c + param_1 * 4) = (DU32(0x43a58c + param_1 * 4) & 0x80) | uVar2;
        if (uVar2 == 1) {
            D32(0x43a58c + param_1 * 4) = 1;
        } else if (uVar2 == 3) {
            D32(0x43a58c + param_1 * 4) = 0x83;
        }
    }
    D32(0x43a438 + param_1 * 4) = local_10;
    D32(0x43a47c + param_1 * 4) = local_14;
    FUN_0040e931(param_1);
}

// 0x40ed72: player death / respawn animations.
void FUN_0040ed72() {
    const void* img;

    if (D32(0x43a6f8) == 2) {
        if (D32(0x43a360) < 0) {
            D32(0x43a360) = D32(0x43a360) + 1;
            FUN_0040e47f(D32(0x43a364), D32(0x43a360));
        } else {
            if (D32(0x43a354) == 0) {
                D32(0x43a6f8) = 0;
                D32(0x43a3a4) = -1;
                FUN_0040d64b();
                return;
            }
            D32(0x43a6f8) = 0xd;
        }
    } else if (D32(0x43a6f8) == 9) {
        if (D32(0x43a360) < 0x1b) {
            D32(0x43a364) = 1;
            D32(0x43a360) = D32(0x43a360) + 1;
            FUN_0040e47f(1, D32(0x43a360));
            Spr_SetZ(D32(0x43a350), 99);
        } else {
            Spr_Delete(D32(0x43a350), 1);
            D32(0x43a350) = 0;
            D32(0x43a6f8) = 10;
            if (-1 < D32(0x43a374)) {
                img = Ang_FrameImage(P(0x43a250), 0);
                if (D32(0x43a378) == 1) {
                    img = Ang_FrameImage(P(0x43a228 + D32(0x43a698 + D32(0x43a374) * 4) * 4), 5);
                }
                Spr_SetZ(D32(0x43a2cc + D32(0x43a374) * 4), 10);
                Spr_SetImage(D32(0x43a2cc + D32(0x43a374) * 4), img);
            }
        }
    }
}

// 0x40eebb: keyboard input (arrows move, Enter/Space picks the tile).
void FUN_0040eebb(u16 param_1) {
    int uVar1;
    int iVar2 = 0;

    if (param_1 < 0x14c) {
        if (param_1 == 0x14b) {
            iVar2 = 1;
        } else if ((param_1 == 0xd) || (param_1 == 0x20)) {
            if (D32(0x43a6f8) == 0) {
                D32(0x43a3a4) = FUN_0040e231();
                if (-1 < D32(0x43a3a4)) {
                    D32(0x43a3a4) = D32(0x43a698 + D32(0x43a3a4) * 4);
                    D32(0x43a6f8) = 4;
                }
            }
        } else if (param_1 == 0x148) {
            iVar2 = 2;
        }
    } else if (param_1 == 0x14d) {
        iVar2 = 3;
    } else if (param_1 == 0x150) {
        iVar2 = 4;
    }
    if (iVar2 != 0) {
        uVar1 = FUN_0040e9f9(D32(0x43a358), D32(0x43a35c), iVar2);
        if (uVar1 == 0) {
            D32(0x43a6f4) = D32(0x43a6f8);
            if (iVar2 == 1) {
                D32(0x43a364) = 4;
            }
            if (iVar2 == 3) {
                D32(0x43a364) = 5;
            }
            D32(0x43a6f8) = 3;
            D32(0x43a700) = iVar2;
            Timer_Set(3, 0x32);
        } else if ((D32(0x43a6f8) == 0xd) && (uVar1 == (int)D16(0x43a70a) + 3)) {
            D32(0x43a6f8) = 0xe;
            Timer_Set(1, 0x32);
            FUN_0040e2f2();
            FUN_00419330(1);
            return;
        }
    }
}

// 0x40efe8: player walking step.
void FUN_0040efe8() {
    const void* pvVar2;

    if (Timer_Expired(3)) {
        Timer_Set(3, 0x32);
        if (D32(0x43a360) < 3) {
            D32(0x43a360) = D32(0x43a360) + 1;
            FUN_0040e47f(D32(0x43a364), D32(0x43a360));
            return;
        }
        D32(0x43a360) = 0;
        if (D32(0x43a700) == 1) {
            D32(0x43a358) = D32(0x43a358) + -1;
        } else if (D32(0x43a700) == 2) {
            D32(0x43a35c) = D32(0x43a35c) + -1;
        } else if (D32(0x43a700) == 3) {
            D32(0x43a358) = D32(0x43a358) + 1;
        } else if (D32(0x43a700) == 4) {
            D32(0x43a35c) = D32(0x43a35c) + 1;
        }
        if (-1 < D32(0x43a374)) {
            pvVar2 = Ang_FrameImage(P(0x43a250), 0);
            if (D32(0x43a378) == 1) {
                pvVar2 = Ang_FrameImage(P(0x43a228 + D32(0x43a698 + D32(0x43a374) * 4) * 4), 5);
            }
            Spr_SetZ(D32(0x43a2cc + D32(0x43a374) * 4), 10);
            Spr_SetImage(D32(0x43a2cc + D32(0x43a374) * 4), pvVar2);
        }
        D32(0x43a6fc) = D32(0x43a6fc) + 1;
        if ((D32(0x43a6fc) == 3) && (D32(0x43a3ac) == 0)) {
            FUN_0040e71d(6, 6);
        }
        FUN_0040e47f(D32(0x43a364), D32(0x43a360));
        D32(0x43a374) = FUN_0040e231();
        if (-1 < D32(0x43a374)) {
            pvVar2 = Ang_FrameImage(P(0x43a228 + D32(0x43a698 + D32(0x43a374) * 4) * 4), 5);
            Spr_SetZ(D32(0x43a2cc + D32(0x43a374) * 4), 0x1e);
            Spr_SetImage(D32(0x43a2cc + D32(0x43a374) * 4), pvVar2);
        }
        D32(0x43a6f8) = D32(0x43a6f4);
    }
}

// 0x40f17c: per-event handler of the activity.
void FUN_0040f17c(u16 param_1) {
    int iVar1;
    int iVar3;

    iVar3 = FUN_0040394c(param_1);
    if (iVar3 != 0) {
        return;
    }
    if (param_1 == 0xc) {
        FUN_00403f95();
    }
    if (Timer_Expired(2)) {
        Timer_Set(2, 100);
        for (iVar3 = 0; iVar1 = D32(0x43a3ac), iVar3 < D32(0x43a3ac); iVar3 = iVar3 + 1) {
            FUN_0040ea51(iVar3);
            if ((((D32(0x43a6f8) < 6) || (D32(0x43a6f8) == 0xd)) && (D32(0x43a6f8) != 9)) &&
                ((D32(0x43a6f8) != 1 && (-1 < D32(0x43a5d0 + iVar3 * 4)))) &&
                ((D32(0x43a3b0 + iVar3 * 4) == D32(0x43a358) &&
                  (D32(0x43a3f4 + iVar3 * 4) == D32(0x43a35c))))) {
                D32(0x43a6f8) = 9;
                Timer_Set(1, 0x32);
                FUN_00402bad(4, 1);
            }
        }
        while (iVar1 = iVar1 + -1, -1 < iVar1) {
            if (D32(0x43a5d0 + iVar1 * 4) < 1) {
                if (D32(0x43a5d0 + iVar1 * 4) < 0) {
                    D32(0x43a5d0 + iVar1 * 4) = D32(0x43a5d0 + iVar1 * 4) + 1;
                    if (D32(0x43a5d0 + iVar1 * 4) == -1) {
                        iVar3 = Rand();
                        D32(0x43a5d0 + iVar1 * 4) = (iVar3 % 5) * 10 + 200;
                    }
                }
            } else {
                D32(0x43a5d0 + iVar1 * 4) = D32(0x43a5d0 + iVar1 * 4) + -1;
                if ((1 < iVar1) && (D32(0x43a5d0 + iVar1 * 4) == 0)) {
                    FUN_0040e0ad(iVar1);
                }
            }
        }
    }
    switch (D32(0x43a6f8)) {
    case 0:
        FUN_0040eebb(param_1);
        break;
    case 1:
        FUN_0040e7c0();
        D32(0x43a6f8) = D32(0x43a6f8) + 1;
        break;
    case 2:
        if (Timer_Expired(1)) {
            FUN_0040ed72();
            Timer_Set(1, 200);
        }
        break;
    case 3:
        FUN_0040efe8();
        break;
    case 4:
        FUN_0040d64b();
        D32(0x43a6f8) = D32(0x43a6f8) + 1;
        break;
    case 5:
        if (D32(0x43a3a8) == D32(0x43a3a4)) {
            FUN_00404068(1);
            FUN_00403e2c(0x19);
            FUN_0040e167();
            D32(0x43a374) = -1;
            D32(0x43a39c) = D32(0x43a39c) + 1;
            D32(0x43a3a4) = -1;
            FUN_0040d64b();
            Timer_Set(3, 2000);
            D32(0x43a6f8) = 0xb;
        } else {
            FUN_00404068(0);
            Timer_Set(1, 0x32);
            D32(0x43a6f8) = 6;
            D32(0x43a6e8) = 5;
        }
        break;
    case 6:
        if (Timer_Expired(1)) {
            FUN_0040e97f(1);  // eax = Timer_Expired result
            Timer_Set(1, 0x32);
        }
        break;
    case 7:
        FUN_0040e71d(D32(0x43a358), D32(0x43a35c));
        Timer_Set(3, 2000);
        D32(0x43a6f8) = 8;
        break;
    case 8:
        FUN_0040eebb(param_1);
        if (Timer_Expired(3)) {
            D32(0x43a6f8) = 0xc;
        }
        break;
    case 9:
        if (Timer_Expired(1)) {
            FUN_0040ed72();
            Timer_Set(1, 0x32);
        }
        break;
    case 10:
        if (D32(0x43a6ec) == 0) {
            D32(0x43a384) = 1;
        } else {
            D32(0x43a6ec) = D32(0x43a6ec) + -1;
            Spr_Delete(D32(0x43a2bc + D32(0x43a6ec) * 4), 1);
            D32(0x43a6f8) = 1;
        }
        break;
    case 0xb:
        FUN_0040eebb(param_1);
        if (Timer_Expired(3)) {
            D32(0x43a6f8) = 0xc;
        }
        break;
    case 0xc:
        FUN_0040eebb(param_1);
        if (D32(0x43a39c) < 0) {
            if ((0xf < D32(0x43a6f0)) || (D32(0x43a614) == 0)) {
                FUN_0040df9c(0, 0);
                D32(0x43a6f8) = 0xd;
                break;
            }
            FUN_0040d4ae(D32(0x43a37c));
        }
        D32(0x43a3a4) = -1;
        FUN_0040d64b();
        D32(0x43a6f8) = 0;
        break;
    case 0xd:
        FUN_0040eebb(param_1);
        FUN_0040e8cf();
        FUN_0040e8ef();
        break;
    case 0xe:
        if (Timer_Expired(1)) {
            Timer_Set(1, 0x32);
            if (D32(0x43a360) < 0x13) {
                FUN_0040e35e(D32(0x43a360) + 1);
            } else {
                if (D32(0x43a350) != 0) {
                    Spr_Delete(D32(0x43a350), 1);
                    D32(0x43a350) = 0;
                    FUN_0040df9c(1, 0);
                }
                if (D32(0x43a36c) < 0xc) {
                    FUN_0040e8ef();
                } else {
                    D32(0x43a384) = 1;
                }
            }
        }
        break;
    }
    Spr_Update(1);
}

// 0x40f5dd
bool FUN_0040f5dd() {
    return 2 < D32(0x43a380);
}

// 0x40f5ef
void FUN_0040f5ef() {
}

// 0x40f5f4
void FUN_0040f5f4() {
    Fx_CloseBlack();
}

// 0x40f5fe: one round of the activity.
void FUN_0040f5fe() {
    FUN_0040cf8c();
    FUN_0040d098();
    Spr_Update(1);
    if (D32(0x43a380) == 0) {
        FUN_0040658a();
    }
    D32(0x43a384) = 0;
    Timer_Set(0, 0x32);
    Timer_Set(1, 200);
    Timer_Set(2, 100);
    Event_Flush();
    do {
        while (!Timer_Expired(0)) {
            int uVar2 = Event_Poll();
            FUN_0040f17c((u16)uVar2);
            Spr_Update(1);
        }
        Timer_Set(0, 0x32);
    } while (D32(0x43a384) == 0);
    FUN_0040f5f4();
    FUN_0040f5ef();
}

// 0x40f697: module entry.
void FUN_0040f697() {
    FUN_0040d225();
    D32(0x43a380) = 0;
    do {
        FUN_0040f5fe();
        D32(0x43a380) = D32(0x43a380) + 1;
    } while (!FUN_0040f5dd());
}

// 0x40f6bc: module cleanup.
void FUN_0040f6bc() {
    FUN_0040f5ef();
    FUN_0040d14f();
    Spr_DeleteAll(0, 0);
    FUN_00405901();
}

// 0x40f6dc: feedback narration (param_1 != 0: fixed line, else rotating).
void FUN_0040f6dc(int param_1) {
    if (param_1 == 0) {
        FUN_00402ec2(D32(0x435a7c) + 0x15b5, 0, 1);
        D32(0x435a7c) = D32(0x435a7c) + 1;
        if (2 < D32(0x435a7c)) {
            D32(0x435a7c) = 0;
        }
    } else {
        FUN_00402ec2(0x15b4, 0, 1);
    }
}
