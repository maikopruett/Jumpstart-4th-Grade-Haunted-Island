// Unit m02 (0x404150..0x404f80): island navigation (module 2).
//
// Location table at 0x432180, 0xc bytes per location (index = location):
//   +0x0 s16 up target    (<0: -1 jumps to module +6, < -1 no forward move)
//   +0x2 s16 left target  +0x4 s16 right target (<0: none)
//   +0x6 int module to jump to (0xe: play the movie and enter module 0xe)
//   +0xa s16 palette index (resource 11000 + n)
// Current location at 0x432154. Location backgrounds are resource
// 10000 + loc; other locations reuse a captured frame (0x43215c).
#include "game/common.h"

static inline s16u& LocUp(int loc) { return D16(0x432180 + loc * 0xc); }
static inline s16u& LocLeft(int loc) { return D16(0x432182 + loc * 0xc); }
static inline s16u& LocRight(int loc) { return D16(0x432184 + loc * 0xc); }
static inline s32u& LocModule(int loc) { return D32(0x432186 + loc * 0xc); }
static inline s16u& LocPal(int loc) { return D16(0x43218a + loc * 0xc); }

REGISTER_CODE(0x404b86, FUN_00404b86);

// 0x404150
void FUN_00404150() { D16(0x4321b0) = (s16)0xfffe; }

