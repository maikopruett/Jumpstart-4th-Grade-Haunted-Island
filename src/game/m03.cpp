// Module 3 (0x4084c0..0x40a144): spelling maze. The player's sprite walks a
// graph of letter nodes and must collect the letters of a word in order
// while (on harder levels) a chaser walks the same graph.
//
// Node table at 0x434724, DAT_00434ce8 records of 0x16 bytes:
//   +0x00 s32 x            +0x04 s32 y          +0x08 s32 node usable for letters
//   +0x0c char letter (' ' = none)              +0x0d u8 link count
//   +0x0e u8 links[8] (links[0..3] = right, down, left, up for arrow keys)
//
// Resources (pointers, P()): 0x439a9c player frames (0x2be), 0x439aa0 chaser
// frames (0x2bf), 0x439aa4 maze letters (0x2c0), 0x439aa8 word letters
// (0x2c1), 0x439aac highlighted word letters (0x2c2), 0x4346e0 word list,
// 0x4346e4 current word (char* into 0x4346e0), 0x439ab0 rope image
// (Img_Create), 0x439adc heap s32[DAT_00434ce8] of letter sprite ids.
//
// Ints: 0x439ab4 round, 0x439ab8 lives, 0x439abc letter count, 0x439ac0 word
// done, 0x439ac4 caught/wrong, 0x439ac8 word count, 0x439acc word index,
// 0x439ad0 next letter, 0x439ad4 chaser on, 0x439ad8 chaser random,
// 0x439ae0 s32[] word letter sprites, 0x439b10 s32[3] life sprites,
// 0x439b1c current life sprite, 0x439b20/24/28 player prev/cur/next node,
// 0x439b2c player step, 0x439b30 player speed, 0x439b34 player steps,
// 0x439b38 player direction, 0x439b3c/40/44 chaser prev/cur/next node,
// 0x439b48 chaser step, 0x439b4c chaser speed, 0x439b50 chaser steps,
// 0x439b54 chaser direction, 0x4346e8/ec player/chaser sprites,
// 0x4346f0 sound handle, 0x4346f4 s32[12] highlighted word letters.
#include <cmath>

#include "game/common.h"

static inline s32& NodeX(int i) { return D32(0x434724 + i * 0x16); }
static inline s32& NodeY(int i) { return D32(0x434728 + i * 0x16); }
static inline s32& NodeOn(int i) { return D32(0x43472c + i * 0x16); }
static inline u8& NodeCh(int i) { return D8(0x434730 + i * 0x16); }
static inline u8& NodeCount(int i) { return D8(0x434731 + i * 0x16); }
static inline u8& NodeLink(int i, int k) { return D8(0x434732 + k + i * 0x16); }

// Image at byte offset `off` of the offset table of the resource in slot `va`.
static inline void* SubImg(u32 va, int off) {
    u8* res = (u8*)P(va);
    return res + RdS32(res + off);
}
static inline s32* LetterSprites() { return (s32*)P(0x439adc); }
static inline char* Word() { return (char*)P(0x4346e4); }

// 0x4084c0: module entry
void FUN_004084c0() {
    FUN_004086b6();
    D32(0x439ab4) = 0;
    do {
        FUN_004084ed();
        D32(0x439ab4) = D32(0x439ab4) + 1;
    } while (!FUN_004085e2());
    FUN_004040f5(1);
}

// 0x4084ed: one round (one word, up to 3 lives)
void FUN_004084ed() {
    u32 uVar2;

    FUN_004087ef();
    D32(0x439ac0) = 0;
    D32(0x439ab8) = 3;
    FUN_00408b85();
    if (D32(0x439ab4) == 0) {
        FUN_0040658a();
    }
    do {
        FUN_00408c25();
        D32(0x439ac4) = 0;
        Timer_Set(0, 100);
        Event_Flush();
        do {
            while (!Timer_Expired(0)) {
                uVar2 = Event_Poll();
                FUN_0040900d((u16)uVar2);
            }
            Timer_Set(0, 100);
            FUN_00409401();
            FUN_004096c3();
            Spr_Update(1);
            FUN_004099d0();
        } while ((D32(0x439ac0) == 0) && (D32(0x439ac4) == 0));
        if (D32(0x439ac0) == 0) {
            FUN_00409d4b();
            D32(0x439ab8) = D32(0x439ab8) - 1;
        } else {
            FUN_00403e2c(0x15e);
            FUN_00419330(1);
            FUN_00409be7();
        }
    } while ((D32(0x439ac0) == 0) && (0 < D32(0x439ab8)));
    FUN_00404068(D32(0x439ac0));
    FUN_004086a0();
    Fx_CloseBlack();
}

// 0x4085e2
bool FUN_004085e2() {
    return 2 < D32(0x439ab4);
}

