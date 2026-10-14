// Unit "main": program flow, shared sound/narration helpers, the toolbar,
// the yes/no dialog and level bookkeeping (0x40107c..0x40111f,
// 0x402450..0x404150 in 4G.EXE).
#include "game/common.h"
#include "game/game.h"

// ---------------------------------------------------------------- island hotspot callbacks

// 0x404d0b (m02's address range, reached only from these callbacks).
void FUN_00404d0b(int param_1) {
    D32(0x439808) = 1;
    D32(0x432154) = param_1;
    FUN_00402610();
}

// 0x40107c: island map action; negative values open an info screen.
void FUN_0040107c(int param_1) {
    if (param_1 < 0) {
        FUN_00404d0b(-param_1);
        return;
    }
    D32(0x432154) = param_1;
    FUN_00402610();
}

void FUN_0040109d() { FUN_0040107c(-0x3f); } // 0x40109d
void FUN_004010aa() { FUN_0040107c(0xb); }   // 0x4010aa
void FUN_004010b7() { FUN_0040107c(0x28); }  // 0x4010b7
void FUN_004010c4() { FUN_0040107c(-0x42); } // 0x4010c4
void FUN_004010d1() { FUN_0040107c(0x11); }  // 0x4010d1
void FUN_004010de() { FUN_0040107c(-0x41); } // 0x4010de
void FUN_004010eb() { FUN_0040107c(-0x44); } // 0x4010eb
void FUN_004010f8() { FUN_0040107c(-0x54); } // 0x4010f8
void FUN_00401105() { FUN_0040107c(-0x1e); } // 0x401105
void FUN_00401112() { FUN_0040107c(0x3e); }  // 0x401112
void FUN_0040111f() { FUN_0040107c(6); }     // 0x40111f

REGISTER_CODE(0x40109d, FUN_0040109d);
REGISTER_CODE(0x4010aa, FUN_004010aa);
REGISTER_CODE(0x4010b7, FUN_004010b7);
REGISTER_CODE(0x4010c4, FUN_004010c4);
REGISTER_CODE(0x4010d1, FUN_004010d1);
REGISTER_CODE(0x4010de, FUN_004010de);
REGISTER_CODE(0x4010eb, FUN_004010eb);
REGISTER_CODE(0x4010f8, FUN_004010f8);
REGISTER_CODE(0x401105, FUN_00401105);
REGISTER_CODE(0x401112, FUN_00401112);
REGISTER_CODE(0x40111f, FUN_0040111f);

// ---------------------------------------------------------------- program flow

// 0x402450: module dispatcher. The original used setjmp(0x43959c); every
// longjmp(jb, n) lands here with n as the next module. `entry` is that value.
static int MainDispatch(int entry) {
    g_prevModule = g_module;
    g_module = entry;
    if (entry == 0) {
        FUN_00402674();
        FUN_00403080();
        g_module = 1;
    } else {
        if (D32(0x4318a0) == 0) D32(0x4318a0) = 1;
        else Pal_FadeToBlack(1);
        FUN_004027ae();
        if (g_module != g_prevModule) FUN_004193ec();
    }
    // Debug aid: JS4_JUMP=n enters module n instead of the island once.
    static bool debugJumped = false;
    if (g_module == 2 && !debugJumped) {
        debugJumped = true;
        if (const char* j = getenv("JS4_JUMP")) {
            if (const char* loc = getenv("JS4_LOCATION")) D32(0x432154) = atoi(loc);
            Module_Jump(atoi(j));
        }
    }
    bool quit = false;
    switch (g_module) {
        case 1: FUN_00404f80(); break;
        case 2: FUN_0040415e(); break;
        case 3: FUN_004084c0(); break;
        case 4: FUN_00406890(); break;
        case 5: FUN_004133c8(); break;
        case 6: FUN_00411524(); break;
        case 7: FUN_0040bccb(); break;
        case 8: FUN_0040f697(); break;
        case 9: FUN_0040f7bc(); break;
        case 10: FUN_0040ce63(); break;
        case 0xb: FUN_00418494(); break;
        case 0xd: FUN_004186af(); break;
        case 0xe: FUN_00419a56(); break;
        case 0xf: FUN_0041a6e8(); break;
        case 0x11: FUN_0041a8c8(); break;
        case 0x12: FUN_0041bab2(1); break;
        case 0x13: FUN_0041935e(); break;
        case 0x14: quit = true; break;
        case 0x16: FUN_0040112c(); break;
    }
    if (!quit) FUN_00402610();
    FUN_00402a3a();
    FUN_00402764();
    return 0;
}

