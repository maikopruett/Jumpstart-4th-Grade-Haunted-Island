// Unit "help": the toolbar help button and the help narration of each
// module. These were unreachable in Ghidra's call graph because the button
// callback (0x406274) is only referenced from the toolbar tables.
#include "game/common.h"

static int Talk(u32 resId) { return FUN_00402e72(resId, P(0x4342f4), D32(0x4342f8), 0, 1); }

// 0x406274: Bosco flies in, explains the current activity, flies out.
void FUN_00406274() {
    Hotspot_Push();
    Hotspot_Clear();
    Hotspot_Set(0x20, g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    D32(0x4342f0) = Spr_Create(nullptr, D32(0x436ee4), D32(0x436ee8), 1000);
    Anim_Load(400);
    int h = FUN_00402c54(8, 1);
    Anim_StartWithSprite(400, 0, 0, 1000, D32(0x4342f0));
    Timer_Set(2, 0);
    while (Anim_IsPlaying(400)) {
        if (Timer_Expired(2)) {
            Timer_Set(2, 100);
            Anim_UpdateAll();
            Spr_Update(1);
        }
        Event_Poll();
    }
    FUN_00402bf5(h);
    int first = D32(0x43988c + g_module * 4) == 0;
    D32(0x43988c + g_module * 4) = 1;
    P(0x4342f4) = Res_Load(0x192);
    D32(0x4342f8) = Spr_Create(nullptr, D32(0x434300), D32(0x434304), 0x3e9);
    switch (g_module) {
        case 2: FUN_00404e27(first); break;
        case 3: FUN_00409f61(first); break;
        case 4: FUN_0040845a(first); break;
        case 5: FUN_00414783(first); break;
        case 6: FUN_00413324(first); break;
        case 7: FUN_0040bd66(first); break;
        case 8: FUN_0040f72a(first); break;
        case 9: FUN_004114bb(first); break;
        case 10: FUN_0040cefe(first); break;
        case 0xb: FUN_00418548(first); break;
        case 0xd: FUN_00419995(first); break;
        case 0xe: FUN_0041a650(first); break;
        case 0xf: FUN_0041a87f(first); break;
        case 0x11: FUN_0041b9df(first); break;
        case 0x12: FUN_0041c5b5(first); break;
    }
    Res_Free(P(0x4342f4));
    P(0x4342f4) = nullptr;
    Anim_Free(400);
    Anim_Load(0x191);
    h = FUN_00402c54(8, 1);
    Anim_StartWithSprite(0x191, 0, 0, 1000, D32(0x4342f0));
    Timer_Set(2, 0);
    while (Anim_IsPlaying(0x191)) {
        if (Timer_Expired(2)) {
            Timer_Set(2, 100);
            Anim_UpdateAll();
            Spr_Update(1);
        }
        Event_Poll();
    }
    Anim_Free(0x191);
    FUN_00402bf5(h);
    Hotspot_Pop();
    Spr_Delete(D32(0x4342f0), 1);
    Spr_Update(1);
}
REGISTER_CODE(0x406274, FUN_00406274);

// 0x404d27: which group of location hints applies here (jump table at
// 0x404d56/0x404da6).
int FUN_00404d27() {
    int loc = D32(0x432154);
    if (0x54 < loc) return 0;
    if ((u32)(loc - 5) >= 0x50) return 0;
    switch (loc) {
        case 13: case 14: case 16: return 1;
        case 9: case 10: case 12: return 2;
        case 37: case 38: case 39: return 3;
        case 81: case 83: case 84: return 4;
        case 18: case 19: case 20: return 5;
        case 26: case 27: case 28: return 6;
        case 29: case 30: case 31: case 65: case 66: case 67: return 7;
        case 21: case 22: case 23: return 8;
        case 32: case 41: case 43: case 44: return 9;
        case 61: case 63: case 64: return 10;
        case 5: case 7: case 8: case 54: case 55: case 56: return 11;
    }
    return 0;
}

// 0x404e27: island help.
void FUN_00404e27(int first) {
    if (D32(0x436d18) != 0) {
        FUN_0041c5b5(first);
        return;
    }
    if (D32(0x432154) == 0x28f || D32(0x432154) == 0x298) {
        Talk(0x1604);
    } else if (first == 0) {
        if (D32(0x432154) < 0x66) {
            int g = FUN_00404d27();
            if (g == 0) {
                if (D32(0x43980c) == 0) {
                    D32(0x43419c) = D32(0x43419c) + 1;
                    if (4 < D32(0x43419c)) D32(0x43419c) = 0;
                    Talk(D32(0x43419c) + 0x816);
                } else {
                    Talk(0x816);
                }
            } else {
                Talk(g + 0x81a);
            }
        } else {
            D32(0x43419c) = D32(0x43419c) + 1;
            if (1 < D32(0x43419c)) D32(0x43419c) = 0;
            Talk(D32(0x43419c) + 0x841);
        }
    } else {
        Talk(0x83f);
    }
}

// Shared shape of most help routines: first visit plays a fixed intro,
// later visits cycle through hints with a per-module counter.
static void Cycle(u32 counter, u32 base, int wrapAbove) {
    Talk(D32(counter) + base);
    D32(counter) = D32(counter) + 1;
    if (wrapAbove < D32(counter)) D32(counter) = 0;
}

void FUN_0040845a(int first) { // 0x40845a (m04)
    if (first == 0) Cycle(0x434610, 0xac8, 1);
    else Talk(0xac7);
}

int FUN_00409f61(int first) { // 0x409f61 (m03)
    if (first == 0) {
        int r = Talk(D32(0x434d38) + 0x8e6);
        D32(0x434d38) = D32(0x434d38) + 1;
        if (2 < D32(0x434d38)) D32(0x434d38) = 0;
        return r;
    }
    if (Talk(0x8e4) != 0) return 1;
    if (Talk(0x8e5) != 0) return 1;
    return 0;
}

int FUN_0040bd66(int first) { // 0x40bd66 (m07)
    if (first == 0) {
        int r = Talk(D32(0x434e58) + 0x15a9);
        D32(0x434e58) = D32(0x434e58) + 1;
        if (2 < D32(0x434e58)) D32(0x434e58) = 0;
        return r;
    }
    if (Talk(0x15a7) != 0) return 1;
    return Talk(0x15a8);
}

int FUN_0040cefe(int first) { // 0x40cefe (m10)
    if (D32(0x43a204) != 1) FUN_0040c485(1);
    int r;
    if (first == 0) {
        r = Talk(D32(0x4351d0) + 0x15c4);
        D32(0x4351d0) = D32(0x4351d0) + 1;
        if (2 < D32(0x4351d0)) D32(0x4351d0) = 0;
    } else {
        r = Talk(0x15c3);
    }
    if (D32(0x43a204) != 1) FUN_0040c524();
    return r;
}

int FUN_0040f72a(int first) { // 0x40f72a (m08)
    if (first == 0) {
        int r = Talk(D32(0x435a80) + 0x15ba);
        D32(0x435a80) = D32(0x435a80) + 1;
        if (0 < D32(0x435a80)) D32(0x435a80) = 0;
        return r;
    }
    if (Talk(0x15b8) != 0) return 1;
    return Talk(0x15b9);
}

void FUN_004114bb(int first) { // 0x4114bb (m09)
    if (first == 0) Cycle(0x435da4, 0x159c, 3);
    else Talk(0x159b);
}

int FUN_00413324(int first) { // 0x413324 (m06)
    if (first == 0) {
        int r = Talk(D32(0x4360b4) + 0x1549);
        D32(0x4360b4) = D32(0x4360b4) + 1;
        if (2 < D32(0x4360b4)) D32(0x4360b4) = 0;
        return r;
    }
    if (Talk(0x1547) != 0) return 1;
    if (Talk(0x1548) != 0) return 1;
    return 0;
}

void FUN_00414783(int first) { // 0x414783 (m05)
    if (first == 0) Cycle(0x43636c, 0x1562, 2);
    else Talk(0x1561);
}

int FUN_00418548(int first) { // 0x418548 (m11)
    if (D32(0x436370) != 0) return Talk(0x15cd);
    if (first == 0) {
        int r = Talk(D32(0x436490) + 0x15d1);
        D32(0x436490) = D32(0x436490) + 1;
        if (2 < D32(0x436490)) D32(0x436490) = 0;
        return r;
    }
    if (Talk(0x15ce) != 0) return 1;
    if (Talk(0x15cf) != 0) return 1;
    return Talk(0x15d0);
}

void FUN_00419995(int first) { // 0x419995 (m13)
    if (first == 0) Talk(D32(0x43b458) == 0 ? 0x844 : 0x846);
    else Talk(D32(0x43b458) == 0 ? 0x843 : 0x845);
}

void FUN_0041a650(int first) { // 0x41a650 (m14)
    if (D32(0x43980c) != 0) Talk(0x16a7);
    else if (D32(0x436610) != 0) Talk(0x8d4);
    else Talk(first == 0 ? 0x8cb : 0x8ca);
}

void FUN_0041a87f(int first) { Talk(first == 0 ? 0x15ff : 0x15fe); } // 0x41a87f (m15)

void FUN_0041b9df(int first) { // 0x41b9df (m17)
    if (D32(0x43b4a4) != 0) Talk(0x15fd);
    else Talk(first == 0 ? 0x15fc : 0x15fb);
}

int FUN_0041c5b5(int first) { // 0x41c5b5 (report)
    if (D32(0x43b514) == 1) return Talk(0x1602);
    if (D32(0x43b514) == 2) return Talk(first == 0 ? 0x1601 : 0x1600);
    return 0;
}