// 0x4085f4: module cleanup
void FUN_004085f4() {
    FUN_004086a0();
    Res_Free(P(0x439a9c));
    P(0x439a9c) = nullptr;
    Res_Free(P(0x439aa0));
    P(0x439aa0) = nullptr;
    Res_Free(P(0x439aa4));
    P(0x439aa4) = nullptr;
    Res_Free(P(0x439aa8));
    P(0x439aa8) = nullptr;
    Res_Free(P(0x439aac));
    P(0x439aac) = nullptr;
    Res_Free(P(0x4346e0));
    P(0x4346e0) = nullptr;
    Mem_Free(P(0x439ab0));
    P(0x439ab0) = nullptr;
    Mem_Free(P(0x439adc));
    P(0x439adc) = nullptr;
    FUN_00405901();
}

// 0x4086a0
void FUN_004086a0() {
    Spr_DeleteAll(0, 0);
    Hotspot_Clear();
}

// 0x4086b6: module init
void FUN_004086b6() {
    u32 uVar1;

    FUN_00402c78(0x7f5);
    FUN_00403e91();
    uVar1 = FUN_004040a2();
    FUN_00402b7e(0x804);
    P(0x439a9c) = Res_Load(0x2be);
    P(0x439aa0) = Res_Load(0x2bf);
    P(0x439aa4) = Res_Load(0x2c0);
    P(0x439aa8) = Res_Load(0x2c1);
    P(0x439aac) = Res_Load(0x2c2);
    P(0x4346e0) = Res_Load(uVar1 + 0x2c3);
    D32(0x439ac8) = RdS32(P(0x4346e0));
    P(0x439ab0) = Img_Create(1, g_screenH);
    P(0x439adc) = Mem_Alloc(D32(0x434ce8) << 2);
    uVar1 = FUN_004040cb();
    if (uVar1 == 0) {
        D32(0x439ad4) = 0;
        D32(0x439b30) = 5;
        return;
    }
    if (uVar1 != 1) {
        if (uVar1 != 2) {
            return;
        }
        D32(0x439ad4) = 1;
        D32(0x439ad8) = 0;
        D32(0x439b30) = 5;
        D32(0x439b4c) = 3;
        return;
    }
    D32(0x439ad4) = 1;
    D32(0x439ad8) = 1;
    D32(0x439b30) = 5;
    D32(0x439b4c) = 3;
}

// 0x4087ef
void FUN_004087ef() {
    D32(0x439abc) = 0xf;
    D32(0x439ad0) = 0;
    FUN_0040882a();
    FUN_0040885e();
    FUN_00408946();
    FUN_00408b20();
    memset(DPTR(0x4346f4), 0, 0x30);
}

// 0x40882a: pick a word (13-byte entries after the count)
void FUN_0040882a() {
    D32(0x439acc) = FUN_00403dd5(D32(0x439ac8), (int*)DPTR(0x434cec), 8);
    P(0x4346e4) = (u8*)P(0x4346e0) + D32(0x439acc) * 0xd + 4;
}

// 0x40885e: scatter the word's letters plus random fillers on the nodes
void FUN_0040885e() {
    int iVar1;
    int iVar2;
    int iVar3;
    int iVar4;

    for (iVar3 = 0; iVar3 < D32(0x434ce8); iVar3 = iVar3 + 1) {
        NodeCh(iVar3) = 0x20;
    }
    iVar3 = (int)strlen(Word());
    iVar4 = 0;
    if (0 < iVar3) {
        do {
            do {
                if (D32(0x434ce8) == 0) {
                    iVar1 = 0;
                } else {
                    iVar1 = Rand();
                    iVar1 = iVar1 % D32(0x434ce8);
                }
                if ((NodeOn(iVar1) == 0) || (NodeCh(iVar1) != ' ')) {
                    iVar1 = -1;
                }
            } while (iVar1 == -1);
            NodeCh(iVar1) = Word()[iVar4];
            iVar4 = iVar4 + 1;
        } while (iVar4 < iVar3);
    }
    for (iVar4 = 0; iVar4 < D32(0x439abc) - iVar3; iVar4 = iVar4 + 1) {
        do {
            if (D32(0x434ce8) == 0) {
                iVar1 = 0;
            } else {
                iVar1 = Rand();
                iVar1 = iVar1 % D32(0x434ce8);
            }
            if ((NodeOn(iVar1) == 0) || (NodeCh(iVar1) != ' ')) {
                iVar1 = -1;
            }
        } while (iVar1 == -1);
        iVar2 = Rand();
        NodeCh(iVar1) = (char)(iVar2 % 0x1a) + 'a';
    }
}

