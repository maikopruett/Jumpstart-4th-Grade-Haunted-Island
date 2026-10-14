// Module 14 (0x419a56..0x41a6e8): the kid movie sequences
// (Movies\H\%s.smk, picture ids 0x133.., palettes 300+kid).
//
// Globals used (all ints unless noted):
//   0x43b460  u8[5]   per-kid progress (number of movie steps already seen)
//   0x43b468  s32[5]  per-kid available steps (from DAT_004395f4)
//   0x43b47c  s32[5]  per-kid "complete" flag
//   0x43b490          reset to 0 on entry
//   0x43b494          current kid index (0..4)
//   0x43b498          inside (1) / outside (0) view
//   0x4365d4..0x4365e0 hotspot area x, y, w, h
//   0x4365e4/e8       movie position
//   0x4365ec  s32[]   location (0x432154) per kid
//   0x4365fc  ptr     talking animation resource
//   0x436600/04       sprite ids, 0x436608/0c sprite position
//   0x436610, 0x436614 (sound handle), 0x43661c (narration rotation)
#include "game/common.h"

REGISTER_CODE(0x41a36f, FUN_0041a36f);

// 0x419a56
void FUN_00419a56() {
    int iVar2;
    int iVar3;
    u16 uVar4;
    char local_108[260];

    FUN_00419fb1();
    FUN_0041a140();
    FUN_0041a1f7(D32(0x43b494));
    FUN_0040658a();
    do {
        if ((D32(0x43b494) < 1) && (D32(0x43b498) == 0)) {
            D32(0x4318a0) = 0;
            return;
        }
        iVar3 = 0;
        while (iVar3 == 0) {
            while (true) {
                FUN_0041a376();
                uVar4 = (u16)Event_Poll();
                iVar2 = FUN_0040394c(uVar4);
                if (iVar2 != 0) break;
                if (uVar4 == 0x148) {
                    if (D32(0x43b498) == 0) {
                        iVar3 = 3;
                    } else if ((D32(0x43b47c + D32(0x43b494) * 4) == 0) ||
                               ((3 < D32(0x43b494)) && (DU32(0x43b43c) < 0xd))) {
                        iVar3 = 5;
                    } else if (D32(0x43980c) == 0) {
                        iVar3 = 4;
                    }
                } else if (uVar4 == 0x14b) {
                    iVar3 = 1;
                } else if (uVar4 == 0x14d) {
                    iVar3 = 2;
                }
                if (iVar3 != 0) goto LAB_00419b06;
            }
        }
    LAB_00419b06:
        switch (iVar3) {
        case 1:
            D32(0x436614) = FUN_00402bd1(0, 1);
            FUN_0041a221(local_108, 'L', '0');
            Video_PlayDoubled(local_108, D32(0x4365e4), D32(0x4365e8), FUN_0041a36f);
            Sound_Stop(D32(0x436614));
            D32(0x43b498) = (D32(0x43b498) == 0);
            D32(0x436610) = 0;
            break;
        case 2:
            D32(0x436614) = FUN_00402bd1(1, 1);
            FUN_0041a221(local_108, 'R', '0');
            Video_PlayDoubled(local_108, D32(0x4365e4), D32(0x4365e8), FUN_0041a36f);
            Sound_Stop(D32(0x436614));
            D32(0x43b498) = (D32(0x43b498) == 0);
            D32(0x436610) = 0;
            break;
        case 3:
            D32(0x436614) = FUN_00402bd1(2, 1);
            FUN_0041a221(local_108, 'F', '0');
            Video_PlayDoubled(local_108, D32(0x4365e4), D32(0x4365e8), FUN_0041a36f);
            FUN_0041a1f7(D32(0x43b494) - 1);
            FUN_00402bad(3, 2);
            FUN_0041a221(local_108, 'F', '1');
            Video_PlayDoubled(local_108, D32(0x4365e4), D32(0x4365e8), FUN_0041a36f);
            Sound_Stop(D32(0x436614));
            D32(0x43b494) = D32(0x43b494) - 1;
            break;
        case 4:
            FUN_0041a221(local_108, 'F', '0');
            Video_PlayDoubled(local_108, D32(0x4365e4), D32(0x4365e8), FUN_0041a36f);
            FUN_0041a1f7(D32(0x43b494) + 1);
            if (D32(0x43b494) == 4) {
                Spr_DeleteAll(0, 0);
                FUN_00402d42(D32(0x43b494) + 0x8cc, nullptr, 0);
                FUN_0041a221(local_108, 'F', 'G');
                Video_Play(local_108, D32(0x4365e4), D32(0x4365e8), FUN_0041a36f);
                FUN_00402d81();
                D32(0x43980c) = 1;
                D32(0x432154) = 0x4d;
                FUN_00402610();
            } else {
                FUN_00402bad(3, 2);
                D32(0x436614) = FUN_00402bd1(2, 1);
                FUN_0041a221(local_108, 'F', '1');
                Video_PlayDoubled(local_108, D32(0x4365e4), D32(0x4365e8), FUN_0041a36f);
                Sound_Stop(D32(0x436614));
                D32(0x43b494) = D32(0x43b494) + 1;
            }
            break;
        case 5:
            FUN_0041a2b6();
            D8(0x43b460 + D32(0x43b494)) = D8(0x43b468 + D32(0x43b494) * 4);
            if (4 < D32(0x43b468 + D32(0x43b494) * 4)) {
                D32(0x43b47c + D32(0x43b494) * 4) = 1;
                if (D32(0x43b494) + 1 < 5) {
                    FUN_0041a221(local_108, 'F', '0');
                    Video_PlayDoubled(local_108, D32(0x4365e4), D32(0x4365e8), FUN_0041a36f);
                    FUN_0041a1f7(D32(0x43b494) + 1);
                    FUN_00402d42(D32(0x43b494) + 0x8cc, nullptr, 0);
                    FUN_0041a221(local_108, 'F', 'G');
                    Video_Play(local_108, D32(0x4365e4), D32(0x4365e8), FUN_0041a36f);
                    FUN_00402d81();
                    D32(0x432154) = D32(0x4365ec + D32(0x43b494) * 4);
                    FUN_00402610();
                } else if (DU32(0x43b43c) < 0xd) {
                    D32(0x436610) = 1;
                    FUN_0041a140();
                    FUN_0040658a();
                } else {
                    Spr_DeleteAll(0, 0);
                    FUN_0041a221(local_108, 'F', '0');
                    Video_PlayDoubled(local_108, D32(0x4365e4), D32(0x4365e8), FUN_0041a36f);
                    FUN_00402d42(D32(0x43b494) + 0x8cc, nullptr, 0);
                    FUN_0041a1f7(D32(0x43b494) + 1);
                    FUN_0041a221(local_108, 'F', 'G');
                    Video_Play(local_108, D32(0x436ee4), D32(0x436ee8), FUN_0041a36f);
                    FUN_00402d81();
                    D32(0x43980c) = 1;
                    D32(0x432154) = 0x4d;
                    FUN_00402610();
                }
            }
            break;
        }
        if ((0 < D32(0x43b494)) || (D32(0x43b498) != 0)) {
            FUN_0041a140();
        }
    } while (true);
}