int FUN_00402450() {
    int entry = 0;
    for (;;) {
        try {
            return MainDispatch(entry);
        } catch (const ModuleJump& j) {
            entry = j.module ? j.module : 1;
        }
    }
}

int Game_Main() {
    Seg_Init();
    return FUN_00402450();
}

// 0x4025e3: toolbar "quit" button.
void FUN_004025e3() {
    if (FUN_00403af5(DSTR(0x431904)) != 0) App_Quit();
}

void FUN_00402610() { Module_Jump(2); }    // 0x402610
void FUN_00402624() { Module_Jump(0xd); }  // 0x402624
void FUN_00402638() { Module_Jump(0x16); } // 0x402638
void FUN_00402660() { Module_Jump(0x11); } // 0x402660

REGISTER_CODE(0x4025e3, FUN_004025e3);
REGISTER_CODE(0x402610, FUN_00402610);
REGISTER_CODE(0x402624, FUN_00402624);
REGISTER_CODE(0x402638, FUN_00402638);
REGISTER_CODE(0x402660, FUN_00402660);
REGISTER_CODE(0x403f95, FUN_00403f95);

// 0x402674: open the resource archives and load global resources.
void FUN_00402674() {
    static const u32 kArchives[] = {0x431858, 0x431860, 0x43186a, 0x43187a, 0x431871, 0x431884};
    for (u32 name : kArchives) {
        char path[260];
        strcpy(path, g_resDir);
        strcat(path, DSTR(name));
        Res_OpenArchive(path);
    }
    FUN_004029ef();
    FUN_0040289f();
    FUN_00402c03();
    FUN_0040321c();
}

// 0x402764: shutdown.
void FUN_00402764() {
    Gfx_FillRect(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h, 0);
    Screen_FlushAll();
    FUN_00405901();
    FUN_00402a28();
    FUN_004029d7();
    FUN_00402c18();
    Res_CloseAll();
    FUN_00403290();
}

// 0x4027ae: clean up the module we are leaving.
void FUN_004027ae() {
    FUN_00402b28();
    Cursor_Set(2);
    switch (g_prevModule) {
        case 1: FUN_00405719(); return;
        case 2: FUN_00404bdb(); return;
        case 3: FUN_004085f4(); return;
        case 4: FUN_004069e8(); return;
        case 5: FUN_004134db(); return;
        case 6: FUN_004115ee(); return;
        case 7: FUN_0040bcf8(); return;
        case 8: FUN_0040f6bc(); return;
        case 9: FUN_0040fcac(); return;
        case 10: FUN_0040ce90(); return;
        case 0xb: FUN_004184b9(); return;
        case 0xd: FUN_00418a61(); return;
        case 0xe: FUN_0041a136(); return;
        case 0xf: FUN_0041a7df(); return;
        case 0x11: FUN_0041a93d(); break;
        case 0x16: FUN_004011b6(); return;
    }
}

// 0x40289f: font/graphics bank (resource 100) and pointers into it.
void FUN_0040289f() {
    u8* res = (u8*)Res_Load(100);
    P(0x4318a4) = res;
    for (int i = 0; i <= 0x16; i++) P(0x4318a8 + i * 4) = res + RdS32(res + i * 4);
}

void FUN_004029d7() { // 0x4029d7
    Res_Free(P(0x4318a4));
    P(0x4318a4) = nullptr;
}

void FUN_004029ef() { // 0x4029ef
    u16 ids[5];
    memcpy(ids, DPTR(0x431924), sizeof ids);
    Cursor_Load(ids);
    Cursor_Set(2);
}

void FUN_00402a28() { // 0x402a28
    Cursor_Set(0);
    Cursor_Free();
}

