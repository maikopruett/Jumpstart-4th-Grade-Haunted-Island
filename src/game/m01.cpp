// Unit m01 (0x404f80..0x406890): sign-in / player roster (module 1), plus
// Ms. Grunkle's pop-up talk used by the other modules (FUN_0040658a).
//
// Roster.Dat (in g_exeDir), little-endian, unchanged from the original:
//   u32 recordSize (0x329; 0x327 = older layout, converted on load)
//   u32 count
//   count records of recordSize bytes
// Kid record (0x329 bytes, packed). The roster is a heap array of these at
// P(0x4341d8), count at 0x4341d4, current kid index at 0x4341e8.
//   +0x000 char name[13]
//   +0x00d u32  <-> 0x43980c
//   +0x011 u8[0x218] <-> 0x4395f0..0x439808
//   +0x229 u8[0x28]  <-> 0x43b414
//   +0x251 u8[0x18]  <-> 0x43b43c
//   +0x269 u32 <-> 0x43b460   +0x26d u32 <-> 0x43b464
//   +0x271 u8[0xac]  <-> 0x439838 (per-module "visited" flags etc.)
//   +0x31d u32 <-> 0x43b508   +0x321 u32 <-> 0x43b50c   +0x325 u32 <-> 0x43b510
// The 0x327-byte layout is the same with a name[11] (all offsets -2).
//
// Button table at 0x4341ec, 4 entries of 0x1e bytes:
//   +0x00 int x, y, w, h   +0x10 int enabled   +0x14 int hover
//   +0x18 int sprite       +0x1c s16 hotspot id
// Images: resource 0xde (P(0x4341dc)), offsets {normal, hover} per button.
#include "game/common.h"

enum { KID_SIZE = 0x329, KID_SIZE_OLD = 0x327 };

static inline u8* Kid(int i) { return (u8*)P(0x4341d8) + i * KID_SIZE; }

// Inlined copies of the current kid's state (globals <-> record).
static void KidFromGlobals(u8* rec) {
    WrU32(rec + 0xd, (u32)D32(0x43980c));
    memcpy(rec + 0x11, DPTR(0x4395f0), 0x86 * 4);
    memcpy(rec + 0x229, DPTR(0x43b414), 10 * 4);
    memcpy(rec + 0x251, DPTR(0x43b43c), 6 * 4);
    WrU32(rec + 0x269, (u32)D32(0x43b460));
    WrU32(rec + 0x26d, (u32)D32(0x43b464));
    memcpy(rec + 0x271, DPTR(0x439838), 0x2b * 4);
    WrU32(rec + 0x31d, (u32)D32(0x43b508));
    WrU32(rec + 0x321, (u32)D32(0x43b50c));
    WrU32(rec + 0x325, (u32)D32(0x43b510));
}
static void KidToGlobals(const u8* rec) {
    D32(0x43980c) = RdS32(rec + 0xd);
    memcpy(DPTR(0x4395f0), rec + 0x11, 0x86 * 4);
    memcpy(DPTR(0x43b414), rec + 0x229, 10 * 4);
    memcpy(DPTR(0x43b43c), rec + 0x251, 6 * 4);
    D32(0x43b460) = RdS32(rec + 0x269);
    D32(0x43b464) = RdS32(rec + 0x26d);
    memcpy(DPTR(0x439838), rec + 0x271, 0x2b * 4);
    D32(0x43b508) = RdS32(rec + 0x31d);
    D32(0x43b50c) = RdS32(rec + 0x321);
    D32(0x43b510) = RdS32(rec + 0x325);
}

REGISTER_CODE(0x4056f7, FUN_004056f7);