// 0x419fb1
void FUN_00419fb1() {
    FUN_00402b7e(0x80e);
    D32(0x432154) = 0x4d;
    D32(0x43b494) = 0;
    D32(0x43b498) = 1;
    FUN_00419fe6();
    FUN_0041a0c9();
}

// 0x419fe6: three hotspots (left quarter, middle half, right quarter)
void FUN_00419fe6() {
    Rect r;
    int w = D32(0x4365dc);

    Hotspot_Clear();
    Rect_Set(&r, w / 4 + D32(0x4365d4), D32(0x4365d8), w / 2, D32(0x4365e0));
    Hotspot_Set(0x148, r);
    Rect_Set(&r, D32(0x4365d4), D32(0x4365d8), w / 4, D32(0x4365e0));
    Hotspot_Set(0x14b, r);
    Rect_Set(&r, (w * 3) / 4 + D32(0x4365d4), D32(0x4365d8), w / 4, D32(0x4365e0));
    Hotspot_Set(0x14d, r);
}

// 0x41a0c9
void FUN_0041a0c9() {
    u32 uVar1 = DU32(0x4395f4) / 5;
    u32 uVar2 = 0;
    do {
        if ((int)uVar2 < (int)uVar1) {
            D32(0x43b468 + uVar2 * 4) = 5;
        } else if (uVar1 == uVar2) {
            D32(0x43b468 + uVar2 * 4) = DU32(0x4395f4) % 5;
        } else {
            D32(0x43b468 + uVar2 * 4) = 0;
        }
        D32(0x43b47c + uVar2 * 4) = (4 < D8(0x43b460 + uVar2));
        uVar2 = uVar2 + 1;
    } while ((int)uVar2 < 5);
    D32(0x43b490) = 0;
}

// 0x41a136
void FUN_0041a136() {
    Hotspot_Clear();
}