// 0x408946: create all sprites
void FUN_00408946() {
    int iVar1;
    int iVar5;
    u32 uVar3;
    u32 uVar4;
    const void* pvVar6;
    Point pt;

    Spr_DeleteAll(0, 0);
    pvVar6 = SubImg(0x439a9c, 0);
    iVar1 = Img_W(pvVar6);
    Pt_Set(&pt, 0, 0);
    iVar5 = 0;
    do {
        D32(0x439b10 + iVar5 * 4) = Spr_Create(pvVar6, pt.x, pt.y, 0xc);
        pt.x = pt.x + iVar1 + 5;
        iVar5 = iVar5 + 1;
    } while (iVar5 < 3);
    pt.x = D32(0x436ee4);
    pt.y = D32(0x436ee8);
    D32(0x4346e8) = Spr_Create(nullptr, D32(0x436ee4), D32(0x436ee8), 10);
    pt.x = D32(0x436ee4);
    pt.y = D32(0x436ee8);
    D32(0x4346ec) = Spr_Create(nullptr, D32(0x436ee4), D32(0x436ee8), 10);
    for (iVar1 = 0; iVar1 < D32(0x434ce8); iVar1 = iVar1 + 1) {
        if (NodeCh(iVar1) == ' ') {
            LetterSprites()[iVar1] = 0;
        } else {
            pvVar6 = SubImg(0x439aa4, -0x184 + (u32)NodeCh(iVar1) * 4);
            pt.x = NodeX(iVar1) - Img_W(pvVar6) / 2;
            pt.y = NodeY(iVar1) - Img_H(pvVar6) / 2;
            LetterSprites()[iVar1] = Spr_Create(pvVar6, pt.x, pt.y, 5);
        }
    }
    pvVar6 = SubImg(0x439aa8, 0x68);
    iVar1 = Img_W(pvVar6);
    uVar3 = FUN_004040a2();
    if (uVar3 != 0) {
        pvVar6 = nullptr;
    }
    iVar5 = (int)strlen(Word());
    Pt_Set(&pt, (int)((u32)(g_screenW - iVar5 * (iVar1 + 5)) >> 1), 0x179);
    uVar3 = 0;
    while (true) {
        uVar4 = (u32)strlen(Word());
        if (uVar4 <= uVar3) break;
        D32(0x439ae0 + uVar3 * 4) = Spr_Create(pvVar6, pt.x, pt.y, 8);
        pt.x = pt.x + iVar1 + 5;
        uVar3 = uVar3 + 1;
    }
}

// 0x408b20: one hotspot per node
void FUN_00408b20() {
    Rect r;

    Hotspot_Clear();
    for (int iVar1 = 0; iVar1 < D32(0x434ce8); iVar1 = iVar1 + 1) {
        Rect_Set(&r, NodeX(iVar1) - 0x12, NodeY(iVar1) - 0x12, 0x24, 0x24);
        Hotspot_Set((s16)iVar1 + 0x300, r);
    }
}