// 0x404f80: module entry
void FUN_00404f80() {
    char local_108[256];
    int local_8 = 0;

    FUN_00402c78(0x7f3);
    if (D32(0x431964) == 0) FUN_0040565a();
    FUN_00405773();
    FUN_00405ac3();
    if (-1 < D32(0x4341e8)) KidFromGlobals(Kid(D32(0x4341e8)));
    int sel = -1;
    memset(DPTR(0x43981d), 0, 0xd);
    D32(0x43982c) = 0;
    FUN_00405da5();
    FUN_00405e83();
    FUN_00405f6e();
    Spr_InvalidateAll();
    Spr_Update(0);
    Screen_FlushAll();
    FUN_00405df7();
    FUN_00405e1d();
    Event_Flush();
    Timer_Set(8, 500);
    char* name = DSTR(0x43981d);
    while (local_8 == 0) {
        FUN_00406056();
        FUN_004060d7();
        Spr_Update(1);
        u32 uVar3 = (u32)Event_Poll();
        u16 uVar2 = (u16)uVar3;
        if (FUN_0040394c(uVar2) != 0) continue;
        if ((uVar3 & 0xffff) < 0x80 && (D8(0x438ee9 + (uVar3 & 0xffff)) & 0xde) != 0) {
            int len = (int)strlen(name);
            if (len < 0xc) {
                FUN_00402bad(0, 1);
                name[len] = (char)uVar3;
                FUN_00405f6e();
            } else {
                FUN_00402bad(6, 1);
            }
        } else if (uVar2 == 8) {
            int len = (int)strlen(name);
            if (len < 1) {
                FUN_00402bad(6, 1);
            } else {
                FUN_00402bad(1, 1);
                D8(0x43981c + len) = 0;
                FUN_00405f6e();
            }
        } else if (uVar2 == 0x153) {
            int len = (int)strlen(name);
            if (len == 0) {
                FUN_00402bad(6, 1);
            } else {
                FUN_00402bad(1, 1);
                memset(name, 0, 0xd);
                FUN_00405f6e();
            }
        } else if (uVar2 == 0xd) {
            FUN_004061e4(name);
            int len = (int)strlen(name);
            if (len == 0) {
                FUN_00402bad(6, 1);
                FUN_00405f6e();
            } else {
                FUN_00406230(name);
                int snd = FUN_00402bad(2, 2);
                while (Sound_IsPlaying(snd)) {
                    u32 ev = (u32)Event_Poll();
                    FUN_0040394c((u16)ev);
                }
                local_8 = 1;
            }
        } else if (uVar2 == 0x148) {
            if (D32(0x43982c) < 1) {
                FUN_00402bad(6, 1);
            } else {
                FUN_00402bad(3, 1);
                for (int n = 0; n < 4 && 0 < D32(0x43982c); D32(0x43982c) = D32(0x43982c) - 1) n = n + 1;
                FUN_00405e83();
            }
        } else if (uVar2 == 0x150) {
            if (D32(0x43982c) < D32(0x4341d4) - 5) {
                FUN_00402bad(3, 1);
                for (int n = 0; n < 4 && D32(0x43982c) < D32(0x4341d4) - 5; D32(0x43982c) = D32(0x43982c) + 1)
                    n = n + 1;
                FUN_00405e83();
            } else {
                FUN_00402bad(6, 1);
            }
        } else if (uVar2 == 0x2d0) {
            sel = (g_mouseY - D32(0x434268)) / D32(0x439834) + D32(0x43982c);
            if (sel < D32(0x4341d4)) {
                FUN_00402bad(4, 2);
                memset(name, 0, 0xd);
                strcpy(name, (char*)Kid(sel));
                FUN_00405f6e();
            }
        } else if (uVar2 == 0x2d1) {
            FUN_00402c30(1, 1);
            FUN_004025e3();
        } else if ((uVar2 == 400 || uVar2 == 0x191) && -1 < sel && sel < D32(0x4341d4)) {
            sprintf(local_108, DSTR(0x434298), (char*)Kid(sel));
            if (FUN_00403af5(local_108) != 0) {
                FUN_00402bad(5, 3);
                for (; sel < D32(0x4341d4) - 1; sel = sel + 1) memmove(Kid(sel), Kid(sel + 1), KID_SIZE);
                D32(0x4341d4) = D32(0x4341d4) - 1;
                sel = -1;
                memset(name, 0, 0xd);
                FUN_00405f6e();
                FUN_00405e83();
            }
        }
        Spr_Update(1);
    }
    D32(0x4341e8) = FUN_00406230(name);
    if (D32(0x4341e8) < 0) FUN_00405c9b(name);
    strcpy(DSTR(0x439810), (char*)Kid(D32(0x4341e8)));
    KidToGlobals(Kid(D32(0x4341e8)));
    FUN_00405c09();
    Mem_Free(P(0x4341d8));
    P(0x4341d8) = nullptr;
    FUN_004186a3();
}