// 0x41a140: draw the current kid picture
void FUN_0041a140() {
    u32 id;
    void* img;

    Gfx_FillRect(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h, 0x24);
    if (D32(0x43b498) == 0) {
        id = D32(0x43b494) * 7 + 0x133;
    } else {
        id = D32(0x43b494) * 7 + (u32)D8(0x43b460 + D32(0x43b494)) + 0x134;
    }
    img = Res_Load(id);
    Img_Draw(D32(0x4365e4), D32(0x4365e8), img, 0);
    Res_Free(img);
    Gfx_SaveBg(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    FUN_004032e3();
    Screen_FlushAll();
}

// 0x41a1f7
void FUN_0041a1f7(int param_1) {
    void* rgb = Res_Load(param_1 + 300);
    Pal_Set((const u8*)rgb, 1);
    Res_Free(rgb);
}

// 0x41a221: builds "Movies\H\<kid><I|O><kind><X|U|L><sub>.smk"
void FUN_0041a221(char* buf, char kind, char sub) {
    char name[6];

    name[0] = (char)D32(0x43b494) + '0';
    name[1] = 'I';
    if (D32(0x43b498) == 0) {
        name[1] = 'O';
    }
    name[2] = kind;
    if (kind == 'F') {
        if ((D32(0x43b498) == 0) || (sub == '0')) {
            name[3] = 'X';
        } else {
            name[3] = 'U';
            if (D32(0x43b480 + D32(0x43b494) * 4) == 0) {
                name[3] = 'L';
            }
        }
    } else {
        name[3] = 'U';
        if (D32(0x43b47c + D32(0x43b494) * 4) == 0) {
            name[3] = 'L';
        }
    }
    name[4] = sub;
    name[5] = 0;
    sprintf(buf, DSTR(0x436620), name);
}

// 0x41a2b6: plays the steps not yet seen for the current kid
void FUN_0041a2b6() {
    u32 uVar3;
    char local_10c[260];
    char local_8[4];

    memcpy(local_8, DPTR(0x436618), 4);
    for (uVar3 = (u32)D8(0x43b460 + D32(0x43b494)); (int)uVar3 < D32(0x43b468 + D32(0x43b494) * 4);
         uVar3 = uVar3 + 1) {
        D8(0x43b460 + D32(0x43b494)) = D8(0x43b460 + D32(0x43b494)) + 1;
        FUN_0041a1f7(D32(0x43b494));
        FUN_00403606();
        if ((int)uVar3 < 4) {
            FUN_00402bad(4, 2);
        } else {
            FUN_00402bad(D32(0x43b494) + 5, 2);
        }
        // asm: bytes 0 and 2 are replaced, 1 and 3 come from DAT_00436618
        local_8[0] = (char)D32(0x43b494) + '0';
        local_8[2] = (char)uVar3 + '1';
        sprintf(local_10c, DSTR(0x436630), local_8);
        Video_PlayDoubled(local_10c, D32(0x4365e4), D32(0x4365e8), FUN_0041a36f);
    }
}

// 0x41a36f: movie abort callback (never aborts)
int FUN_0041a36f(int ev) {
    (void)ev;
    return 0;
}

// 0x41a376
void FUN_0041a376() {
    s16 sVar1;

    if (g_mouseY < 0x1ae) {
        sVar1 = Hotspot_HitTest(g_mouseX, g_mouseY);
        if (sVar1 == 0x148) {
            Cursor_Set(4);
        } else if (sVar1 == 0x14b) {
            Cursor_Set(3);
        } else if (sVar1 == 0x14d) {
            Cursor_Set(5);
        }
    } else {
        Cursor_Set(2);
    }
}

// 0x41a3e9
void FUN_0041a3e9() {
    Anim_Load(0x6c4);
    D32(0x436600) = Spr_Create(nullptr, D32(0x436ee4), D32(0x436ee8), 0xc);
    Anim_StartWithSprite(0x6c4, 0, 0, 0xc, D32(0x436600));
    Timer_Set(2, 0);
    while (Anim_IsPlaying(0x6c4)) {
        if (Timer_Expired(2)) {
            Timer_Set(2, 100);
            Anim_UpdateAll();
            Spr_Update(1);
        }
    }
    Hotspot_Set(0x20, g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    P(0x4365fc) = Res_Load(0x6c6);
    D32(0x436604) = Spr_Create(nullptr, D32(0x436608), D32(0x43660c), 0xd);
    void* ang = P(0x4365fc);
    int spr = D32(0x436604);
    int r = Rand();
    FUN_00402e72(r % 7 + 0x8d5, ang, spr, 0, 1);
    Anim_Free(0x6c4);
    Spr_Delete(D32(0x436604), 0);
    Res_Free(P(0x4365fc));
    P(0x4365fc) = nullptr;
    Anim_Load(0x6c5);
    Anim_StartWithSprite(0x6c5, 0, 0, 0xc, D32(0x436600));
    Timer_Set(2, 0);
    while (Anim_IsPlaying(0x6c5)) {
        if (Timer_Expired(2)) {
            Timer_Set(2, 100);
            Anim_UpdateAll();
            Spr_Update(1);
        }
    }
    Hotspot_Remove(0x20);
    Spr_Delete(D32(0x436600), 1);
    Anim_Free(0x6c5);
    Spr_Update(1);
}

// 0x41a5ab: Ms. Grunkle's intro narration for this module
void FUN_0041a5ab(int param_1) {
    if (D32(0x43b494) == 0) {
        if (param_1 == 0) {
            if (D32(0x43980c) == 0) {
                FUN_00402ec2(D32(0x43661c) + 0x8c7, 0, 1);
                D32(0x43661c) = D32(0x43661c) + 1;
                if (2 < D32(0x43661c)) {
                    D32(0x43661c) = 0;
                }
            } else {
                FUN_00402ec2(0x16a6, 0, 1);
            }
        } else {
            FUN_00402ec2(0x8c6, 0, 1);
        }
    } else {
        FUN_00402ec2(D32(0x43661c) + 0x8d1, 0, 1);
        D32(0x43661c) = D32(0x43661c) + 1;
        if (2 < D32(0x43661c)) {
            D32(0x43661c) = 0;
        }
    }
}
