// Module 7: math activity (0x40a144..0x40be0d).
//
// Globals (all segment memory):
//   0x439b58[2]   text sprite ids                 0x434d3c[2]  P: text images (Img_Create)
//   0x439b60      P: current hit/fall animation   0x439b64     P: life icon animation
//   0x439b68      P: player animation             0x439b6c[8]  P: shot animations (per direction)
//   0x439b8c      P: number digits animation      0x439b90     P: obstacle animation
//   0x439b94[3]   life icon sprite ids            0x439ba0     player sprite, 0x439ba4 shot sprite
//   0x439ba8[8]   obstacle sprite ids             0x439bc8[8]  number sprite ids (units digit)
//   0x439be8[8]   number sprite ids (tens digit)  0x439c18[8]  animation phase (0/1)
//   0x439c38[8]   obstacle step                   0x439c58[8]  lane state / counter
//   0x439c78[8]   lane kind (0 obstacle, 1 number) 0x439c98[8] number step
//   0x439cd8[8]   number values                   0x439d14 / 0x439d64  problem / solution text
//   0x439db4      answer                          0x439db8     hit value (-1 none, 9999 missed)
//   0x434d44[8]   byte: obstacle kind per lane    0x434d4c     table of (x, y) int pairs, 32 entries
#include "game/common.h"

// 0x40a144
void FUN_0040a144() {
    int iVar2;

    Spr_DeleteAll(0, 0);
    D32(0x439dbc) = 3;
    iVar2 = 0;
    do {
        D32(0x439c78 + iVar2 * 4) = -1;
        D32(0x439c58 + iVar2 * 4) = 0;
        iVar2 = iVar2 + 1;
    } while (iVar2 < 8);
    iVar2 = 0;
    do {
        D32(0x439ba8 + iVar2 * 4) = 0;
        D32(0x439bc8 + iVar2 * 4) = 0;
        D32(0x439be8 + iVar2 * 4) = 0;
        iVar2 = iVar2 + 1;
    } while (iVar2 < 8);
    iVar2 = 0;
    do {
        D32(0x439b58 + iVar2 * 4) = Spr_Create(nullptr, D32(0x436ee4), D32(0x436ee8), 0xb);
        iVar2 = iVar2 + 1;
    } while (iVar2 < 2);
    D32(0x439ba4) = 0;
    D32(0x439ba0) = 0;
}