// 0x40565a: intro movie to the sign-in screen
void FUN_0040565a() {
    Hotspot_Set(0x20, g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    Gfx_FillRect(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h, 0);
    Screen_FlushAll();
    void* rgb = Res_Load(0xcc);
    Pal_Set((const u8*)rgb, 1);
    Res_Free(rgb);
    Video_Play(DSTR(0x4342b9), D32(0x436ee4), D32(0x436ee8), FUN_004056f7);
    Hotspot_Remove(0x20);
    Pal_FadeToBlack(1);
}

// 0x4056f7: movie abort callback
int FUN_004056f7(int ev) {
    if ((s16)ev == 0x20) return 1;
    if ((s16)ev == 0x1b) App_Quit();
    return 0;
}

// 0x405719: module cleanup
void FUN_00405719() {
    Spr_DeleteAll(0, 0);
    Res_Free(P(0x4341dc));
    P(0x4341dc) = nullptr;
    Mem_Free(P(0x4341e0));
    P(0x4341e0) = nullptr;
    if (P(0x4341d8) != nullptr) {
        FUN_00405c09();
        Mem_Free(P(0x4341d8));
        P(0x4341d8) = nullptr;
    }
}

// 0x405773
void FUN_00405773() {
    Point p;
    u32 w;
    int h;
    P(0x439830) = P(0x4318b0);
    D32(0x439834) = Font_Height(P(0x439830));
    Text_MeasureFont(DSTR(0x4342cf), P(0x439830), &w, &h);
    D32(0x43426c) = (s32)w;
    D32(0x434270) = h * 5;
    FUN_00402b7e(0x80a);
    P(0x4341dc) = Res_Load(0xde);
    int i = 0;
    do {
        Pt_Set(&p, D32(0x4341ec + i * 0x1e), D32(0x4341f0 + i * 0x1e));
        if (D32(0x4341fc + i * 0x1e) == 0) {
            D32(0x434204 + i * 0x1e) = Spr_Create(nullptr, p.x, p.y, 10);
        } else {
            u8* res = (u8*)P(0x4341dc);
            D32(0x434204 + i * 0x1e) = Spr_Create(res + RdS32(res + i * 8), p.x, p.y, 10);
        }
        i = i + 1;
    } while (i < 4);
    P(0x4341e0) = Img_Create(D32(0x43428c), D32(0x434290));
    ((u8*)P(0x4341e0))[8] = 1;
    Gfx_SetTargetImg(P(0x4341e0));
    Gfx_FillRect(D32(0x434284), D32(0x434288), D32(0x43428c), D32(0x434290), 1);
    Gfx_ResetTarget();
    Pt_Set(&p, D32(0x434274), D32(0x434278));
    D32(0x4341e4) = Spr_Create(P(0x4341e0), p.x, p.y, 10);
}

// 0x405901: store the current kid and save the roster
void FUN_00405901() {
    if (-1 < D32(0x4341e8)) {
        FUN_00405ac3();
        KidFromGlobals(Kid(D32(0x4341e8)));
        FUN_00405c09();
        Mem_Free(P(0x4341d8));
        P(0x4341d8) = nullptr;
    }
}

// 0x405a09: convert a 0x327-byte record (name[11]) to the 0x329 layout
void FUN_00405a09(void* param_1, char* param_2) {
    u8* dst = (u8*)param_1;
    memset(dst, 0, 0xd);
    strncpy((char*)dst, param_2, 0xd);
    memcpy(dst + 0xd, param_2 + 0xb, 4);
    memcpy(dst + 0x11, param_2 + 0xf, 0x86 * 4);
    memcpy(dst + 0x229, param_2 + 0x227, 10 * 4);
    memcpy(dst + 0x251, param_2 + 0x24f, 6 * 4);
    memcpy(dst + 0x269, param_2 + 0x267, 4);
    memcpy(dst + 0x26d, param_2 + 0x26b, 4);
    memcpy(dst + 0x271, param_2 + 0x26f, 0x2b * 4);
    memcpy(dst + 0x31d, param_2 + 0x31b, 4);
    memcpy(dst + 0x321, param_2 + 799, 4);
    memcpy(dst + 0x325, param_2 + 0x323, 4);
}

// 0x405ac3: load Roster.Dat
void FUN_00405ac3() {
    bool old = false;
    char local_434[808];
    char path[260];
    s32 recSize;
    strcpy(path, g_exeDir);
    strcat(path, DSTR(0x4342da));
    int h = File_Open(path, FILE_READ);
    if (h < 0) {
        D32(0x4341d4) = 0;
    } else {
        File_Read(h, &recSize, 4);
        if (recSize == KID_SIZE) {
            File_Read(h, DPTR(0x4341d4), 4);
        } else if (recSize == KID_SIZE_OLD) {
            old = true;
            File_Read(h, DPTR(0x4341d4), 4);
        } else {
            D32(0x4341d4) = 0;
        }
        // Port addition: don't trust the count of a damaged file.
        int maxKids = (File_Size(h) - 8) / (old ? KID_SIZE_OLD : KID_SIZE);
        if (D32(0x4341d4) < 0 || D32(0x4341d4) > maxKids) D32(0x4341d4) = maxKids > 0 ? maxKids : 0;
        if (D32(0x4341d4) == 0) File_Close(h);
    }
    u8* buf = (u8*)Mem_Alloc((D32(0x4341d4) + 1) * KID_SIZE);
    P(0x4341d8) = buf;
    if (0 < D32(0x4341d4)) {
        if (old) {
            for (int i = 0; i < D32(0x4341d4); i = i + 1) {
                File_Read(h, local_434, KID_SIZE_OLD);
                FUN_00405a09(buf, local_434);
                buf = buf + KID_SIZE;
            }
        } else {
            File_Read(h, buf, D32(0x4341d4) * KID_SIZE);
        }
        File_Close(h);
    }
}

// 0x405c09: save Roster.Dat
void FUN_00405c09() {
    char path[260];
    strcpy(path, g_exeDir);
    strcat(path, DSTR(0x4342e5));
    int h = File_Open(path, FILE_CREATE);
    s32 recSize = KID_SIZE;
    File_Write(h, &recSize, 4);
    File_Write(h, DPTR(0x4341d4), 4);
    File_Write(h, P(0x4341d8), D32(0x4341d4) * KID_SIZE);
    File_Close(h);
}

// 0x405c9b: append a new kid
void FUN_00405c9b(char* param_1) {
    u8* rec = Kid(D32(0x4341d4));
    memset(rec, 0, 0xd);
    strcpy((char*)rec, param_1);
    WrU32(rec + 0xd, 0);
    memset(rec + 0x11, 0, 0x218);
    WrU32(rec + 0x221, 10);
    memset(rec + 0x229, 0, 0x28);
    memset(rec + 0x251, 0, 0x18);
    memset(rec + 0x269, 0, 8);
    memset(rec + 0x271, 0, 0xac);
    memset(rec + 0x31d, 0, 0xc);
    D32(0x4341e8) = D32(0x4341d4);
    D32(0x4341d4) = D32(0x4341d4) + 1;
}

// 0x405da5
void FUN_00405da5() {
    void* img = Res_Load(0xdc);
    Img_Draw(D32(0x436ee4), D32(0x436ee8), img, 0);
    Res_Free(img);
    Gfx_SaveBg(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
}

// 0x405df7
void FUN_00405df7() {
    void* rgb = Res_Load(0xdd);
    Pal_FadeTo((const u8*)rgb, 1);
    Res_Free(rgb);
}

// 0x405e1d
void FUN_00405e1d() {
    int i = 0;
    do {
        Hotspot_Set(DU16(0x434208 + i * 0x1e), D32(0x4341ec + i * 0x1e), D32(0x4341f0 + i * 0x1e),
                    D32(0x4341f4 + i * 0x1e), D32(0x4341f8 + i * 0x1e));
        i = i + 1;
    } while (i < 4);
    Hotspot_Set(0x2d0, D32(0x434264), D32(0x434268), D32(0x43426c), D32(0x434270));
}

// 0x405e83: draw the visible part of the roster list (5 names)
void FUN_00405e83() {
    Point p;
    Gfx_RestoreBg(D32(0x434264), D32(0x434268), D32(0x43426c), D32(0x434270));
    Pt_Set(&p, D32(0x434264), D32(0x434268));
    for (int i = D32(0x43982c); i < D32(0x4341d4) && i < D32(0x43982c) + 5; i = i + 1) {
        if (RdS32(Kid(i) + 0xd) == 0)
            Text_DrawScreenFont(p.x, p.y, (char*)Kid(i), P(0x439830), 0x24);
        else
            Text_DrawScreenFont(p.x, p.y, (char*)Kid(i), P(0x439830), 0x5b);
        p.y = p.y + D32(0x439834);
    }
    Screen_Flush(D32(0x434264), D32(0x434268), D32(0x43426c), D32(0x434270));
}

// 0x405f6e: redraw the typed name and move the caret
void FUN_00405f6e() {
    Point p;
    int w = 0;
    for (u8* c = DPTR(0x43981d); *c != 0; c = c + 1) w = w + Font_CharWidth((u32)*c, P(0x439830));
    Pt_Set(&p, w + D32(0x434274), D32(0x434278));
    Spr_SetPos(D32(0x4341e4), p.x, p.y);
    Spr_Update(0);
    Gfx_RestoreBg(D32(0x434274), D32(0x434278), D32(0x43427c), D32(0x434280));
    Pt_Set(&p, D32(0x434274), D32(0x434278));
    Text_DrawScreenFont(p.x, p.y, DSTR(0x43981d), P(0x439830), 0x24);
    Screen_Flush(D32(0x434274), D32(0x434278), D32(0x43427c), D32(0x434280));
}

// 0x406056: caret blink
void FUN_00406056() {
    if (Timer_Expired(8)) {
        Timer_Set(8, 500);
        D32(0x434294) = (u32)(D32(0x434294) == 0);
        Gfx_SetTargetImg(P(0x4341e0));
        u8 color = 0x24;
        if (D32(0x434294) == 0) color = 1;
        Gfx_FillRect(D32(0x434284), D32(0x434288), D32(0x43428c), D32(0x434290), color);
        Gfx_ResetTarget();
        Spr_Invalidate(D32(0x4341e4));
    }
}

// 0x4060d7: button enable/hover state
void FUN_004060d7() {
    int my = g_mouseY;
    int mx = g_mouseX;
    D32(0x43421a) = (u32)(0 < D32(0x43982c));
    D32(0x434238) = (u32)(D32(0x43982c) < D32(0x4341d4) - 5);
    int i = 0;
    do {
        if (D32(0x4341fc + i * 0x1e) == 0) {
            Spr_SetImage(D32(0x434204 + i * 0x1e), nullptr);
        } else {
            u8* res = (u8*)P(0x4341dc);
            bool in = Pt_InRect(mx, my, D32(0x4341ec + i * 0x1e), D32(0x4341f0 + i * 0x1e), D32(0x4341f4 + i * 0x1e),
                                D32(0x4341f8 + i * 0x1e));
            if (!in) {
                D32(0x434200 + i * 0x1e) = 0;
                Spr_SetImage(D32(0x434204 + i * 0x1e), res + RdS32(res + i * 8));
            } else {
                D32(0x434200 + i * 0x1e) = 1;
                Spr_SetImage(D32(0x434204 + i * 0x1e), res + RdS32(res + 4 + i * 8));
            }
        }
        i = i + 1;
    } while (i < 4);
}

// 0x4061e4: trim spaces
void FUN_004061e4(char* param_1) {
    int len = (int)strlen(param_1);
    if (0 < len) {
        for (char* p = param_1 + len - 1; param_1 <= p && *p == ' '; p = p - 1) {
            *p = '\0';
            len = len - 1;
        }
        while (*param_1 == ' ') {
            memmove(param_1, param_1 + 1, len - 1);
            len = len - 1;
        }
        param_1[len] = '\0';
    }
}

// 0x406230: roster index of a name (case-insensitive), -1 if absent
int FUN_00406230(char* param_1) {
    int i = 0;
    while (i < D32(0x4341d4) && SDL_strcasecmp(param_1, (char*)Kid(i)) != 0) i = i + 1;
    if (D32(0x4341d4) <= i) i = -1;
    return i;
}

// 0x40658a: Ms. Grunkle pops up and gives the current module's hint.
void FUN_0040658a() {
    Hotspot_Push();
    Hotspot_Clear();
    Hotspot_Set(0x20, g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    D32(0x4342f0) = Spr_Create(nullptr, D32(0x436ee4), D32(0x436ee8), 1000);
    Anim_Load(0x193);
    int snd = FUN_00402c54(8, 1);
    Anim_StartWithSprite(0x193, 0, 0, 1000, D32(0x4342f0));
    Timer_Set(2, 0);
    while (Anim_IsPlaying(0x193)) {
        if (Timer_Expired(2)) {
            Timer_Set(2, 100);
            Anim_UpdateAll();
            Spr_Update(1);
        }
        Event_Poll();
    }
    // (0x40665d: an abort path guarded by a local that is always 0)
    FUN_00402bf5(snd);
    u32 first = (u32)(D32(0x439838 + g_module * 4) == 0);
    D32(0x439838 + g_module * 4) = 1;
    P(0x4342f4) = Res_Load(0x195);
    D32(0x4342f8) = Spr_Create(nullptr, D32(0x434308), D32(0x43430c), 0x3e9);
    P(0x4342fc) = Res_Load(0x196);
    switch (g_module) {
    case 2: FUN_00404be5(first); break;
    case 3: FUN_00409e5c(first); break;
    case 4: FUN_0040840c(first); break;
    case 5: FUN_004146e4(first); break;
    case 6: FUN_00413233(first); break;
    case 7: FUN_0040bd18(first); break;
    case 8: FUN_0040f6dc(first); break;
    case 9: FUN_00411449(first); break;
    case 10: FUN_0040ceb0(first); break;
    case 0xb: FUN_004184de(first); break;
    case 0xe: FUN_0041a5ab(first); break;
    }
    Anim_Free(0x193);
    Res_Free(P(0x4342f4));
    P(0x4342f4) = nullptr;
    Res_Free(P(0x4342fc));
    P(0x4342fc) = nullptr;
    Anim_Load(0x194);
    snd = FUN_00402c54(8, 1);
    Anim_StartWithSprite(0x194, 0, 0, 1000, D32(0x4342f0));
    Timer_Set(2, 0);
    while (Anim_IsPlaying(0x194)) {
        if (Timer_Expired(2)) {
            Timer_Set(2, 100);
            Anim_UpdateAll();
            Spr_Update(1);
        }
        Event_Poll();
    }
    Anim_Free(0x194);
    FUN_00402bf5(snd);
    Hotspot_Pop();
    Spr_Delete(D32(0x4342f0), 1);
    Spr_Update(1);
}