// 0x40415e: module entry
void FUN_0040415e() {
    char local_28[33];
    u8 local_7 = 0; // uninitialized in the original (set before any move)

    int r = Rand() % 4;
    FUN_00402c78(r + 0x7f8);
    if (D32(0x43980c) != 0) FUN_00404150();
    int iVar7 = D32(0x432154);
    s16 local_6 = LocPal(D32(0x432154));
    FUN_00402b7e(0x809);
    FUN_0040467f(D32(0x432154));
    FUN_00404726(D32(0x432154), 1);
    FUN_00404aca(D32(0x432154), 1);
    FUN_00404895(D32(0x432154));
    if (g_prevModule == 1) {
        FUN_0040658a();
    } else if (2 < g_prevModule && g_prevModule < 0xd && Rand() % 7 < 1 && D32(0x43980c) == 0) {
        FUN_0041a3e9();
    }
    if (D32(0x432154) == 0x28f || D32(0x432154) == 0x298) {
        D32(0x43217c) = FUN_00402bd1(4, 1);
        if (D32(0x4398e0) == 0) {
            FUN_0040658a();
            D32(0x4398e0) = 1;
        }
    }
    Timer_Set(0xf, 300000);
    do {
        Event_Flush();
        while (iVar7 == D32(0x432154)) {
            FUN_00404b13();
            u32 uVar2 = (u32)Event_Poll();
            if (FUN_0040394c((u16)uVar2) == 0) {
                if (Timer_Expired(0xf)) {
                    Timer_Set(0xf, 300000);
                    FUN_004040f5(-1);
                }
                if (D32(0x439808) != 0) {
                    uVar2 = 0x148;
                    D32(0x439808) = 0;
                }
                s16 sVar6 = (s16)uVar2;
                if (sVar6 == 0x148) {
                    int iVar4 = LocUp(D32(0x432154));
                    if (iVar4 < 0) {
                        if (iVar4 == -1) Module_Jump(LocModule(D32(0x432154)));
                    } else {
                        local_7 = 0x66; // 'f'
                        int iVar3 = FUN_00404ba4(D32(0x432154), 0x148);
                        iVar7 = iVar4;
                        if (iVar3 == 0)
                            D32(0x432178) = FUN_00402bd1(2, 1);
                        else
                            D32(0x432178) = FUN_00402bd1(3, 1);
                    }
                } else if (sVar6 == 0x14b) {
                    int iVar4 = LocLeft(D32(0x432154));
                    if (-1 < iVar4) {
                        local_7 = 0x6c; // 'l'
                        int iVar3 = FUN_00404ba4(D32(0x432154), 0x14b);
                        iVar7 = iVar4;
                        if (iVar3 == 0)
                            D32(0x432178) = FUN_00402bd1(0, 1);
                        else
                            D32(0x432178) = FUN_00402bd1(3, 1);
                    }
                } else if (sVar6 == 0x14d) {
                    int iVar4 = LocRight(D32(0x432154));
                    if (-1 < iVar4) {
                        local_7 = 0x72; // 'r'
                        int iVar3 = FUN_00404ba4(D32(0x432154), 0x14d);
                        iVar7 = iVar4;
                        if (iVar3 == 0)
                            D32(0x432178) = FUN_00402bd1(1, 1);
                        else
                            D32(0x432178) = FUN_00402bd1(3, 1);
                    }
                } else if ((u16)(sVar6 - 0x181U) < 9) {
                    Module_Jump((int)(uVar2 & 0xffff) - 0x17e);
                }
            }
        }
        if (LocPal(iVar7) != local_6) {
            FUN_00404aca(iVar7, 0);
            local_6 = LocPal(iVar7);
        }
        if (LocModule(iVar7) == 0xe) {
            Hotspot_Set(0x20, g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
            int suffix = 0x6c; // 'l'
            if (4 < D8(0x43b460)) suffix = 0x75; // 'u'
            sprintf(local_28, DSTR(0x4341a0), D32(0x432154) / 0x14, D32(0x432154), (u32)local_7, suffix);
            Video_PlayDoubled(local_28, D32(0x432170), D32(0x432174), FUN_00404b86);
            Sound_Stop(D32(0x432178));
            Hotspot_Remove(0x20);
            D32(0x4318a0) = 0;
            Module_Jump(0xe);
        }
        FUN_0040467f(iVar7);
        Hotspot_Set(0x20, g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
        sprintf(local_28, DSTR(0x4341ba), D32(0x432154) / 0x14, D32(0x432154), (u32)local_7);
        Video_PlayDoubled(local_28, D32(0x432170), D32(0x432174), FUN_00404b86);
        Sound_Stop(D32(0x432178));
        FUN_004046b9(iVar7);
        Hotspot_Remove(0x20);
        if (D32(0x432154) == 0x28f || D32(0x432154) == 0x298) FUN_00402bf5(D32(0x43217c));
        if (iVar7 == 0x28f || iVar7 == 0x298) D32(0x43217c) = FUN_00402bd1(4, 1);
        D32(0x432154) = iVar7;
        FUN_00404726(iVar7, 1);
        if (FUN_0041862c()) FUN_00418678();
        int rep = FUN_0041ba50();
        if (rep != 0) {
            FUN_0041bab2(rep);
            FUN_0040467f(D32(0x432154));
            FUN_00404726(D32(0x432154), 0);
            FUN_0041bd8c();
        }
        if ((D32(0x432154) == 0x28f || D32(0x432154) == 0x298) && D32(0x4398e0) == 0) {
            FUN_0040658a();
            D32(0x4398e0) = 1;
        }
        FUN_00404895(D32(0x432154));
    } while (true);
}

// 0x40464c: 1 if the location has its own background resource
int FUN_0040464c(int param_1) {
    if (param_1 < 0x66) return 1;
    for (int i = 0;; i = i + 1) {
        if (D32(0x434170) <= i) return 0;
        if (param_1 == D32(0x434148 + i * 4)) break;
    }
    return 1;
}

// 0x40467f
void FUN_0040467f(int param_1) {
    if (FUN_0040464c(param_1) != 0) {
        Mem_Free(P(0x43215c));
        P(0x43215c) = nullptr;
        P(0x432158) = Res_Load(param_1 + 10000);
    }
}

// 0x4046b9
void FUN_004046b9(int param_1) {
    int r[4];
    for (int i = 0; i < 4; i++) r[i] = D32(0x434174 + i * 4);
    if (FUN_0040464c(param_1) == 0) {
        if (P(0x43215c) == nullptr) {
            u32 n = Img_RawSize(r[0], r[1], r[2], r[3]);
            P(0x43215c) = Mem_Alloc(n);
        }
        Img_Capture(r[0], r[1], r[2], r[3], P(0x43215c));
    }
}

// 0x404726
void FUN_00404726(int param_1, int param_2) {
    int local_c = D32(0x434184);
    int local_8 = D32(0x434188);
    int r[4];
    for (int i = 0; i < 4; i++) r[i] = D32(0x43418c + i * 4);
    if (FUN_0040464c(param_1) == 0) {
        if (P(0x43215c) != nullptr) {
            Gfx_FillRect(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h, 0x24);
            Img_DrawOpaque(local_c, local_8, P(0x43215c));
            Gfx_Copy2x(r[0], r[1], r[2], r[3], D32(0x432170), D32(0x432174));
            Gfx_SaveBg(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
            FUN_004032e3();
            Spr_Update(0);
            if (param_2 != 0) Screen_FlushAll();
        }
    } else {
        Gfx_FillRect(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h, 0x24);
        Img_Draw(D32(0x432170), D32(0x432174), P(0x432158), 0);
        Res_Free(P(0x432158));
        P(0x432158) = nullptr;
        Gfx_SaveBg(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
        FUN_004032e3();
        Spr_Update(0);
        if (param_2 != 0) Screen_FlushAll();
    }
}

// 0x404895: navigation hotspots (0x148 forward, 0x14b left, 0x14d right)
void FUN_00404895(int param_1) {
    Rect r;
    Hotspot_Clear();
    FUN_004038b9();
    int x = D32(0x432160), y = D32(0x432164), w = D32(0x432168), h = D32(0x43216c);
    if (param_1 == 1) {
        Rect_Set(&r, x, y, w / 2, h);
        Hotspot_Set(0x14b, r);
        Rect_Set(&r, w / 2 + x, y, w / 2, h);
        Hotspot_Set(0x14d, r);
        Rect_Set(&r, 0xed, 0x54, 2, 2);
        Hotspot_Set(0x148, r);
    } else if (LocUp(param_1) < -1) {
        Rect_Set(&r, x, y, w / 2, h);
        Hotspot_Set(0x14b, r);
        Rect_Set(&r, w / 2 + x, y, w / 2, h);
        Hotspot_Set(0x14d, r);
    } else {
        Rect_Set(&r, w / 4 + x, y, w / 2, h);
        Hotspot_Set(0x148, r);
        Rect_Set(&r, x, y, w / 4, h);
        Hotspot_Set(0x14b, r);
        Rect_Set(&r, (w * 3) / 4 + x, y, w / 4, h);
        Hotspot_Set(0x14d, r);
    }
}

// 0x404aca
void FUN_00404aca(int param_1, int param_2) {
    void* rgb = Res_Load((int)LocPal(param_1) + 11000);
    if (param_2 == 0)
        Pal_Set((const u8*)rgb, 1);
    else
        Pal_FadeTo((const u8*)rgb, 1);
    Res_Free(rgb);
}

// 0x404b13: cursor shape from the hotspot under the mouse
void FUN_00404b13() {
    if (g_mouseY < 0x1ae) {
        s16 id = Hotspot_HitTest(g_mouseX, g_mouseY);
        if (id == 0x148)
            Cursor_Set(4);
        else if (id == 0x14b)
            Cursor_Set(3);
        else if (id == 0x14d)
            Cursor_Set(5);
    } else {
        Cursor_Set(2);
    }
}

// 0x404b86: movie abort callback
int FUN_00404b86(int ev) {
    if ((s16)ev == 0x20 && D32(0x432154) < 0x66) return 1;
    return 0;
}

// 0x404ba4
int FUN_00404ba4(int param_1, s16 param_2) {
    if ((u32)(param_1 - 0x1d) < 4 || (u32)(param_1 - 0x29) < 4) return 1;
    if (param_1 == 0x44 && param_2 == 0x148) return 1;
    return 0;
}

// 0x404bdb
void FUN_00404bdb() { Hotspot_Clear(); }

// 0x404be5: Ms. Grunkle's talk for this module (called from FUN_0040658a)
int FUN_00404be5(int param_1) {
    int ret = 0;
    if (D32(0x432154) == 0x28f || D32(0x432154) == 0x298) {
        ret = FUN_00402ec2(0x1603, 0, 1);
    } else if (param_1 == 0) {
        if (D32(0x43980c) == 0) {
            if (D32(0x43b414) == 0) {
                int r = Rand() % 4;
                ret = FUN_00402ec2(r + 0x838, 0, 1);
            } else {
                int r = Rand();
                ret = FUN_00402ec2(r % 3 + 0x83c, 0, 1);
            }
        } else {
            ret = FUN_00402ec2(0x16a6, 0, 1);
        }
    } else {
        if (FUN_00402ec2(0x834, 0, 1) != 0) return 1;
        if (FUN_00402ec2(0x835, 0, 1) != 0) return 1;
        if (FUN_00402ec2(0x836, 0, 1) != 0) return 1;
        if (FUN_00402ec2(0x837, 0, 1) != 0) return 1;
    }
    return ret;
}