// 0x40a1d7
void FUN_0040a1d7() {
    void* pvVar1 = Res_Load(0x2f44);
    Img_Draw(D32(0x436ee4), D32(0x436ee8), pvVar1, 0);
    Res_Free(pvVar1);
    Gfx_SaveBg(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    FUN_004032e3();
    Spr_Update(0);
    Screen_FlushAll();
    if (D32(0x439cfc) == 0) {
        pvVar1 = Res_Load(0x2f45);
        Pal_FadeTo((const u8*)pvVar1, 1);
        Res_Free(pvVar1);
    }
}

// 0x40a263
void FUN_0040a263() {
    int iVar1;

    if (P(0x439b60) != nullptr) {
        Res_Free(P(0x439b60));
        P(0x439b60) = nullptr;
    }
    iVar1 = 0;
    do {
        Mem_Free(P(0x434d3c + iVar1 * 4));
        P(0x434d3c + iVar1 * 4) = nullptr;
        iVar1 = iVar1 + 1;
    } while (iVar1 < 2);
    Res_Free(P(0x439b90));
    P(0x439b90) = nullptr;
    Res_Free(P(0x439b8c));
    P(0x439b8c) = nullptr;
    iVar1 = 0;
    do {
        Res_Free(P(0x439b6c + iVar1 * 4));
        P(0x439b6c + iVar1 * 4) = nullptr;
        iVar1 = iVar1 + 1;
    } while (iVar1 < 8);
    Res_Free(P(0x439b68));
    P(0x439b68) = nullptr;
    Res_Free(P(0x439b64));
    P(0x439b64) = nullptr;
}

// 0x40a318
void FUN_0040a318() {
    u32 uVar1;
    int iVar3;
    int local_c;
    u32 local_8;

    FUN_00403e91();
    uVar1 = FUN_004040cb();
    D32(0x439d0c) = uVar1 + 1;
    uVar1 = FUN_004040a2();
    D32(0x439d10) = uVar1 + 1;
    if (3 < D32(0x439d10)) {
        D32(0x439d10) = 3;
    }
    FUN_00402c78(0x7ec);
    P(0x439b60) = nullptr;
    P(0x439b64) = Res_Load(0x2f59);
    P(0x439b68) = Res_Load(0x2f4e);
    iVar3 = 0;
    do {
        P(0x439b6c + iVar3 * 4) = Res_Load(iVar3 + 0x2f4f);
        iVar3 = iVar3 + 1;
    } while (iVar3 < 8);
    P(0x439b8c) = Res_Load(0x2f57);
    P(0x439b90) = Res_Load(0x2f58);
    Text_MeasureFont(DSTR(0x434e5c), P(0x4318b0), &local_8, &local_c);
    iVar3 = 0;
    do {
        P(0x434d3c + iVar3 * 4) = Img_Create(local_8, local_c);
        iVar3 = iVar3 + 1;
    } while (iVar3 < 2);
    FUN_00402b7e(6000);
}

// 0x40a403: builds a new problem. param_1 = difficulty (1: +/-, 2: x/:, 3: mixed).
void FUN_0040a403(int param_1) {
    int iVar1;
    int iVar2;
    int iVar3;
    int iVar4;
    int local_c;
    int local_8;
    char* prob = DSTR(0x439d14);
    char* sol = DSTR(0x439d64);

    D32(0x439d08) = param_1;
    D32(0x439dc0) = D32(0x439dc0) + 1;
    D32(0x439db8) = -1;
    do {
        iVar1 = Rand();
        local_c = iVar1 % 3;
        do {
            if (param_1 == 1) {
                local_8 = Rand() % 2;
                iVar2 = Rand();
                iVar2 = iVar2 % 0xb;
                iVar3 = Rand();
                iVar3 = iVar3 % 0xb;
                iVar1 = iVar3 + iVar2;
            } else if (param_1 == 2) {
                do {
                    local_8 = Rand() % 2;
                    local_8 = local_8 + 2;
                    iVar2 = Rand();
                    iVar2 = iVar2 % 0xb;
                    iVar3 = Rand();
                    iVar3 = iVar3 % 0xb;
                    iVar1 = iVar2 * iVar3;
                } while ((local_8 == 2) && (99 < iVar1));
            } else {
                do {
                    local_8 = Rand() % 4;
                    if (local_8 < 2) {
                        iVar2 = Rand();
                        iVar2 = iVar2 % 0x33;
                        iVar3 = Rand();
                        iVar3 = iVar3 % 0x33;
                        iVar1 = iVar3 + iVar2;
                    } else {
                        iVar2 = Rand();
                        iVar2 = iVar2 % 0x15;
                        iVar3 = Rand();
                        iVar3 = iVar3 % 0x15;
                        iVar1 = iVar2 * iVar3;
                    }
                } while ((local_8 == 2) && (99 < iVar1));
            }
        } while ((iVar2 == 0) && (iVar3 == 0));
        if (local_8 == 0) {
            if (local_c == 0) {
                sprintf(prob, DSTR(0x434e6a), iVar2, iVar3);
                D32(0x439db4) = iVar1;
            } else if (local_c == 1) {
                sprintf(prob, DSTR(0x434e76), iVar3, iVar1);
                D32(0x439db4) = iVar2;
            } else if (local_c == 2) {
                sprintf(prob, DSTR(0x434e82), iVar2, iVar1);
                D32(0x439db4) = iVar3;
            }
            sprintf(sol, DSTR(0x434e8e), iVar2, iVar3, iVar1);
        } else if (local_8 == 1) {
            if (local_c == 0) {
                sprintf(prob, DSTR(0x434e9b), iVar1, iVar2);
                D32(0x439db4) = iVar3;
            } else if (local_c == 1) {
                sprintf(prob, DSTR(0x434ea7), iVar2, iVar3);
                D32(0x439db4) = iVar1;
            } else if (local_c == 2) {
                sprintf(prob, DSTR(0x434eb3), iVar1, iVar3);
                D32(0x439db4) = iVar2;
            }
            sprintf(sol, DSTR(0x434ebf), iVar1, iVar2, iVar3);
        } else if (local_8 == 2) {
            if (local_c == 0) {
                sprintf(prob, DSTR(0x434ecc), iVar2, 0xb5, iVar3);
                D32(0x439db4) = iVar1;
            } else if (local_c == 1) {
                if (iVar3 == 0) {
                    iVar3 = iVar2;
                    iVar2 = 0;
                }
                sprintf(prob, DSTR(0x434ed9), 0xb5, iVar3, iVar1);
                D32(0x439db4) = iVar2;
            } else if (local_c == 2) {
                iVar4 = iVar3;
                if (iVar2 == 0) {
                    iVar4 = 0;
                    iVar2 = iVar3;
                }
                sprintf(prob, DSTR(0x434ee6), iVar2, 0xb5, iVar1);
                iVar3 = iVar4;
                D32(0x439db4) = iVar4;
            }
            sprintf(sol, DSTR(0x434ef3), iVar2, 0xb5, iVar3, iVar1);
        } else if (local_8 == 3) {
            iVar4 = iVar3;
            if (iVar2 == 0) {
                iVar4 = 0;
                iVar2 = iVar3;
            }
            if ((iVar4 == 0) && (local_c == 2)) {
                local_c = Rand() % 2;
            }
            if (local_c == 0) {
                sprintf(prob, DSTR(0x434f01), iVar1, 0xf7, iVar2);
                D32(0x439db4) = iVar4;
            } else if (local_c == 1) {
                sprintf(prob, DSTR(0x434f0e), 0xf7, iVar2, iVar4);
                D32(0x439db4) = iVar1;
            } else if (local_c == 2) {
                sprintf(prob, DSTR(0x434f1b), iVar1, 0xf7, iVar4);
                D32(0x439db4) = iVar2;
            }
            sprintf(sol, DSTR(0x434f28), iVar1, 0xf7, iVar2, iVar4);
        }
    } while (99 < D32(0x439db4));
}

// 0x40a7b4: renders text into text image param_2 and shows it at (param_3, param_4).
void FUN_0040a7b4(char* param_1, int param_2, int param_3, int param_4) {
    void* img = P(0x434d3c + param_2 * 4);
    Rect r;

    Gfx_SetTargetImg(img);
    Img_FullRect(img, &r);
    u8 bVar1 = Img_Key(img);
    Gfx_FillRect(r.x, r.y, r.w, r.h, bVar1);
    Text_DrawBoxFont(r.x, r.y, r.w, r.h, param_1, P(0x4318b0), 0x33);
    Gfx_ResetTarget();
    Spr_Invalidate(D32(0x439b58 + param_2 * 4));
    Spr_SetImage(D32(0x439b58 + param_2 * 4), img);
    Spr_SetPos(D32(0x439b58 + param_2 * 4), param_3, param_4);
}

// 0x40a865: shows the problem (or the solution once something was hit).
void FUN_0040a865() {
    char* puVar1;
    Rect r;
    int local_c;
    int local_8;

    local_c = D32(0x434e4c);
    local_8 = D32(0x434e50);
    Img_FullRect(P(0x434d3c), &r);
    local_c = (D32(0x43184c) - r.w) / 2;
    if (D32(0x439db8) < 0) {
        puVar1 = DSTR(0x439d14);
    } else {
        puVar1 = DSTR(0x439d64);
    }
    FUN_0040a7b4(puVar1, 0, local_c, local_8);
}

// 0x40a8cc
void FUN_0040a8cc() {
    if (D32(0x439ba4) != 0) {
        Spr_Delete(D32(0x439ba4), 1);
        D32(0x439ba4) = 0;
    }
    if (D32(0x439ba0) != 0) {
        Spr_Delete(D32(0x439ba0), 1);
        D32(0x439ba0) = 0;
    }
}

// 0x40a912
void FUN_0040a912() {
    const void* img;
    int iVar1;
    int iVar2;
    Point pt;

    FUN_0040a8cc();
    if (P(0x439b60) != nullptr) {
        Res_Free(P(0x439b60));
        P(0x439b60) = nullptr;
    }
    iVar1 = D32(0x439c08) / 2;
    iVar2 = iVar1 + -2;
    if (iVar2 < 0) {
        iVar2 = iVar1 + 6;
    }
    P(0x439b60) = Res_Load(iVar2 + 0x2f46);
    D32(0x439c08) = 0;
    Ang_FrameInfo(P(0x439b60), 1, &pt);
    img = Ang_FrameImage(P(0x439b60), D32(0x439c08));
    D32(0x439ba0) = Spr_Create(img, pt.x, pt.y, -2);
}

// 0x40a9b8
void FUN_0040a9b8(int param_1) {
    const void* img;
    Point pt;

    if (D32(0x439ba0) != 0) {
        D32(0x439c08) = param_1;
        Ang_FrameInfo(P(0x439b60), param_1 + 1, &pt);
        img = Ang_FrameImage(P(0x439b60), D32(0x439c08));
        Spr_SetImage(D32(0x439ba0), img);
        Spr_SetPos(D32(0x439ba0), pt.x, pt.y);
    }
}

// 0x40aa1f
void FUN_0040aa1f(int param_1) {
    const void* img;
    Point pt;

    if (D32(0x439ba0) != 0) {
        if (D32(0x439ba4) != 0) {
            Spr_Delete(D32(0x439ba4), 1);
            D32(0x439ba4) = 0;
        }
        D32(0x439c08) = param_1;
        Ang_FrameInfo(P(0x439b68), param_1 + 1, &pt);
        img = Ang_FrameImage(P(0x439b68), D32(0x439c08));
        Spr_SetImage(D32(0x439ba0), img);
        Spr_SetPos(D32(0x439ba0), pt.x, pt.y);
    }
}

// 0x40aaa6: centers image param_2 on param_3 (x, y; updated in place) and sets sprite param_1.
void FUN_0040aaa6(int param_1, const void* param_2, int* param_3) {
    Rect r;

    Img_FullRect(param_2, &r);
    param_3[0] = param_3[0] - r.w / 2;
    param_3[1] = param_3[1] - r.h / 2;
    Spr_SetImage(param_1, param_2);
    Spr_SetPos(param_1, param_3[0], param_3[1]);
}

// 0x40aafd
void FUN_0040aafd(int param_1, int param_2) {
    const void* pvVar1;
    int iVar2;
    int pos[2];

    if (D32(0x439ba8 + param_1 * 4) != 0) {
        D32(0x439c38 + param_1 * 4) = param_2;
        iVar2 = (param_1 * 4 + 3) - param_2;
        pos[0] = D32(0x434d4c + iVar2 * 8);
        pos[1] = D32(0x434d50 + iVar2 * 8);
        pvVar1 = Ang_FrameImage(P(0x439b90), (3 - param_2) + (u32)D8(0x434d44 + param_1) * 5 +
                                                  D32(0x439c18 + param_1 * 4) * 0x28);
        FUN_0040aaa6(D32(0x439ba8 + param_1 * 4), pvVar1, pos);
    }
}

// 0x40ab84
void FUN_0040ab84(int param_1, int param_2) {
    int iVar1;
    const void* pvVar2;
    Rect r;
    int local_10;
    int pos[2]; // local_c, local_8

    if (D32(0x439bc8 + param_1 * 4) != 0) {
        local_10 = D32(0x439cd8 + param_1 * 4) % 10;
        D32(0x439c98 + param_1 * 4) = param_2;
        iVar1 = (param_1 * 4 + 3) - param_2;
        pos[0] = D32(0x434d4c + iVar1 * 8);
        pos[1] = D32(0x434d50 + iVar1 * 8);
        pvVar2 = Ang_FrameImage(P(0x439b8c),
                                (3 - param_2) + local_10 * 5 + D32(0x439c18 + param_1 * 4) * 0x32);
        FUN_0040aaa6(D32(0x439bc8 + param_1 * 4), pvVar2, pos);
        if (9 < D32(0x439cd8 + param_1 * 4)) {
            iVar1 = (param_1 * 4 + 3) - param_2;
            pos[0] = D32(0x434d4c + iVar1 * 8);
            pos[1] = D32(0x434d50 + iVar1 * 8);
            pvVar2 = Ang_FrameImage(P(0x439b8c), (3 - param_2) + local_10 * 5);
            Img_FullRect(pvVar2, &r);
            pos[0] = pos[0] - r.w / 2;
            pos[1] = pos[1] + r.h / 2;
            local_10 = D32(0x439cd8 + param_1 * 4) / 10;
            pvVar2 = Ang_FrameImage(P(0x439b8c), (3 - param_2) + local_10 * 5);
            Img_FullRect(pvVar2, &r);
            pos[0] = pos[0] - r.w / 2;
            pos[1] = pos[1] - r.h / 2;
            pvVar2 = Ang_FrameImage(P(0x439b8c),
                                    (3 - param_2) + local_10 * 5 + D32(0x439c18 + param_1 * 4) * 0x32);
            Img_FullRect(pvVar2, &r);
            pos[0] = pos[0] - r.w / 2;
            pos[1] = pos[1] - r.h / 2;
            Spr_SetImage(D32(0x439be8 + param_1 * 4), pvVar2);
            Spr_SetPos(D32(0x439be8 + param_1 * 4), pos[0], pos[1]);
        }
    }
}

// 0x40ad5f: fires a shot in the direction the player faces.
void FUN_0040ad5f(u32 param_1) {
    int iVar1;
    int iVar2;
    const void* img;
    Point pt;

    if (D32(0x439ba4) != 0) {
        Spr_Delete(D32(0x439ba4), 1);
        D32(0x439ba4) = 0;
    }
    iVar1 = (s32)param_1 / 2;
    iVar2 = iVar1 + 4;
    if (7 < iVar2) {
        iVar2 = iVar1 + -4;
    }
    D32(0x439c14) = 0;
    D32(0x439c10) = D32(0x439c08) / 2;
    D32(0x439c0c) = 3;
    if (D32(0x439ba8 + iVar2 * 4) != 0) {
        D32(0x439c14) = 1;
        D32(0x439c0c) = D32(0x439c38 + iVar2 * 4);
        D32(0x439c58 + iVar2 * 4) = -1;
    }
    if (D32(0x439bc8 + iVar2 * 4) != 0) {
        D32(0x439c14) = 1;
        D32(0x439c0c) = D32(0x439c98 + iVar2 * 4);
        D32(0x439c58 + iVar2 * 4) = -2;
    }
    Ang_FrameInfo(P(0x439b6c + D32(0x439c10) * 4), D32(0x439c0c) + 1, &pt);
    img = Ang_FrameImage(P(0x439b6c + D32(0x439c10) * 4), D32(0x439c0c));
    D32(0x439ba4) = Spr_Create(img, pt.x, pt.y, -2);
}

// 0x40ae70: creates obstacle sprite in lane param_1 at step param_2.
void FUN_0040ae70(int param_1, int param_2) {
    const void* img;
    int iVar1;
    int pos[2];

    D32(0x439c38 + param_1 * 4) = param_2;
    iVar1 = (param_1 * 4 + 3) - param_2;
    pos[0] = D32(0x434d4c + iVar1 * 8);
    pos[1] = D32(0x434d50 + iVar1 * 8);
    img = Ang_FrameImage(P(0x439b90), (3 - param_2) + (u32)D8(0x434d44 + param_1) * 5);
    iVar1 = Spr_Create(img, pos[0], pos[1], -9);
    D32(0x439ba8 + param_1 * 4) = iVar1;
    FUN_0040aaa6(D32(0x439ba8 + param_1 * 4), img, pos);
}

// 0x40aefa: creates number sprite(s) in lane param_1 at step param_2.
void FUN_0040aefa(int param_1, int param_2) {
    int iVar1;
    const void* pvVar2;
    Rect r;
    int local_10;
    int pos[2]; // local_c, local_8

    local_10 = D32(0x439cd8 + param_1 * 4) % 10;
    D32(0x439c98 + param_1 * 4) = param_2;
    iVar1 = (param_1 * 4 + 3) - param_2;
    pos[0] = D32(0x434d4c + iVar1 * 8);
    pos[1] = D32(0x434d50 + iVar1 * 8);
    pvVar2 = Ang_FrameImage(P(0x439b8c), (3 - param_2) + local_10 * 5);
    iVar1 = Spr_Create(pvVar2, pos[0], pos[1], -9);
    D32(0x439bc8 + param_1 * 4) = iVar1;
    FUN_0040aaa6(D32(0x439bc8 + param_1 * 4), pvVar2, pos);
    if (9 < D32(0x439cd8 + param_1 * 4)) {
        iVar1 = (param_1 * 4 + 3) - param_2;
        pos[0] = D32(0x434d4c + iVar1 * 8);
        pos[1] = D32(0x434d50 + iVar1 * 8);
        pvVar2 = Ang_FrameImage(P(0x439b8c), (3 - param_2) + local_10 * 5);
        Img_FullRect(pvVar2, &r);
        pos[0] = pos[0] - r.w / 2;
        pos[1] = pos[1] + r.h / 2;
        local_10 = D32(0x439cd8 + param_1 * 4) / 10;
        pvVar2 = Ang_FrameImage(P(0x439b8c), (3 - param_2) + local_10 * 5);
        Img_FullRect(pvVar2, &r);
        // not halved here (unlike FUN_0040ab84), as in the original
        pos[0] = pos[0] - r.w;
        pos[1] = pos[1] - r.h;
        iVar1 = Spr_Create(pvVar2, pos[0], pos[1], -9);
        D32(0x439be8 + param_1 * 4) = iVar1;
    }
}

// 0x40b069
void FUN_0040b069(int param_1) {
    const void* img;
    Point pt;

    FUN_0040a8cc();
    D32(0x439c08) = param_1;
    Ang_FrameInfo(P(0x439b68), param_1 + 1, &pt);
    img = Ang_FrameImage(P(0x439b68), param_1);
    D32(0x439ba0) = Spr_Create(img, pt.x, pt.y, -2);
}

// 0x40b0c0
void FUN_0040b0c0() {
    const void* img;
    int iVar1;
    int iVar2;
    int iVar3;
    Point pt;

    D32(0x439dcc) = 0;
    D32(0x439dc0) = 0;
    D32(0x439dd0) = 0;
    D32(0x439dd4) = 0;
    D32(0x439d04) = 2;
    D32(0x439dc4) = 0;
    D32(0x439dc8) = 0;
    FUN_0040a403(D32(0x439d10));
    FUN_0040a865();
    iVar2 = 0;
    do {
        D32(0x439c38 + iVar2 * 4) = -9;
        D32(0x439c98 + iVar2 * 4) = -9;
        iVar2 = iVar2 + 1;
    } while (iVar2 < 8);
    img = Ang_FrameImage(P(0x439b64), 0);
    iVar2 = Img_W(img);
    Pt_Set(&pt, 0, 0);
    for (iVar3 = 0; iVar3 < D32(0x439dbc); iVar3 = iVar3 + 1) {
        iVar1 = Spr_Create(img, pt.x, pt.y, 0xc);
        D32(0x439b94 + iVar3 * 4) = iVar1;
        pt.x = pt.x + iVar2 + 5;
    }
    FUN_0040b069(0);
    D32(0x439dbc) = D32(0x439dbc) + -1;
    Spr_Delete(D32(0x439b94 + D32(0x439dbc) * 4), 0);
}

// 0x40b1b2
void FUN_0040b1b2(int param_1) {
    if (D32(0x439ba8 + param_1 * 4) != 0) {
        Spr_Delete(D32(0x439ba8 + param_1 * 4), 1);
        D32(0x439ba8 + param_1 * 4) = 0;
        D32(0x439c58 + param_1 * 4) = 0;
        D32(0x439c38 + param_1 * 4) = -9;
        D32(0x439dc4) = D32(0x439dc4) + -1;
    }
}

// 0x40b1fa
void FUN_0040b1fa(int param_1) {
    if (D32(0x439bc8 + param_1 * 4) != 0) {
        Spr_Delete(D32(0x439bc8 + param_1 * 4), 1);
        D32(0x439bc8 + param_1 * 4) = 0;
        if (D32(0x439be8 + param_1 * 4) != 0) {
            Spr_Delete(D32(0x439be8 + param_1 * 4), 1);
            D32(0x439be8 + param_1 * 4) = 0;
        }
        D32(0x439c58 + param_1 * 4) = 0;
        D32(0x439c98 + param_1 * 4) = -9;
        D32(0x439cd8 + param_1 * 4) = -1;
        D32(0x439dc8) = D32(0x439dc8) + -1;
    }
}

// 0x40b271: shot impact.
void FUN_0040b271() {
    const void* img;
    int iVar1;
    int iVar2;
    Point pt;

    if (D32(0x439ba4) != 0) {
        iVar1 = D32(0x439c08) / 2;
        iVar2 = iVar1 + 4;
        if (7 < iVar2) {
            iVar2 = iVar1 + -4;
        }
        if (D32(0x439c14) == 0) {
            if (D32(0x439ba4) != 0) {
                Spr_Delete(D32(0x439ba4), 1);
                D32(0x439ba4) = 0;
            }
            D32(0x439dd8) = 0;
        } else {
            D32(0x439c14) = 0;
            D32(0x439c0c) = 0;
            Ang_FrameInfo(P(0x439b6c + D32(0x439c10) * 4), 1, &pt);
            img = Ang_FrameImage(P(0x439b6c + D32(0x439c10) * 4), D32(0x439c0c));
            Spr_SetImage(D32(0x439ba4), img);
            Spr_SetPos(D32(0x439ba4), pt.x, pt.y);
            if (D32(0x439ba8 + iVar2 * 4) != 0) {
                FUN_00402bad(10, 2);
                FUN_0040b1b2(iVar2);
                D32(0x439dd4) = D32(0x439dd4) + 1;
                FUN_00403e2c(10);
            }
            if (D32(0x439bc8 + iVar2 * 4) != 0) {
                D32(0x439db8) = D32(0x439cd8 + iVar2 * 4);
                if (D32(0x439db8) == D32(0x439db4)) {
                    FUN_00402bad(6, 3);
                } else {
                    FUN_00402bad(7, 3);
                }
                FUN_0040b1fa(iVar2);
                D32(0x439dd4) = D32(0x439dd4) + 1;
            }
        }
    }
}

// 0x40b3cf: player rotation step.
void FUN_0040b3cf() {
    if (D32(0x439dd8) == 1) {
        D32(0x439ddc) = D32(0x439ddc) + -1;
        if (D32(0x439ddc) < 1) {
            D32(0x439dd8) = 0;
        } else {
            if ((DU16(0x439de0) == 0x148) || (DU16(0x439de0) == 0x14b)) {
                D32(0x439c08) = D32(0x439c08) + 1;
                if (0xf < D32(0x439c08)) {
                    D32(0x439c08) = 0;
                }
                FUN_0040aa1f(D32(0x439c08));
                return;
            }
            if ((DU16(0x439de0) == 0x14d) || (DU16(0x439de0) == 0x150)) {
                if (D32(0x439c08) < 1) {
                    D32(0x439c08) = 0x10;
                }
                D32(0x439c08) = D32(0x439c08) + -1;
                FUN_0040aa1f(D32(0x439c08));
                return;
            }
        }
    }
}

// 0x40b465: advances lane param_1.
void FUN_0040b465(int param_1) {
    int iVar1;
    int iVar2;
    int uVar3;

    if (0 < D32(0x439c58 + param_1 * 4)) {
        if (D32(0x439c78 + param_1 * 4) == 0) {
            iVar1 = D32(0x439c38 + param_1 * 4) + -1;
            if (-1 < iVar1) {
                FUN_0040aafd(param_1, iVar1);
                return;
            }
            D32(0x439c38 + param_1 * 4) = iVar1;
            if (iVar1 == -1) {
                FUN_00402bad(0xb, 1);
                Spr_Delete(D32(0x439ba8 + param_1 * 4), 1);
                D32(0x439ba8 + param_1 * 4) = 0;
                D32(0x439dc4) = D32(0x439dc4) + -1;
                D32(0x439db8) = 9999;
                return;
            }
            if (D32(0x439dc4) < D32(0x439d08)) {
                FUN_0040ae70(param_1, 3);
                D32(0x439dc4) = D32(0x439dc4) + 1;
                return;
            }
        } else {
            iVar1 = D32(0x439c98 + param_1 * 4);
            iVar2 = iVar1 + -1;
            if (-1 < iVar2) {
                FUN_0040ab84(param_1, iVar2);
                return;
            }
            D32(0x439c98 + param_1 * 4) = iVar2;
            if (iVar1 == 0) {
                FUN_00402bad(0xb, 1);
                Spr_Delete(D32(0x439bc8 + param_1 * 4), 1);
                D32(0x439bc8 + param_1 * 4) = 0;
                if (D32(0x439be8 + param_1 * 4) != 0) {
                    Spr_Delete(D32(0x439be8 + param_1 * 4), 1);
                    D32(0x439be8 + param_1 * 4) = 0;
                }
                D32(0x439dc8) = D32(0x439dc8) + -1;
                if (D32(0x439cd8 + param_1 * 4) == D32(0x439db4)) {
                    D32(0x439db8) = 9999;
                    return;
                }
            } else if (D32(0x439dc8) < D32(0x439d04)) {
                if (D32(0x439dc8) == 1) {
                    uVar3 = 1;
                    iVar1 = 0;
                    do {
                        if (((param_1 != iVar1) && (D32(0x439bc8 + iVar1 * 4) != 0)) &&
                            (D32(0x439cd8 + iVar1 * 4) == D32(0x439db4))) {
                            uVar3 = 0;
                            break;
                        }
                        iVar1 = iVar1 + 1;
                    } while (iVar1 < 8);
                } else {
                    uVar3 = Rand() % 2;
                }
                if (uVar3 == 0) {
                    iVar1 = Rand();
                    D32(0x439cd8 + param_1 * 4) = (D32(0x439db4) - iVar1 % 3) + -1;
                    uVar3 = Rand() % 2;
                    if ((uVar3 != 0) || (D32(0x439cd8 + param_1 * 4) < 0)) {
                        iVar1 = Rand();
                        D32(0x439cd8 + param_1 * 4) = iVar1 % 3 + D32(0x439db4) + 1;
                        if (99 < D32(0x439cd8 + param_1 * 4)) {
                            iVar1 = Rand();
                            D32(0x439cd8 + param_1 * 4) = (D32(0x439db4) - iVar1 % 3) + -1;
                        }
                    }
                } else {
                    D32(0x439cd8 + param_1 * 4) = D32(0x439db4);
                }
                FUN_0040aefa(param_1, 3);
                D32(0x439dc8) = D32(0x439dc8) + 1;
            }
        }
    }
}

// 0x40b69e: input while idle (arrow keys rotate, space fires).
void FUN_0040b69e(u16 param_1) {
    if (D32(0x439dd8) == 0) {
        if (param_1 < 0x14c) {
            if (param_1 == 0x14b) {
            rotate:
                D32(0x439ddc) = 3;
                DU16(0x439de0) = param_1; // word store
                D32(0x439dd8) = 1;
                Timer_Set(1, 0x32);
                return;
            }
            if (param_1 == 0x20) {
                if (D32(0x439db8) != 9999) {
                    FUN_0040ad5f(D32(0x439c08));
                    FUN_00402bad(5, 4);
                    D32(0x439dd8) = 2;
                    Timer_Set(1, 200);
                }
            } else if (param_1 == 0x148) {
                goto rotate;
            }
        } else if ((param_1 == 0x14d) || (param_1 == 0x150)) {
            goto rotate;
        }
    }
}

// 0x40b743: per-event game step.
void FUN_0040b743(u16 param_1) {
    int iVar2;
    int iVar3;

    iVar2 = FUN_0040394c(param_1);
    if (iVar2 == 0) {
        if (param_1 == 0xc) {
            FUN_00403f95();
        }
        if (Timer_Expired(3)) {
            Timer_Set(3, 0x32);
            iVar2 = 0;
            do {
                if (D32(0x439ba8 + iVar2 * 4) != 0) {
                    D32(0x439c18 + iVar2 * 4) = 1 - D32(0x439c18 + iVar2 * 4);
                    FUN_0040aafd(iVar2, D32(0x439c38 + iVar2 * 4));
                }
                if (D32(0x439bc8 + iVar2 * 4) != 0) {
                    D32(0x439c18 + iVar2 * 4) = 1 - D32(0x439c18 + iVar2 * 4);
                    FUN_0040ab84(iVar2, D32(0x439c98 + iVar2 * 4));
                }
                iVar2 = iVar2 + 1;
            } while (iVar2 < 8);
        }
        if (Timer_Expired(2) && (D32(0x439db8) < 0)) {
            iVar2 = 0;
            do {
                if (-1 < D32(0x439c58 + iVar2 * 4)) {
                    iVar3 = Rand();
                    D32(0x439c58 + iVar2 * 4) = (u32)(iVar3 % 6 == 1);
                    if (D32(0x439c58 + iVar2 * 4) == 0) {
                        D32(0x439cf8) = D32(0x439cf8) + -1;
                        if (10 < D32(0x439cf8)) {
                            D32(0x439c58 + iVar2 * 4) = 1;
                            D32(0x439cf8) = 0;
                        }
                    }
                }
                if ((D32(0x439ba8 + iVar2 * 4) == 0) && (D32(0x439bc8 + iVar2 * 4) == 0)) {
                    D32(0x439c78 + iVar2 * 4) = Rand() % 2;
                }
                FUN_0040b465(iVar2);
            } while ((D32(0x439c58 + iVar2 * 4) < 1) && (iVar2 = iVar2 + 1, iVar2 < 8));
            Timer_Set(2, 500);
        }
        switch (D32(0x439dd8)) {
        case 0:
            FUN_0040b69e(param_1);
            if (-1 < D32(0x439db8)) {
                FUN_00402bad(8, 2);
                iVar2 = 0;
                do {
                    if (D32(0x439ba8 + iVar2 * 4) != 0) {
                        FUN_0040b1b2(iVar2);
                    }
                    if (D32(0x439bc8 + iVar2 * 4) != 0) {
                        FUN_0040b1fa(iVar2);
                    }
                    D32(0x439c58 + iVar2 * 4) = -5;
                    iVar2 = iVar2 + 1;
                } while (iVar2 < 8);
                if (D32(0x439db8) == D32(0x439db4)) {
                    if (D32(0x439ba4) != 0) {
                        Spr_Delete(D32(0x439ba4), 1);
                        D32(0x439ba4) = 0;
                    }
                    FUN_0040a865();
                    D32(0x439dcc) = D32(0x439dcc) + 1;
                    FUN_00404068(1);
                    FUN_00403e2c(0x14);
                    D32(0x439dd8) = 5;
                    Timer_Set(1, 500);
                } else {
                    FUN_00404068(0);
                    FUN_00402bad(0xc, 4);
                    FUN_0040a912();
                    D32(0x439dd8) = 3;
                    Timer_Set(1, 100);
                }
            }
            break;
        case 1:
            if (Timer_Expired(1)) {
                FUN_0040b3cf();
                Timer_Set(1, 0x32);
            }
            break;
        case 2:
            if (Timer_Expired(1)) {
                FUN_0040b271();
                Timer_Set(1, 200);
            }
            break;
        case 3:
            if (Timer_Expired(1)) {
                Timer_Set(1, 100);
                D32(0x439c08) = D32(0x439c08) + 1;
                if (D32(0x439c08) < 5) {
                    FUN_0040a9b8(D32(0x439c08));
                } else {
                    FUN_0040a8cc();
                    D32(0x439dd8) = 4;
                }
            }
            break;
        case 4:
            if (Timer_Expired(1)) {
                Timer_Set(1, 500);
                D32(0x439c58) = D32(0x439c58) + 1;
                if (-3 < D32(0x439c58)) {
                    if (D32(0x439dbc) == 0) {
                        D32(0x439d00) = 1;
                    } else {
                        D32(0x439dbc) = D32(0x439dbc) + -1;
                        Spr_Delete(D32(0x439b94 + D32(0x439dbc) * 4), 1);
                        FUN_00402bad(9, 2);
                        D32(0x439dd8) = 5;
                    }
                }
            }
            break;
        case 5:
            if (Timer_Expired(1)) {
                Timer_Set(1, 500);
                D32(0x439c58) = D32(0x439c58) + 1;
                if (-2 < D32(0x439c58)) {
                    if (0x13 < D32(0x439dc0)) {
                        D32(0x439dc0) = 0;
                        D32(0x439dcc) = 0;
                        if (D32(0x439ba0) != 0) {
                            D32(0x439c58) = -16;
                            D32(0x439ddc) = 0x29 - D32(0x439c08);
                            D32(0x439dd8) = 6;
                            FUN_00402bad(0xd, 1);
                            return;
                        }
                    }
                    if (D32(0x439ba0) == 0) {
                        FUN_0040b069(0);
                    }
                    iVar2 = 0;
                    do {
                        D32(0x439c58 + iVar2 * 4) = 0;
                        iVar2 = iVar2 + 1;
                    } while (iVar2 < 8);
                    FUN_0040a403(D32(0x439d10));
                    FUN_0040a865();
                    FUN_00402bad(0, 1);
                    D32(0x439dd8) = 0;
                }
            }
            break;
        case 6:
            if (D32(0x439ddc) < 2) {
                FUN_00419330(1);
                D32(0x439d00) = 1;
                D32(0x439dd8) = 7;
            } else if (Timer_Expired(1)) {
                D32(0x439dd8) = 1;
                DU16(0x439de0) = 0x14b; // word store
                FUN_0040b3cf();
                D32(0x439dd8) = 6;
                Timer_Set(1, 0x32);
            }
        }
    }
}

// 0x40bbf5
bool FUN_0040bbf5() {
    return 2 < D32(0x439cfc);
}

// 0x40bc07
void FUN_0040bc07() {
}

// 0x40bc0c
void FUN_0040bc0c() {
    Fx_CloseBlack();
}

// 0x40bc16: one round.
void FUN_0040bc16() {
    FUN_0040a144();
    FUN_0040a1d7();
    FUN_0040b0c0();
    Spr_Update(1);
    if (D32(0x439cfc) == 0) {
        FUN_0040658a();
    }
    D32(0x439dd8) = 0;
    D32(0x439d00) = 0;
    Timer_Set(0, 0x32);
    Timer_Set(1, 200);
    Timer_Set(2, 500);
    Timer_Set(3, 0x32);
    Event_Flush();
    do {
        while (!Timer_Expired(0)) {
            int uVar2 = Event_Poll();
            FUN_0040b743((u16)uVar2);
            Spr_Update(1);
        }
        Timer_Set(0, 0x32);
    } while (D32(0x439d00) == 0);
    FUN_0040bc0c();
    FUN_0040bc07();
}

// 0x40bccb: module 7 entry.
void FUN_0040bccb() {
    FUN_0040a318();
    D32(0x439cfc) = 0;
    do {
        FUN_0040bc16();
        D32(0x439cfc) = D32(0x439cfc) + 1;
    } while (!FUN_0040bbf5());
    FUN_004040f5(1);
}

// 0x40bcf8: module 7 cleanup.
void FUN_0040bcf8() {
    FUN_0040bc07();
    FUN_0040a263();
    Spr_DeleteAll(0, 0);
    FUN_00405901();
}

// 0x40bd18: help narration.
void FUN_0040bd18(int param_1) {
    if (param_1 == 0) {
        FUN_00402ec2(D32(0x434e54) + 0x15a4, 0, 1);
        D32(0x434e54) = D32(0x434e54) + 1;
        if (2 < D32(0x434e54)) {
            D32(0x434e54) = 0;
        }
    } else {
        FUN_00402ec2(0x15a3, 0, 1);
    }
}