// 0x402a3a: credits movie shown on exit.
void FUN_00402a3a() {
    Spr_DeleteAll(0, 0);
    Spr_Update(0);
    Gfx_FillRect(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h, 0);
    Screen_FlushAll();
    void* pal = Res_Load(0xcd);
    Pal_Set((const u8*)pal, 1);
    Res_Free(pal);
    void* snd = Res_Load(0x7e8);
    int h = Sound_Play(snd, 0, 1000);
    Hotspot_Set(0x20, g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    Video_Play(DSTR(0x43192e), D32(0x436ee4), D32(0x436ee8), FUN_00402b12);
    Hotspot_Remove(0x20);
    Sound_Stop(h);
    Res_Free(snd);
}

int FUN_00402b12(int ev) { return (s16)ev == 0x20 ? 1 : 0; } // 0x402b12

// ---------------------------------------------------------------- sounds and narration

void FUN_00402b28() { // 0x402b28
    Sound_StopAllChannels();
    Sound_Reset();
    Res_Free(P(0x431940));
    P(0x431940) = nullptr;
    Res_Free(P(0x431948));
    P(0x431948) = nullptr;
    D32(0x43194c) = 0;
    FUN_00402d81();
    Res_Free(P(0x431950));
    P(0x431950) = nullptr;
}

// 0x402b7e: load the module's sound bank.
void FUN_00402b7e(u32 param_1) {
    Res_Free(P(0x431940));
    P(0x431940) = nullptr;
    P(0x431940) = Res_Load(param_1);
    D32(0x4395cc) = (s32)param_1;
}

static void* BankEntry(u32 slot, int idx) {
    u8* bank = (u8*)P(slot);
    return bank + RdS32(bank + idx * 4);
}

int FUN_00402bad(int param_1, u32 param_2) { return Sound_Play(BankEntry(0x431940, param_1), 0, param_2); }
int FUN_00402bd1(int param_1, u32 param_2) { return Sound_Play(BankEntry(0x431940, param_1), 1, param_2); }
void FUN_00402bf5(int param_1) { Sound_Stop(param_1); }

void FUN_00402c03() { P(0x431944) = Res_Load(0x802); } // 0x402c03

void FUN_00402c18() { // 0x402c18
    Res_Free(P(0x431944));
    P(0x431944) = nullptr;
}

int FUN_00402c30(int param_1, u32 param_2) { return Sound_Play(BankEntry(0x431944, param_1), 0, param_2); }
int FUN_00402c54(int param_1, u32 param_2) { return Sound_Play(BankEntry(0x431944, param_1), 1, param_2); }

// 0x402c78: looping background music.
int FUN_00402c78(u32 param_1) {
    if (D32(0x43194c) != 0) Sound_Stop(D32(0x43194c));
    Res_Free(P(0x431948));
    P(0x431948) = nullptr;
    P(0x431948) = Res_Load(param_1);
    D32(0x43194c) = Sound_Play(P(0x431948), 1, 1000);
    return D32(0x43194c);
}

// 0x402cd2: start a talking sound {u32 kwavOffset, u32 framesOffset, ...}
// with optional lip sync on `sprite` using frames from `ang`.
int FUN_00402cd2(void* talk, void* ang, int sprite) {
    FUN_00402d81();
    u8* t = (u8*)talk;
    u32 hdr = RdU32(t);
    D32(0x431954) = Sound_Play(t + hdr, 0, 999);
    if (1 < (hdr >> 2) - 1 && ang != nullptr && sprite != 0) {
        D32(0x43195c) = sprite;
        D32(0x431958) = LipSync_Add(ang, t + RdU32(t + 4), sprite);
        Timer_Set(10, 100);
    }
    return D32(0x431954);
}

int FUN_00402d42(u32 resId, void* ang, int sprite) { // 0x402d42
    FUN_00402d81();
    Res_Free(P(0x431950));
    P(0x431950) = nullptr;
    P(0x431950) = Res_Load(resId);
    return FUN_00402cd2(P(0x431950), ang, sprite);
}

void FUN_00402d81() { // 0x402d81
    if (D32(0x431954) != 0) {
        Sound_Stop(D32(0x431954));
        D32(0x431954) = 0;
    }
    if (D32(0x431958) != 0) {
        LipSync_Remove(D32(0x431958));
        D32(0x431958) = 0;
        Spr_SetImage(D32(0x43195c), nullptr);
    }
}

// 0x402dcf: play a talking sound to completion. Returns 1 if skipped.
int FUN_00402dcf(void* talk, void* ang, int sprite, int toolbar, int abortable) {
    int h = FUN_00402cd2(talk, ang, sprite);
    for (;;) {
        if (!Sound_IsPlaying(h)) {
            LipSync_Remove(D32(0x431958));
            D32(0x431958) = 0;
            Spr_SetImage(D32(0x43195c), nullptr);
            return 0;
        }
        u32 ev = Event_Poll();
        if (toolbar != 0) FUN_0040394c((u16)ev);
        if ((u16)ev == 0x20 && abortable != 0) break;
        if (Timer_Expired(10)) {
            LipSync_Update();
            Timer_Set(10, 100);
        }
        Spr_Update(1);
    }
    FUN_00402d81();
    return 1;
}

int FUN_00402e72(u32 resId, void* ang, int sprite, int toolbar, int abortable) { // 0x402e72
    FUN_00402d81();
    Res_Free(P(0x431950));
    P(0x431950) = nullptr;
    P(0x431950) = Res_Load(resId);
    int r = FUN_00402dcf(P(0x431950), ang, sprite, toolbar, abortable);
    Res_Free(P(0x431950));
    P(0x431950) = nullptr;
    return r;
}

// 0x402ec2: Ms. Grunkle narration with lip sync and eye blinks.
int FUN_00402ec2(u32 resId, int toolbar, int abortable) {
    int aborted = 0;
    FUN_00402d81();
    Res_Free(P(0x431950));
    P(0x431950) = nullptr;
    P(0x431950) = Res_Load(resId);
    Timer_Set(0xb, 0xce4);
    int h = FUN_00402cd2(P(0x431950), P(0x4342f4), D32(0x4342f8));
    for (;;) {
        if (!Sound_IsPlaying(h)) break;
        u32 ev = Event_Poll();
        if (toolbar != 0) FUN_0040394c((u16)ev);
        if ((u16)ev == 0x20 && abortable != 0) {
            FUN_00402d81();
            aborted = 1;
            break;
        }
        if (Timer_Expired(10)) {
            LipSync_Update();
            Timer_Set(10, 100);
        }
        if (Timer_Expired(0xb)) {
            if (D32(0x431960) == 0) {
                D32(0x4395c4) = Rand() % 3;
                bool same = D32(0x4395c4) == D32(0x4395c8);
                D32(0x4395c8) = D32(0x4395c4);
                if (same) D32(0x431960) = 1;
            }
            if (D32(0x431960) < 2) {
                Point pt;
                const void* img = Ang_FrameImageAt(P(0x4342fc), D32(0x4395c4) * 2 + D32(0x431960), &pt);
                Spr_SetImage(D32(0x4342f0), img);
                Spr_SetPos(D32(0x4342f0), pt.x, pt.y);
                Timer_Set(0xb, 100);
                D32(0x431960) = D32(0x431960) + 1;
            } else {
                Timer_Set(0xb, 0xce4);
                D32(0x431960) = 0;
            }
        }
        Spr_Update(1);
    }
    LipSync_Remove(D32(0x431958));
    D32(0x431958) = 0;
    Spr_SetImage(D32(0x43195c), nullptr);
    Res_Free(P(0x431950));
    P(0x431950) = nullptr;
    return aborted;
}

// ---------------------------------------------------------------- startup logos

// 0x403080: Knowledge Adventure / JumpStart logos and school intro.
void FUN_00403080() {
    u32 pals[4], snds[4];
    memcpy(pals, DPTR(0x431968), sizeof pals);
    memcpy(snds, DPTR(0x431978), sizeof snds);
    void* sndRes = nullptr;
    int h = 0;
    Hotspot_Set(0x20, g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    for (int i = 0; i < 4 && D32(0x431964) == 0; i++) {
        Gfx_FillRect(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h, 0);
        Screen_FlushAll();
        void* pal = Res_Load(pals[i]);
        Pal_Set((const u8*)pal, 1);
        Res_Free(pal);
        if (snds[i] != 0) {
            sndRes = Res_Load(snds[i]);
            h = Sound_Play(sndRes, 0, 1000);
        }
        char path[260];
        sprintf(path, DSTR(0x4319b4), (char*)P(0x431988 + i * 4));
        D32(0x431964) = Video_Play(path, D32(0x436ee4), D32(0x436ee8), FUN_004031f7);
        if (i < 2) D32(0x431964) = 0;
        if (snds[i] != 0) {
            Sound_Stop(h);
            Res_Free(sndRes);
            sndRes = nullptr;
        }
    }
    Hotspot_Remove(0x20);
    Pal_FadeToBlack(1);
}

int FUN_004031f7(int ev) { // 0x4031f7
    if ((s16)ev == 0x20) return 1;
    if ((s16)ev == 0x1b) App_Quit();
    return 0;
}

// 0x40321c: toolbar graphics and the score display image.
void FUN_0040321c() {
    P(0x4319c4) = Res_Load(0x78);
    u32 w;
    int h;
    Text_MeasureFont(DSTR(0x43210c), P(0x4318b0), &w, &h);
    P(0x4319c8) = Img_Create((int)w, h);
    D32(0x4395e0) = (g_screenW - (int)w) - 0x10;
    D32(0x4395e4) = 0x10;
    D32(0x4395e8) = (int)w;
    D32(0x4395ec) = h;
    P(0x4319cc) = Res_Load(0x7a);
}

void FUN_00403290() { // 0x403290
    Res_Free(P(0x4319c4));
    P(0x4319c4) = nullptr;
    Mem_Free(P(0x4319c8));
    P(0x4319c8) = nullptr;
    Res_Free(P(0x4319cc));
    P(0x4319cc) = nullptr;
    Res_Free(P(0x4319d0));
    P(0x4319d0) = nullptr;
}

// ---------------------------------------------------------------- toolbar
// The current toolbar table (D32(0x4395d4) holds its address in the data
// segment) has 10 dwords per button: x, y, w, h, image, hover image,
// enabled, hovered, callback (code address), sound.

static u32 Btn(int i, int field) { return DU32(0x4395d4) + i * 0x28 + field * 4; }

static void* ToolbarImage(int idx) {
    u8* bank = (u8*)P(0x4319c4);
    return bank + RdS32(bank + idx * 4);
}

// 0x4032e3: lay out the toolbar for the current module.
void FUN_004032e3() {
    D32(0x4395d8) = 1;
    D32(0x4395dc) = 0;
    switch (g_module) {
        default: DU32(0x4395d4) = 0x431d64; break;
        case 1:
            D32(0x4319d4) = 0;
            D32(0x4395d8) = 0;
            D32(0x4395dc) = 0;
            return;
        case 2:
        case 0x12:
            D32(0x4395dc) = 1;
            if (D32(0x436d18) == 0) DU32(0x4395d4) = D32(0x43980c) == 0 ? 0x431a1c : 0x431b34;
            else DU32(0x4395d4) = 0x431c4c;
            break;
        case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 0xb:
            DU32(0x4395d4) = 0x431f94;
            break;
        case 0x16: DU32(0x4395d4) = 0x431e7c; break;
    }
    D32(0x4319d4) = 7;
    for (int i = 0; i < D32(0x4319d4); i++) {
        void* img = ToolbarImage(D32(Btn(i, 4)));
        int cx = (int)(((u64)(i * 2 + 1) * (u64)(u32)g_screenW & 0xffffffffu) / (u64)(u32)(D32(0x4319d4) * 2));
        int w = Img_W(img), h = Img_H(img);
        D32(Btn(i, 0)) = cx - w / 2;
        D32(Btn(i, 1)) = g_screenH - h;
        D32(Btn(i, 2)) = w;
        D32(Btn(i, 3)) = h;
    }
    for (int i = 0; i < 8; i++) Spr_Delete(D32(0x4319d8 + i * 4), 1);
    Spr_Delete(D32(0x4319f8), 1);
    Spr_Delete(D32(0x431a00), 1);
    Spr_Delete(D32(0x4319fc), 1);
    for (int i = 0; i < D32(0x4319d4); i++)
        D32(0x4319d8 + i * 4) = Spr_Create(nullptr, D32(Btn(i, 0)), D32(Btn(i, 1)), 1);
    if (D32(0x4395d8) != 0) {
        D32(0x4319f8) = Spr_Create(P(0x4319c8), D32(0x4395e0), D32(0x4395e4), 1);
        int dy = (int)((u32)D32(0x4395ec) - 0x32) / 2;
        int dx = (int)((u32)D32(0x4395e8) - 0x32) / 2;
        D32(0x431a00) = Spr_Create(nullptr, dx + D32(0x4395e0), dy + D32(0x4395e4), 2);
    }
    if (D32(0x4395dc) != 0) D32(0x4319fc) = Spr_Create(nullptr, D32(0x431a04), D32(0x431a08), 1);
    FUN_00403606();
    Spr_Update(0);
    FUN_004038b9();
}

void FUN_00403606() { // 0x403606
    for (int i = 0; i < D32(0x4319d4); i++) FUN_0040363d(i);
    if (D32(0x4395d8) != 0) FUN_00403757();
    if (D32(0x4395dc) != 0) FUN_0040388b();
}

// 0x40363d: (re)draw toolbar button i; a button whose normal image index is 0
// (the last entry of every table) gets the current level printed into it.
void FUN_0040363d(int param_1) {
    int idx = D32(Btn(param_1, 7)) != 0 ? D32(Btn(param_1, 5)) : D32(Btn(param_1, 4));
    void* img = ToolbarImage(idx);
    if (D32(Btn(param_1, 4)) == 0) {
        Gfx_SetTargetImg(img);
        Gfx_FillRect(D32(0x431a0c), D32(0x431a10), D32(0x431a14), D32(0x431a18), Img_Key(img));
        char text[120];
        sprintf(text, DSTR(0x432116), D32(0x4395f4));
        int color = D32(Btn(param_1, 7)) != 0 ? 0x1c : 0x1b;
        Text_DrawBoxFont(D32(0x431a0c), D32(0x431a10), D32(0x431a14), D32(0x431a18), text, P(0x4318a8), color);
        Gfx_ResetTarget();
        Spr_Invalidate(D32(0x4319d8 + param_1 * 4));
    }
    Spr_SetImage(D32(0x4319d8 + param_1 * 4), img);
}

// 0x403757: render the score into its image.
void FUN_00403757() {
    void* img = P(0x4319c8);
    Gfx_SetTargetImg(img);
    Gfx_FillRect(0, 0, D32(0x4395e8), D32(0x4395ec), Img_Key(img));
    char text[16];
    FUN_0040381a((u32)D32(0x4395f0), text);
    u32 w;
    Text_MeasureFont(text, P(0x4318ac), &w, nullptr);
    Text_DrawScreenFont(D32(0x4395e8) - (int)w, 0, text, P(0x4318ac), 0x33);
    Gfx_ResetTarget();
    Spr_Invalidate(D32(0x4319f8));
}

// 0x40381a: format a number with thousands separators.
void FUN_0040381a(u32 value, char* out) {
    char tmp[16];
    u32 n = 0;
    do {
        if ((n & 3) == 3) {
            tmp[n] = ',';
        } else {
            tmp[n] = (char)(value % 10) + '0';
            value /= 10;
        }
        n++;
    } while (value != 0 && (int)n < 0x10);
    int o = 0;
    while ((int)(n - 1) > -1) {
        out[o++] = tmp[n - 1];
        n--;
    }
    out[o] = 0;
}

void FUN_0040388b() { // 0x40388b: lives indicator
    u8* bank = (u8*)P(0x4319cc);
    Spr_SetImage(D32(0x4319fc), bank + RdS32(bank + (10 - D32(0x439800)) * 4));
}

void FUN_004038b9() { // 0x4038b9
    for (int i = 0; i < D32(0x4319d4); i++)
        Hotspot_Set((s16)(i + 0x280), D32(Btn(i, 0)), D32(Btn(i, 1)), D32(Btn(i, 2)), D32(Btn(i, 3)));
}

// 0x4038f4: a new level was reached; flash the badge.
void FUN_004038f4() {
    FUN_00402c30(9, 10);
    P(0x4319d0) = Res_Load(D32(0x4395f4) + 0x7c);
    D32(0x4395d0) = 5;
    Timer_Set(0xe, 500);
    Spr_SetImage(D32(0x431a00), P(0x4319d0));
    FUN_00403606();
}

// 0x40394c: toolbar hover and click handling for event ev.
int FUN_0040394c(u16 ev) {
    int my = g_mouseY, mx = g_mouseX;
    for (int i = 0; i < D32(0x4319d4); i++) {
        if (!Pt_InRect(mx, my, D32(Btn(i, 0)), D32(Btn(i, 1)), D32(Btn(i, 2)), D32(Btn(i, 3)))) {
            if (D32(Btn(i, 7)) != 0) {
                D32(Btn(i, 7)) = 0;
                FUN_0040363d(i);
                Spr_Update(1);
            }
        } else if (D32(Btn(i, 7)) == 0 && D32(Btn(i, 6)) != 0) {
            D32(Btn(i, 7)) = 1;
            FUN_0040363d(i);
            Spr_Update(1);
        }
    }
    if (0 < D32(0x4395d0) && Timer_Expired(0xe)) {
        Timer_Set(0xe, 500);
        D32(0x4395d0) = D32(0x4395d0) - 1;
        void* img = P(0x4319d0);
        if ((D32(0x4395d0) & 1) == 0) img = nullptr;
        Spr_SetImage(D32(0x431a00), img);
        Spr_Update(1);
        if (D32(0x4395d0) == 0) {
            Res_Free(P(0x4319d0));
            P(0x4319d0) = nullptr;
        }
    }
    if (ev < 0x280 || D32(0x4319d4) + 0x280 <= (int)ev) return 0;
    int i = ev - 0x280;
    if (D32(Btn(i, 6)) != 0 && D32(Btn(i, 8)) != 0) {
        FUN_00402d81();
        int h = FUN_00402c30(D32(Btn(i, 9)), 1);
        while (Sound_IsPlaying(h)) Event_Poll();
        CODE(D32(Btn(i, 8)))();
    }
    return 1;
}

// 0x403af5: two-button (yes/no) confirmation box. Returns the button index.
u16 FUN_00403af5(const char* text) {
    int bgX = D32(0x4320ac), bgY = D32(0x4320b0);
    int save[4], btnPos[4], btnRect[8], textRect[4];
    memcpy(save, DPTR(0x4320b4), sizeof save);
    memcpy(btnPos, DPTR(0x4320c4), sizeof btnPos);
    memcpy(btnRect, DPTR(0x4320d4), sizeof btnRect);
    memcpy(textRect, DPTR(0x4320f4), sizeof textRect);
    int sounds[2] = {D32(0x432104), D32(0x432108)};
    int done = 0;
    u8* res = (u8*)Res_Load(0x65);
    void* bg = res + RdS32(res);
    void* normal[2];
    void* hover[2];
    int hovered[2];
    for (int i = 0; i < 2; i++) {
        normal[i] = res + RdS32(res + (i * 2 + 1) * 4);
        hover[i] = res + RdS32(res + (i * 2 + 2) * 4);
        Hotspot_Set((s16)(i + 0x290), btnRect[i * 4], btnRect[i * 4 + 1], btnRect[i * 4 + 2], btnRect[i * 4 + 3]);
        hovered[i] = 0;
    }
    void* saved = Mem_Alloc(Img_RawSize(save[0], save[1], save[2], save[3]));
    Img_Capture(save[0], save[1], save[2], save[3], saved);
    Img_Draw(bgX, bgY, bg, 1);
    Text_DrawBoxFont(textRect[0], textRect[1], textRect[2], textRect[3], text, P(0x4318b8), 0x33);
    Screen_Flush(save[0], save[1], save[2], save[3]);
    int prevCursor = Cursor_Show(true);
    u16 sel = 0;
    while (done == 0) {
        int mx = g_mouseX, my = g_mouseY;
        for (int i = 0; i < 2; i++) {
            const int* r = &btnRect[i * 4];
            if (!Pt_InRect(mx, my, r[0], r[1], r[2], r[3])) {
                if (hovered[i] != 0) {
                    hovered[i] = 0;
                    Img_Draw(btnPos[i * 2], btnPos[i * 2 + 1], normal[i], 1);
                    Screen_Flush(r[0], r[1], r[2], r[3]);
                }
            } else if (hovered[i] == 0) {
                hovered[i] = 1;
                Img_Draw(btnPos[i * 2], btnPos[i * 2 + 1], hover[i], 1);
                Screen_Flush(r[0], r[1], r[2], r[3]);
            }
        }
        u32 ev = Event_Poll();
        sel = (u16)((s16)ev - 0x290);
        if (sel < 2) done = 1;
    }
    int h = FUN_00402c30(sounds[sel], 1);
    while (Sound_IsPlaying(h)) Event_Poll();
    for (int i = 0; i < 2; i++) Hotspot_Remove((s16)(i + 0x290));
    Res_Free(res);
    Img_DrawOpaque(bgX, bgY, saved);
    Screen_Flush(save[0], save[1], save[2], save[3]);
    Cursor_Show(prevCursor != 0);
    Mem_Free(saved);
    return sel;
}

// ---------------------------------------------------------------- scores and levels

// 0x403dd5: random number in [0, range) not among the last `count` picks.
int FUN_00403dd5(int range, int* history, int count) {
    int v;
    bool fresh;
    do {
        v = range == 0 ? 0 : Rand() % range;
        fresh = true;
        for (int i = 0; i < count; i++)
            if (v == history[i]) fresh = false;
    } while (!fresh);
    for (int i = 0; i < count - 1; i++) history[i] = history[i + 1];
    history[count - 1] = v;
    return v;
}

// 0x403e2c: add points to the score.
void FUN_00403e2c(int param_1) {
    u32 before = DU32(0x4395f0) / 0x9c4;
    DU32(0x4395f0) = DU32(0x4395f0) + param_1;
    if (before < DU32(0x4395f0) / 0x9c4) D32(0x43b50c) = 0;
    FUN_00403757();
    u32 level = DU32(0x4395f0) / 5000;
    if (DU32(0x4395f4) < level && level < 0x1a) {
        DU32(0x4395f4) = level;
        FUN_004038f4();
    }
}

// 0x403e91: automatic difficulty advance for the current activity.
void FUN_00403e91() {
    int m = g_module - 3;
    if (D32(0x43211c) >= 0) {
        D32(0x4395f8 + m * 4) = D32(0x43211c);
        D32(0x43211c) = -1;
        return;
    }
    u32 maxLevel = m == 2 ? 5 : m == 6 ? 2 : 4;
    u32 lvl = DU32(0x4395f8 + m * 4);
    while (lvl < maxLevel) {
        u32 tries = DU32(0x439620 + m * 0x30 + lvl * 8);
        u32 right = DU32(0x439624 + m * 0x30 + lvl * 8);
        if (m != 6 && tries >= 10 && right >= (tries * 80) / 100) {
            lvl++;
            DU32(0x4395f8 + m * 4) = lvl;
            continue;
        }
        if (m != 6) return;
        if (D32(0x435ac4 + lvl * 4) > (s32)D8(0x439804)) return;
        if (right < (tries * 80) / 100) return;
        lvl++;
        DU32(0x4395f8 + m * 4) = lvl;
        D8(0x439804) = 0;
        D8(0x439805) = 0;
    }
}

// 0x403f95: toolbar "level" button: offer the next difficulty level.
void FUN_00403f95() {
    int m = g_module - 3;
    int maxLevel = g_module == 5 ? 5 : m == 6 ? 2 : 4;
    int next = D32(0x4395f8 + m * 4) + 1;
    if (maxLevel < next) next = 0;
    char text[256];
    sprintf(text, DSTR(0x432120), D32(0x4395f8 + m * 4) + 1, next + 1);
    if (FUN_00403af5(text) != 0) {
        D32(0x4395f8 + m * 4) = next;
        int lvl = D32(0x4395f8 + m * 4);
        D32(0x439624 + lvl * 8 + m * 0x30) = 0;
        D32(0x439620 + lvl * 8 + m * 0x30) = 0;
        if (m == 6) {
            D8(0x439804) = 0;
            D8(0x439805) = 0;
        }
        Module_Jump(g_module);
    }
}

// 0x404068: record an attempt (and whether it was correct).
void FUN_00404068(int param_1) {
    int m = g_module - 3;
    int lvl = D32(0x4395f8 + m * 4);
    D32(0x439620 + lvl * 8 + m * 0x30) = D32(0x439620 + lvl * 8 + m * 0x30) + 1;
    if (param_1 != 0) D32(0x439624 + lvl * 8 + m * 0x30) = D32(0x439624 + lvl * 8 + m * 0x30) + 1;
}

u32 FUN_004040a2() { // 0x4040a2
    u32 v = DU32(0x4395f8 + (g_module - 3) * 4);
    if (1 < (u32)(g_module - 5) && (u32)(g_module - 5) != 4) v >>= 1;
    return v;
}

u32 FUN_004040cb() { // 0x4040cb
    u32 v = DU32(0x4395f8 + (g_module - 3) * 4);
    if (1 < (u32)(g_module - 5) && (u32)(g_module - 5) != 4) v = (v + 1) >> 1;
    return v;
}

// 0x4040f5: change the number of lives (ghost meter).
void FUN_004040f5(int param_1) {
    D32(0x439800) = D32(0x439800) + param_1;
    if (D32(0x439800) < 0xb) {
        if (D32(0x439800) < 0) D32(0x439800) = 0;
    } else {
        D32(0x439800) = 10;
    }
    FUN_0040388b();
    Spr_Update(1);
    if (D32(0x439800) == 0) {
        D32(0x432154) = 0x28f;
        FUN_00402610();
    }
}