// 0x408b85: background
void FUN_00408b85() {
    void* img = Res_Load(0x2bd);
    Img_Draw(D32(0x436ee4), D32(0x436ee8), img, 0);
    Res_Free(img);
    Gfx_SaveBg(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    FUN_004032e3();
    Spr_Update(0);
    if (D32(0x439ab4) == 0) {
        Screen_FlushAll();
        FUN_00408bff();
        return;
    }
    Fx_OpenCenter();
}

// 0x408bff
void FUN_00408bff() {
    void* rgb = Res_Load(700);
    Pal_FadeTo((const u8*)rgb, 1);
    Res_Free(rgb);
}

// 0x408c25: start a life
void FUN_00408c25() {
    FUN_00408c44();
    FUN_00408f05();
    Spr_SetImage(D32(0x439b1c), nullptr);
}

// 0x408c44: the life sprite walks to the start node and lowers on a rope
void FUN_00408c44() {
    int iVar4;
    int iVar6;
    const void* img;
    Rect line;
    Rect full;
    Point ropePos;
    Point target;
    Point pos;

    D32(0x439b38) = 4;
    D32(0x439b2c) = 0;
    img = SubImg(0x439a9c, 0);
    iVar6 = Img_H(img) / 2;
    iVar4 = D32(0x434c7c) - iVar6;
    Pt_Set(&target, D32(0x434c78) - Img_W(img) / 2, iVar4);
    D32(0x439b1c) = D32(0x439b0c + D32(0x439ab8) * 4);
    Spr_GetPos(D32(0x439b1c), &pos);
    Timer_Set(0, 100);
    while (pos.x < target.x) {
        while (!Timer_Expired(0)) {
            FUN_0040394c((u16)Event_Poll());
        }
        Timer_Set(0, 100);
        pos.x = pos.x + 10;
        Spr_SetPos(D32(0x439b1c), pos.x, pos.y);
        Spr_SetImage(D32(0x439b1c), SubImg(0x439a9c, (D32(0x439b38) * 2 + D32(0x439b2c) % 2) * 4));
        D32(0x439b2c) = D32(0x439b2c) + 1;
        if (1 < D32(0x439b2c)) {
            D32(0x439b2c) = 0;
        }
        Spr_Update(1);
    }
    pos.x = target.x;
    Spr_SetImage(D32(0x439b1c), SubImg(0x439a9c, 0));
    Img_FullRect(P(0x439ab0), &full);
    Gfx_SetTargetImg(P(0x439ab0));
    Gfx_FillRect(full.x, full.y, full.w, full.h, Img_Key(P(0x439ab0)));
    Gfx_ResetTarget();
    Pt_Set(&ropePos, D32(0x434c78), 0);
    iVar4 = Spr_Create(P(0x439ab0), ropePos.x, ropePos.y, 9);
    FUN_00402bad(0, 1);
    Timer_Set(0, 0x42);
    while (pos.y < target.y) {
        while (!Timer_Expired(0)) {
            FUN_0040394c((u16)Event_Poll());
        }
        Timer_Set(0, 0x42);
        pos.y = pos.y + 10;
        Spr_SetPos(D32(0x439b1c), pos.x, pos.y);
        Rect_Set(&line, 0, 0, 0, pos.y + iVar6);
        Gfx_SetTargetImg(P(0x439ab0));
        Gfx_Line(line.x, line.y, line.w, line.h, 0x33);
        Gfx_ResetTarget();
        Spr_Update(1);
    }
    Spr_Delete(iVar4, 1);
    D32(0x439b24) = 0x3e;
    D32(0x439b28) = 0x34;
    D32(0x439b2c) = 0;
    D32(0x439b40) = 0x37;
    D32(0x439b44) = 0x19;
    D32(0x439b48) = 0;
}

// 0x408f05: say the word
void FUN_00408f05() {
    int iVar1;
    u32 uVar2;
    int local_10[3];

    local_10[0] = D32(0x434d0c);
    local_10[1] = D32(0x434d10);
    local_10[2] = D32(0x434d14);
    uVar2 = FUN_004040a2();
    iVar1 = local_10[uVar2];
    FUN_0040a003(D32(0x439acc) * 2 + iVar1, 1, 1);
    if (D32(0x439ab8) == 3) {
        FUN_0040a003(D32(0x439acc) * 2 + iVar1 + 1, 1, 1);
        FUN_0040a003(iVar1 + D32(0x439acc) * 2, 1, 1);
    }
}

// 0x408f83: player is less than 3 steps from the end of its edge
bool FUN_00408f83() {
    return D32(0x439b34) - D32(0x439b2c) < 3;
}

// 0x408f9c: index of node param_1 in the links of node param_2
int FUN_00408f9c(u32 param_1, int param_2) {
    int iVar1;
    int iVar2 = 0;
    bool bVar3 = false;

    while (!bVar3) {
        iVar1 = iVar2 + param_2 * 0x16;
        iVar2 = iVar2 + 1;
        bVar3 = param_1 == D8(0x434732 + iVar1);
    }
    return iVar2 - 1;
}

// 0x40900d: input
void FUN_0040900d(u16 param_1) {
    bool bVar1;
    int iVar2;
    u32 uVar3;
    Point dir;

    iVar2 = FUN_0040394c(param_1);
    if (iVar2 == 0) {
        Pt_Set(&dir, -1, 0);
        if (D32(0x439b24) < 0x35) {
            if (param_1 < 0x14c) {
                if (param_1 == 0x14b) {
                    FUN_00402bad(1, 1);
                    if (D32(0x4346dc) != 0) {
                        D32(0x439b28) = NodeLink(D32(0x439b24), 2);
                        return;
                    }
                    bVar1 = FUN_00408f83();
                    if ((!bVar1) && ((u32)NodeLink(D32(0x439b20), 0) == DU32(0x439b24))) {
                        D32(0x439b28) = D32(0x439b20);
                        return;
                    }
                    D32(0x439b28) = NodeLink(D32(0x439b24), 2);
                    return;
                }
                if (param_1 == 0xc) {
                    FUN_00403f95();
                    return;
                }
                if (param_1 == 0x148) {
                    FUN_00402bad(1, 1);
                    if (D32(0x4346dc) != 0) {
                        D32(0x439b28) = NodeLink(D32(0x439b24), 3);
                        return;
                    }
                    bVar1 = FUN_00408f83();
                    if ((!bVar1) && ((u32)NodeLink(D32(0x439b20), 1) == DU32(0x439b24))) {
                        D32(0x439b28) = D32(0x439b20);
                        return;
                    }
                    D32(0x439b28) = NodeLink(D32(0x439b24), 3);
                    return;
                }
            } else {
                if (param_1 == 0x14d) {
                    FUN_00402bad(1, 1);
                    if (D32(0x4346dc) != 0) {
                        D32(0x439b28) = NodeLink(D32(0x439b24), 0);
                        return;
                    }
                    bVar1 = FUN_00408f83();
                    if ((!bVar1) && ((u32)NodeLink(D32(0x439b20), 2) == DU32(0x439b24))) {
                        D32(0x439b28) = D32(0x439b20);
                        return;
                    }
                    D32(0x439b28) = NodeLink(D32(0x439b24), 0);
                    return;
                }
                if (param_1 == 0x150) {
                    FUN_00402bad(1, 1);
                    if (D32(0x4346dc) != 0) {
                        D32(0x439b28) = NodeLink(D32(0x439b24), 1);
                        return;
                    }
                    bVar1 = FUN_00408f83();
                    if ((!bVar1) && ((u32)NodeLink(D32(0x439b20), 3) == DU32(0x439b24))) {
                        D32(0x439b28) = D32(0x439b20);
                        return;
                    }
                    D32(0x439b28) = NodeLink(D32(0x439b24), 1);
                    return;
                }
            }
            uVar3 = (u32)param_1 - 0x300;
            if (((-1 < (int)uVar3) && ((int)uVar3 < D32(0x434ce8))) && (uVar3 != DU32(0x439b24))) {
                bVar1 = false;
                for (iVar2 = 0; iVar2 < (int)(u32)NodeCount(D32(0x439b24)); iVar2 = iVar2 + 1) {
                    if (uVar3 == NodeLink(D32(0x439b24), iVar2)) {
                        bVar1 = true;
                        break;
                    }
                }
                if (bVar1) {
                    FUN_00402bad(1, 1);
                    D32(0x439b28) = uVar3;
                }
            }
        } else {
            D32(0x439b28) = FUN_00409302(D32(0x439b24), dir.x, dir.y);
        }
    }
}

// 0x409302: link of node param_1 whose direction best matches (param_2, param_3)
u32 FUN_00409302(int param_1, int param_2, int param_3) {
    u8 bVar1;
    u32 local_24 = 0xffffffff;
    float local_20 = -2.0f;
    float local_1c, local_18, local_14, local_10;
    int local_c, local_8;

    for (int iVar3 = 0; iVar3 < (int)(u32)NodeCount(param_1); iVar3 = iVar3 + 1) {
        bVar1 = NodeLink(param_1, iVar3);
        local_c = NodeX(bVar1) - NodeX(param_1);
        local_8 = NodeY(bVar1) - NodeY(param_1);
        local_10 = (float)(local_c * param_2 + local_8 * param_3);
        local_14 = (float)(local_c * local_c + local_8 * local_8);
        local_18 = (float)(param_2 * param_2 + param_3 * param_3);
        local_1c = (float)((double)local_10 / sqrt((double)local_14 * (double)local_18));
        // fcomp/jae: also taken when unordered (NaN from 0/0)
        if (!(local_20 >= local_1c)) {
            local_24 = (u32)bVar1;
            local_20 = local_1c;
        }
    }
    return local_24;
}

// 0x409401: move the player one step
void FUN_00409401() {
    u8 bVar1;
    int iVar2, iVar5, iVar6, iVar7, iVar8, iVar9;
    u32 uVar4 = DU32(0x439b28);
    u32 uVar10 = DU32(0x439b24);
    const void* img;

    if (D32(0x439b2c) == 0) {
        if (D32(0x439b28) == -1) {
            bVar1 = NodeCount(D32(0x439b24));
            if (bVar1 < 5) {
                iVar8 = FUN_00408f9c(DU32(0x439b20), D32(0x439b24));
                D32(0x439b28) =
                    NodeLink(D32(0x439b24), (int)(iVar8 + (u32)bVar1 - 2) % (int)(u32)bVar1);
            } else {
                D32(0x439b28) = FUN_00409302(D32(0x439b24), NodeX(D32(0x439b24)) - NodeX(D32(0x439b20)),
                                             NodeY(D32(0x439b24)) - NodeY(D32(0x439b20)));
            }
        }
        uVar4 = DU32(0x439b28);
        uVar10 = DU32(0x439b24);
        D32(0x439b20) = D32(0x439b24);
        D32(0x439b24) = D32(0x439b28);
        D32(0x439b28) = -1;
        iVar8 = NodeX(uVar4) - NodeX(uVar10);
        iVar9 = NodeY(uVar4) - NodeY(uVar10);
        D32(0x439b38) = FUN_00409912(iVar8, iVar9);
        D32(0x439b34) = (int)(sqrt((double)(iVar8 * iVar8 + iVar9 * iVar9)) / (double)D32(0x439b30)) + 1;
    } else if (DU32(0x439b28) == DU32(0x439b20)) {
        D32(0x439b20) = D32(0x439b24);
        D32(0x439b24) = D32(0x439b28);
        D32(0x439b28) = -1;
        D32(0x439b38) = FUN_00409912(NodeX(uVar4) - NodeX(uVar10), NodeY(uVar4) - NodeY(uVar10));
        D32(0x439b2c) = D32(0x439b34) - D32(0x439b2c);
    }
    D32(0x439b2c) = D32(0x439b2c) + 1;
    img = SubImg(0x439a9c, (D32(0x439b38) * 2 + D32(0x439b2c) % 2) * 4);
    iVar8 = NodeX(D32(0x439b24));
    iVar9 = NodeX(D32(0x439b20));
    iVar2 = NodeY(D32(0x439b24));
    iVar7 = NodeY(D32(0x439b20));
    iVar5 = Img_W(img) / 2;
    iVar9 = (D32(0x439b2c) * (iVar8 - iVar9)) / D32(0x439b34);
    iVar8 = NodeX(D32(0x439b20));
    iVar6 = Img_H(img) / 2;
    iVar7 = (D32(0x439b2c) * (iVar2 - iVar7)) / D32(0x439b34);
    iVar2 = NodeY(D32(0x439b20));
    Spr_SetImage(D32(0x4346e8), img);
    Spr_SetPos(D32(0x4346e8), (iVar9 + iVar8) - iVar5, (iVar7 + iVar2) - iVar6);
    if (D32(0x439b34) <= D32(0x439b2c)) {
        D32(0x439b2c) = 0;
    }
}

// 0x4096c3: move the chaser one step
void FUN_004096c3() {
    int iVar1, iVar3, iVar4, iVar5, iVar6, iVar7;
    u32 uVar8;
    const void* img;
    int local_c, local_8;

    if (D32(0x439ad4) != 0) {
        if (D32(0x439b48) == 0) {
            if (D32(0x439ad8) == 0) {
                if (D32(0x439b24) == D32(0x439b40)) {
                    local_c = NodeX(D32(0x439b20)) - NodeX(D32(0x439b40));
                    local_8 = NodeY(D32(0x439b20)) - NodeY(D32(0x439b40));
                } else {
                    local_c = NodeX(D32(0x439b24)) - NodeX(D32(0x439b40));
                    local_8 = NodeY(D32(0x439b24)) - NodeY(D32(0x439b40));
                }
                D32(0x439b44) = FUN_00409302(D32(0x439b40), local_c, local_8);
            } else {
                if (NodeCount(D32(0x439b40)) == 0) {
                    iVar3 = 0;
                } else {
                    iVar3 = Rand();
                    iVar3 = iVar3 % (int)(u32)NodeCount(D32(0x439b40));
                }
                D32(0x439b44) = NodeLink(D32(0x439b40), iVar3);
            }
            uVar8 = DU32(0x439b40);
            D32(0x439b3c) = D32(0x439b40);
            D32(0x439b40) = D32(0x439b44);
            iVar3 = NodeX(D32(0x439b44)) - NodeX(uVar8);
            iVar4 = NodeY(D32(0x439b44)) - NodeY(uVar8);
            D32(0x439b54) = FUN_00409912(iVar3, iVar4);
            D32(0x439b50) = (int)(sqrt((double)(iVar3 * iVar3 + iVar4 * iVar4)) / (double)D32(0x439b4c)) + 1;
        }
        D32(0x439b48) = D32(0x439b48) + 1;
        img = SubImg(0x439aa0, (D32(0x439b54) * 2 + D32(0x439b48) % 2) * 4);
        iVar3 = NodeX(D32(0x439b40));
        iVar4 = NodeX(D32(0x439b3c));
        iVar1 = NodeY(D32(0x439b40));
        iVar7 = NodeY(D32(0x439b3c));
        iVar5 = Img_W(img) / 2;
        iVar4 = (D32(0x439b48) * (iVar3 - iVar4)) / D32(0x439b50);
        iVar3 = NodeX(D32(0x439b3c));
        iVar6 = Img_H(img) / 2;
        iVar7 = (D32(0x439b48) * (iVar1 - iVar7)) / D32(0x439b50);
        iVar1 = NodeY(D32(0x439b3c));
        Spr_SetImage(D32(0x4346ec), img);
        Spr_SetPos(D32(0x4346ec), (iVar4 + iVar3) - iVar5, (iVar7 + iVar1) - iVar6);
        if (D32(0x439b50) <= D32(0x439b48)) {
            D32(0x439b48) = 0;
        }
    }
}

// 0x409912: direction (0..15) of the vector (param_1, param_2)
int FUN_00409912(int param_1, int param_2) {
    int iVar1;
    int iVar2;
    float local_18[4];

    memcpy(local_18, DPTR(0x434d18), sizeof(local_18));
    if (param_1 < 0) {
        if (param_2 < 0) {
            iVar2 = 3;
        } else {
            iVar2 = 2;
        }
    } else if (param_2 < 1) {
        iVar2 = 0;
    } else {
        iVar2 = 1;
    }
    if (param_2 == 0) {
        if (iVar2 == 0) {
            iVar1 = 0;
        } else {
            iVar1 = 8;
        }
    } else {
        float f = (float)fabs((double)param_1 / (double)param_2);
        iVar1 = 0;
        do {
            if (!(f > local_18[iVar1])) break; // jbe (also on unordered)
            iVar1 = iVar1 + 1;
        } while (iVar1 < 4);
        if (iVar2 != 0) {
            if (iVar2 == 1) {
                iVar1 = 8 - iVar1;
            } else if (iVar2 == 2) {
                iVar1 = iVar1 + 8;
            } else if (iVar2 == 3) {
                if (iVar1 == 0) {
                    iVar1 = 0;
                } else {
                    iVar1 = 0x10 - iVar1;
                }
            } else {
                iVar1 = 0;
            }
        }
    }
    return iVar1;
}

// 0x4099d0: letter pickup and chaser collision
void FUN_004099d0() {
    u32 uVar1;
    Rect r1;
    Rect r2;
    Point pos;
    const void* local_8;

    if (D32(0x439b2c) == 0) {
        if (Word()[D32(0x439ad0)] == (char)NodeCh(D32(0x439b24))) {
            D32(0x4346f0) = FUN_00402bad(2, 4);
            NodeCh(D32(0x439b24)) = 0x20;
            Spr_SetImage(LetterSprites()[D32(0x439b24)], nullptr);
            local_8 = SubImg(0x439aa8, -0x184 + (u32)(u8)Word()[D32(0x439ad0)] * 4);
            Spr_SetImage(D32(0x439ae0 + D32(0x439ad0) * 4), local_8);
            uVar1 = (u32)strlen(Word());
            D32(0x439ad0) = D32(0x439ad0) + 1;
            if (uVar1 <= DU32(0x439ad0)) {
                D32(0x439ac0) = 1;
            }
        } else if (NodeCh(D32(0x439b24)) != ' ') {
            FUN_00402bad(3, 4);
            D32(0x439ac4) = 1;
        }
    }
    if (D32(0x439ad4) != 0) {
        Spr_GetPos(D32(0x4346e8), &pos);
        Spr_GetImage(D32(0x4346e8), &local_8);
        pos.x = pos.x + Img_W(local_8) / 2;
        pos.y = pos.y + Img_H(local_8) / 2;
        Rect_Set(&r1, pos.x - 10, pos.y - 10, 0x14, 0x14);
        Spr_GetPos(D32(0x4346ec), &pos);
        Spr_GetImage(D32(0x4346ec), &local_8);
        pos.x = pos.x + Img_W(local_8) / 2;
        pos.y = pos.y + Img_H(local_8) / 2;
        Rect_Set(&r2, pos.x - 0xf, pos.y - 0xf, 0x1e, 0x1e);
        if (Rect_Intersect(r1.x, r1.y, r1.w, r1.h, r2.x, r2.y, r2.w, r2.h, nullptr)) {
            FUN_00402bad(4, 4);
            D32(0x439ac4) = 1;
        }
    }
}

// 0x409be7: word complete: spell it out
void FUN_00409be7() {
    u32 uVar2;
    u32 uVar3;
    int local_10[3];

    local_10[0] = D32(0x434d28);
    local_10[1] = D32(0x434d2c);
    local_10[2] = D32(0x434d30);
    while (Sound_IsPlaying(D32(0x4346f0))) {
        uVar3 = Event_Poll();
        FUN_0040394c((u16)uVar3);
    }
    uVar3 = 0;
    while (true) {
        uVar2 = (u32)strlen(Word());
        if (uVar2 <= uVar3) break;
        Spr_SetImage(D32(0x439ae0 + uVar3 * 4), SubImg(0x439aac, -0x184 + (u32)(u8)Word()[uVar3] * 4));
        Spr_Update(1);
        D32(0x4346f4 + uVar3 * 4) = 1;
        FUN_0040a003((u8)Word()[uVar3] + 0xa48, 1, 0);
        Spr_SetImage(D32(0x439ae0 + uVar3 * 4), SubImg(0x439aa8, -0x184 + (u32)(u8)Word()[uVar3] * 4));
        Spr_Update(1);
        D32(0x4346f4 + uVar3 * 4) = 0;
        uVar3 = uVar3 + 1;
    }
    uVar3 = 0;
    while (true) {
        uVar2 = (u32)strlen(Word());
        if (uVar2 <= uVar3) break;
        Spr_SetImage(D32(0x439ae0 + uVar3 * 4), SubImg(0x439aac, -0x184 + (u32)(u8)Word()[uVar3] * 4));
        D32(0x4346f4 + uVar3 * 4) = 1;
        uVar3 = uVar3 + 1;
    }
    Spr_Update(1);
    uVar3 = FUN_004040a2();
    FUN_0040a003(local_10[uVar3] + D32(0x439acc) * 2, 1, 0);
}

// 0x409d4b: life lost: spin and fall off the screen
void FUN_00409d4b() {
    int iVar3;
    Point pos;

    Timer_Set(0, 0x32);
    iVar3 = 0;
    do {
        while (!Timer_Expired(0)) {
            FUN_0040394c((u16)Event_Poll());
        }
        Timer_Set(0, 0x32);
        D32(0x439b38) = D32(0x439b38) + 1;
        if (0xf < D32(0x439b38)) {
            D32(0x439b38) = 0;
        }
        Spr_SetImage(D32(0x4346e8), SubImg(0x439a9c, D32(0x439b38) * 8));
        Spr_Update(1);
        iVar3 = iVar3 + 1;
    } while (iVar3 < 0x10);
    Timer_Set(0, 0x32);
    Spr_GetPos(D32(0x4346e8), &pos);
    while (pos.y < 0x1a9) {
        while (!Timer_Expired(0)) {
            FUN_0040394c((u16)Event_Poll());
        }
        Timer_Set(0, 0x32);
        pos.y = pos.y + 10;
        Spr_SetPos(D32(0x4346e8), pos.x, pos.y);
        Spr_Update(1);
    }
    Spr_SetImage(D32(0x4346e8), nullptr);
    Spr_SetImage(D32(0x4346ec), nullptr);
    Spr_Update(1);
}

// 0x409e5c: Ms. Grunkle's intro narration for this module
int FUN_00409e5c(int param_1) {
    int iVar1;
    int uVar2 = 0;

    if (param_1 == 0) {
        if (D32(0x434d34) < 2) {
            uVar2 = FUN_00402ec2(D32(0x434d34) + 0x8e0, 0, 1);
        } else {
            iVar1 = FUN_00402ec2(D32(0x434d34) + 0x8e0, 0, 1);
            if (iVar1 != 0) {
                return 1;
            }
            uVar2 = FUN_0040a003(D32(0x434d34) + 0x8e1, 0, 1);
        }
        D32(0x434d34) = D32(0x434d34) + 1;
        if (2 < D32(0x434d34)) {
            D32(0x434d34) = 0;
        }
    } else {
        iVar1 = FUN_00402ec2(0x8dc, 0, 1);
        if (iVar1 != 0) {
            return 1;
        }
        iVar1 = FUN_0040a003(0x8dd, 0, 1);
        if (iVar1 != 0) {
            return 1;
        }
        iVar1 = FUN_00402ec2(0x8de, 0, 1);
        if (iVar1 != 0) {
            return 1;
        }
        iVar1 = FUN_0040a003(0x8df, 0, 1);
        if (iVar1 != 0) {
            return 1;
        }
    }
    return uVar2;
}

// 0x40a003: play a sound resource while blinking the maze letters
// (param_3 != 0) or the highlighted word letters; 1 if aborted (event 0x20)
int FUN_0040a003(u32 param_1, int param_2, int param_3) {
    int iVar2;
    u32 uVar3;
    u32 uVar4;
    int iVar5;
    int iVar6 = 0;

    iVar2 = FUN_00402d42(param_1, nullptr, 0);
    while (true) {
        if (!Sound_IsPlaying(iVar2)) {
            return 0;
        }
        uVar3 = Event_Poll();
        if (param_2 != 0) {
            FUN_0040394c((u16)uVar3);
        }
        if ((u16)uVar3 == 0x20) break;
        if (Timer_Expired(10)) {
            if (param_3 == 0) {
                for (uVar3 = 0; uVar4 = (u32)strlen(Word()), uVar3 < uVar4; uVar3 = uVar3 + 1) {
                    if (D32(0x4346f4 + uVar3 * 4) != 0) {
                        Spr_SetImage(D32(0x439ae0 + uVar3 * 4),
                                     SubImg(0x439aac, -0x184 + ((u32)(u8)Word()[uVar3] + iVar6 * 0x1a) * 4));
                    }
                }
            } else {
                for (iVar5 = 0; iVar5 < D32(0x434ce8); iVar5 = iVar5 + 1) {
                    if (NodeCh(iVar5) == ' ') {
                        LetterSprites()[iVar5] = 0;
                    } else {
                        Spr_SetImage(LetterSprites()[iVar5],
                                     SubImg(0x439aa4, -0x184 + ((u32)NodeCh(iVar5) + iVar6 * 0x1a) * 4));
                    }
                }
            }
            iVar6 = iVar6 + 1;
            if (1 < iVar6) {
                iVar6 = 0;
            }
            Timer_Set(10, 100);
        }
        Spr_Update(1);
    }
    FUN_00402d81();
    return 1;
}
