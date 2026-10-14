// Module 5 (0x4133c8..0x4147ec): notes scroll in from the right along a
// staff/keyboard; the player clicks the matching key (hotspot 0x440) before
// each one reaches the left edge. Afterwards the tune is played back.
//
// Data segment layout used here:
//   0x43aca8..0x43acb8  P  resources (note images, hands, key highlight
//                          sets, key images, miss marker)
//   0x436120            P  current tune resource: u32 count, then
//                          {u8 key, u8 duration} pairs
//   0x43acf0            P  = 0x436120 + 4 (the pairs)
//   0x436124[20]           note sprite ids; 0x43ad08[20] "flying away" flags
//   0x436174[2]            left/right hand sprites; 0x43617c key highlight
//   0x436180[3]            miss marker sprites; 0x43618c playback note
#include "game/common.h"

// Image i of a resource whose header is a table of u32 offsets.
static inline void* ResFrame(void* res, int i) { return (u8*)res + RdS32((u8*)res + i * 4); }

// Tune pair i: key (byte 0) / duration (byte 1).
static inline u8 TuneKey(int i) { return ((u8*)P(0x43acf0))[i * 2]; }

// 0x4133c8: module entry.
void FUN_004133c8() {
    FUN_004135a2();
    D32(0x43aca4) = 0;
    bool done;
    do {
        FUN_00413401();
        D32(0x43aca4) = D32(0x43aca4) + 1;
        done = FUN_004134c9();
    } while (!done);
    Spr_DeleteAll(0, 0);
    FUN_004040f5(1);
}

// 0x413401: one round.
void FUN_00413401() {
    FUN_004139b9();
    FUN_00413ac9();
    FUN_00413db5();
    if (D32(0x43aca4) == 0) {
        FUN_0040658a();
    }
    D32(0x43acf8) = 0;
    D32(0x43acf4) = 0;
    Event_Flush();
    do {
        int ev = Event_Poll();
        FUN_004142e1((u16)ev);
        FUN_00413db5();
        FUN_00413b5b();
        Spr_Update(1);
        if (D32(0x43acc0) <= D32(0x43acf4)) break;
    } while (D32(0x43acf8) < 3);
    FUN_00413d77();
    if (D32(0x43acf4) < D32(0x43acc0)) {
        Timer_Set(0, 1000);
        while (!Timer_Expired(0)) {
            int ev = Event_Poll();
            FUN_0040394c((u16)ev);
        }
    } else {
        FUN_00403e2c(0xfa);
        FUN_00419330(1);
        FUN_00414315();
    }
    FUN_00413565();
    Fx_CloseBlack();
}

// 0x4134c9
bool FUN_004134c9() { return 2 < D32(0x43aca4); }

// 0x4134db: module cleanup.
void FUN_004134db() {
    FUN_00413565();
    Mem_Free(P(0x43aca8));
    P(0x43aca8) = nullptr;
    Mem_Free(P(0x43acac));
    P(0x43acac) = nullptr;
    Mem_Free(P(0x43acb0));
    P(0x43acb0) = nullptr;
    Mem_Free(P(0x43acb4));
    P(0x43acb4) = nullptr;
    Mem_Free(P(0x43acb8));
    P(0x43acb8) = nullptr;
    Spr_DeleteAll(0, 0);
    Hotspot_Clear();
    Cursor_Show(1);
    FUN_00405901();
}

// 0x413565: per-round cleanup.
void FUN_00413565() {
    FUN_00402d81();
    Res_Free(P(0x436120));
    P(0x436120) = nullptr;
    for (int i = 0; i < D32(0x43acf8); i = i + 1) {
        Spr_Delete(D32(0x436180 + i * 4), 0);
    }
}

// 0x4135a2: module init (level parameters, resources, sprites).
void FUN_004135a2() {
    FUN_00403e91();
    u32 level = FUN_004040a2();
    switch (level) {
    case 0:
        D32(0x43acbc) = 0;
        D32(0x43ace4) = 0x41a;
        D32(0x43ace8) = 0x13;
        break;
    case 1:
        D32(0x43acbc) = 1;
        D32(0x43ace4) = 0x42d;
        D32(0x43ace8) = 0x15;
        break;
    case 2:
        D32(0x43acbc) = 2;
        D32(0x43ace4) = 0x41a;
        D32(0x43ace8) = 0x13;
        break;
    case 3:
        D32(0x43acbc) = 2;
        D32(0x43ace4) = 0x442;
        D32(0x43ace8) = 0x1b;
        break;
    case 4:
        D32(0x43acbc) = 3;
        D32(0x43ace4) = 0x442;
        D32(0x43ace8) = 0x1b;
        break;
    case 5:
        D32(0x43acbc) = 3;
        D32(0x43ace4) = 0x41a;
        D32(0x43ace8) = 0x42;
        break;
    }
    D32(0x43acc0) = 0x14;
    u32 speed = FUN_004040cb();
    switch (speed) {
    case 0:
        D32(0x43acdc) = 2;
        D32(0x43ace0) = 0xa0;
        break;
    case 1:
        D32(0x43acdc) = 2;
        D32(0x43ace0) = 0xa0;
        break;
    case 2:
        D32(0x43acdc) = 3;
        D32(0x43ace0) = 0xa0;
        break;
    case 3:
        D32(0x43acdc) = 4;
        D32(0x43ace0) = 0x78;
        break;
    case 4:
        D32(0x43acdc) = 4;
        D32(0x43ace0) = 0x78;
        break;
    case 5:
        D32(0x43acdc) = 5;
        D32(0x43ace0) = 100;
        break;
    }
    if (D32(0x43acbc) == 0) {
        D32(0x43acc4) = 10;
        D32(0x43acc8) = 0xb;
        D32(0x43accc) = 0xcc;
        D32(0x43acd0) = 0xe;
        D32(0x43acd8) = 0x82;
        D32(0x43acd4) = 0x25;
        D32(0x43611c) = 0x1b;
    } else if (D32(0x43acbc) == 1) {
        D32(0x43acc4) = 0;
        D32(0x43acc8) = 10;
        D32(0x43accc) = 0xbe;
        D32(0x43acd0) = 0xe;
        D32(0x43acd8) = 0x88;
        D32(0x43acd4) = 0x29;
        D32(0x43611c) = 0x1b;
    } else if (D32(0x43acbc) == 2) {
        D32(0x43acc4) = 0;
        D32(0x43acc8) = 0x15;
        D32(0x43accc) = 0x106;
        D32(0x43acd0) = 0xb;
        D32(0x43acd8) = 0x82;
        D32(0x43acd4) = 0x22;
        D32(0x43611c) = 0x1b;
    } else if (D32(0x43acbc) == 3) {
        D32(0x43acc4) = 0;
        D32(0x43acc8) = 0x15;
        D32(0x43accc) = 0x106;
        D32(0x43acd0) = 0xb;
        D32(0x43acd8) = 0x5a;
        D32(0x43acd4) = 0x22;
        D32(0x43611c) = 0x1b;
    }
    FUN_00402b7e(0x807);
    if (D32(0x43acbc) == 0 || D32(0x43acbc) == 1) {
        P(0x43aca8) = Res_Load(0x3ee);
    } else {
        P(0x43aca8) = Res_Load(0x3ef);
    }
    P(0x43acac) = Res_Load(0x3ed);
    P(0x43acb0) = Res_Load(D32(0x43acbc) + 0x3f1);
    P(0x43acb4) = Res_Load(D32(0x43acbc) + 0x3f5);
    P(0x43acb8) = Res_Load(0x3f0);
    for (int i = 0; i < D32(0x43acc0); i = i + 1) {
        D32(0x436124 + i * 4) = Spr_Create(nullptr, D32(0x436ee4), D32(0x436ee8), 10);
    }
    int i = 0;
    do {
        D32(0x436174 + i * 4) = Spr_Create(nullptr, D32(0x436ee4), D32(0x436ee8), 0x14);
        i = i + 1;
    } while (i < 2);
    D32(0x43617c) = Spr_Create(nullptr, D32(0x436ee4), D32(0x436ee8), 5);
    D32(0x43618c) = Spr_Create(nullptr, D32(0x436ee4), D32(0x436ee8), 10);
    // 0x43188d is not 4-aligned in the original either (0, 0, 640, 430).
    Hotspot_Set(0x440, D32(0x43188d), D32(0x431891), D32(0x431895), D32(0x431899));
}

// 0x4139b9: round init: picks a tune and lines its notes up off screen.
void FUN_004139b9() {
    int pick = FUN_00403dd5(D32(0x43ace8), (int*)DPTR(0x436344), 8);
    D32(0x43ad58) = (pick + D32(0x43ace4)) - 0x41a;
    P(0x436120) = Res_Load(pick + D32(0x43ace4));
    D32(0x43acec) = RdS32(P(0x436120));
    P(0x43acf0) = (u8*)P(0x436120) + 4;
    int x = g_screenW;
    int h = Img_H(ResFrame(P(0x43aca8), 0));
    for (int i = 0; i < D32(0x43acc0); i = i + 1) {
        Spr_SetPos(D32(0x436124 + i * 4), x,
                   (D32(0x43accc) - ((int)TuneKey(i) - D32(0x43acc4)) * D32(0x43acd0)) - h / 2);
        x = x + D32(0x43ace0);
    }
    memset(DPTR(0x43ad08), 0, 0x50);
    Timer_Set(1, 0);
    D32(0x43acfc) = -1;
    D32(0x43ad04) = 0;
    D32(0x43ad00) = 0;
    Spr_InvalidateAll();
}

// 0x413ac9: draws the background and fades in.
void FUN_00413ac9() {
    void* img = Res_Load(D32(0x43acbc) + 0x3e9);
    Img_Draw(D32(0x436ee4), D32(0x436ee8), img, 0);
    Res_Free(img);
    Gfx_SaveBg(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    FUN_004032e3();
    if (D32(0x43aca4) == 0) {
        Screen_FlushAll();
        void* pal = Res_Load(1000);
        Pal_FadeTo((const u8*)pal, 1);
        Res_Free(pal);
        return;
    }
    Fx_OpenCenter();
}

// 0x413b5b: moves the notes every 100 ms.
void FUN_00413b5b() {
    Point p;
    if (Timer_Expired(1)) {
        Timer_Set(1, 100);
        int i;
        int j;
        for (i = 0; j = D32(0x43acf4), i < D32(0x43acf4); i = i + 1) {
            if (D32(0x43ad08 + i * 4) != 0) {
                // a hit note flies away to the upper left
                Spr_GetPos(D32(0x436124 + i * 4), &p);
                p.x = p.x - 0x14;
                p.y = p.y + -0x14;
                Spr_SetPos(D32(0x436124 + i * 4), p.x, p.y);
                if (p.x < 0 || p.y < 0) {
                    D32(0x43ad08 + i * 4) = 0;
                    Spr_SetImage(D32(0x436124 + i * 4), nullptr);
                } else {
                    Spr_SetImage(D32(0x436124 + i * 4), ResFrame(P(0x43aca8), D32(0x436364) % 2));
                }
            }
        }
        for (; j < D32(0x43acc0); j = j + 1) {
            Spr_GetPos(D32(0x436124 + j * 4), &p);
            p.x = p.x - D32(0x43acdc);
            Spr_SetPos(D32(0x436124 + j * 4), p.x, p.y);
            if (p.x < D32(0x43acd8)) {
                // missed
                FUN_00404068(0);
                FUN_004146d0(-1);
                FUN_00414270(D32(0x43acd8), TuneKey(j));
                Spr_SetImage(D32(0x436124 + j * 4), nullptr);
                D32(0x43acf8) = D32(0x43acf8) + 1;
                D32(0x43acf4) = D32(0x43acf4) + 1;
            } else if ((u32)p.x < (u32)g_screenW) {
                Spr_SetImage(D32(0x436124 + j * 4), ResFrame(P(0x43aca8), D32(0x436364) % 2));
            }
        }
        D32(0x436364) = D32(0x436364) + 1;
    }
}

// 0x413ce2: after a hit, slides the remaining notes left until the next
// one is on screen.
void FUN_00413ce2() {
    Point p;
    if (D32(0x43acf4) < D32(0x43acc0)) {
        Spr_GetPos(D32(0x436124 + D32(0x43acf4) * 4), &p);
        while ((u32)g_screenW < (u32)p.x) {
            for (int i = D32(0x43acf4); i < D32(0x43acc0); i = i + 1) {
                Spr_GetPos(D32(0x436124 + i * 4), &p);
                p.x = p.x - 10;
                Spr_SetPos(D32(0x436124 + i * 4), p.x, p.y);
                Spr_Update(0);
            }
            Spr_GetPos(D32(0x436124 + D32(0x43acf4) * 4), &p);
        }
        Spr_Update(1);
    }
}

// 0x413d77: waits until all hit notes have flown away.
void FUN_00413d77() {
    bool done = false;
    while (!done) {
        FUN_00413b5b();
        Spr_Update(1);
        done = true;
        for (int i = 0; i < D32(0x43acc0); i = i + 1) {
            if (D32(0x43ad08 + i * 4) != 0) {
                done = false;
            }
        }
    }
}

// 0x413db5: mouse tracking (hands over the keys) and click evaluation.
void FUN_00413db5() {
    Point p;
    int mx = g_mouseX;
    int my = g_mouseY;
    if (!Pt_InRect(g_mouseX, g_mouseY, g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h) || 0x1ad < my) {
        Cursor_Show(1);
    } else {
        Cursor_Show(0);
    }
    if (D32(0x43ad04) != 0 && D32(0x43ad00) == 0) {
        D32(0x43ad04) = 0;
        if (D32(0x43acfc) < 0) {
            D32(0x43ad04) = 0;
            return;
        }
        D32(0x43ad00) = 1;
        Timer_Set(2, 200);
        Spr_SetImage(D32(0x43617c), ResFrame(P(0x43acb4), D32(0x43acfc) - D32(0x43acc4)));
        Spr_SetPos(D32(0x43617c), D32(0x436190 + D32(0x43acfc) * 8), D32(0x436194 + D32(0x43acfc) * 8));
        if (D32(0x43acfc) < 10) {
            Spr_SetImage(D32(0x436174), ResFrame(P(0x43acac), 1));
            Pt_Set(&p, mx + -0x32, 0x148);
            Spr_SetPos(D32(0x436174), p.x, p.y);
        } else {
            Spr_SetImage(D32(0x436178), ResFrame(P(0x43acac), 3));
            Pt_Set(&p, mx + -4, 0x148);
            Spr_SetPos(D32(0x436178), p.x, p.y);
        }
        if ((int)TuneKey(D32(0x43acf4)) == D32(0x43acfc)) {
            FUN_004146d0(D32(0x43acfc));
            FUN_00404068(1);
            D32(0x43ad08 + D32(0x43acf4) * 4) = 1;
            D32(0x43acf4) = D32(0x43acf4) + 1;
            FUN_00403e2c(10);
            FUN_00413ce2();
        } else {
            FUN_004146d0(-1);
            FUN_00404068(0);
            Spr_GetPos(D32(0x436124 + D32(0x43acf4) * 4), &p);
            FUN_00414270(p.x, D32(0x43acfc));
            D32(0x43acf8) = D32(0x43acf8) + 1;
        }
    }
    if (D32(0x43ad00) != 0 && Timer_Expired(2)) {
        D32(0x43ad00) = 0;
        if (D32(0x43acfc) < 10) {
            Spr_SetImage(D32(0x436174), ResFrame(P(0x43acac), 0));
            Pt_Set(&p, mx + -0x32, 0x148);
            Spr_SetPos(D32(0x436174), p.x, p.y);
        } else {
            Spr_SetImage(D32(0x436178), ResFrame(P(0x43acac), 2));
            Pt_Set(&p, mx + -4, 0x148);
            Spr_SetPos(D32(0x436178), p.x, p.y);
        }
    }
    if (D32(0x43ad00) == 0 && my < 0x1ae) {
        D32(0x43acfc) = (mx - D32(0x43acd4)) / D32(0x43611c);
        if (D32(0x43acfc) < D32(0x43acc4)) {
            D32(0x43acfc) = D32(0x43acc4);
        } else if (D32(0x43acc4) + D32(0x43acc8) <= D32(0x43acfc)) {
            D32(0x43acfc) = (D32(0x43acc4) + D32(0x43acc8)) - 1;
        }
        if (D32(0x43acc4) <= D32(0x43acfc)) {
            Spr_SetImage(D32(0x43617c), ResFrame(P(0x43acb0), D32(0x43acfc) - D32(0x43acc4)));
            Spr_SetPos(D32(0x43617c), D32(0x436190 + D32(0x43acfc) * 8), D32(0x436194 + D32(0x43acfc) * 8));
        }
        if (D32(0x43acfc) < 10) {
            Spr_SetImage(D32(0x436174), ResFrame(P(0x43acac), 0));
            Pt_Set(&p, mx + -0x32, 0x148);
            Spr_SetPos(D32(0x436174), p.x, p.y);
            if (D32(0x43acbc) == 2 || D32(0x43acbc) == 3) {
                Spr_SetImage(D32(0x436178), ResFrame(P(0x43acac), 2));
                Pt_Set(&p, D32(0x43611c) * 10 + D32(0x43acd4), 0x148);
                Spr_SetPos(D32(0x436178), p.x, p.y);
            }
        } else {
            Spr_SetImage(D32(0x436178), ResFrame(P(0x43acac), 2));
            Pt_Set(&p, mx + -4, 0x148);
            Spr_SetPos(D32(0x436178), p.x, p.y);
            if (D32(0x43acbc) == 2 || D32(0x43acbc) == 3) {
                void* img = ResFrame(P(0x43acac), 0);
                Spr_SetImage(D32(0x436174), img);
                int y = 0x148;
                int w = Img_W(img);
                Pt_Set(&p, (D32(0x43611c) * 10 + D32(0x43acd4)) - w, y);
                Spr_SetPos(D32(0x436174), p.x, p.y);
            }
        }
    }
}

// 0x414270: places a miss marker at x for key.
void FUN_00414270(int x, int key) {
    int h = Img_H(P(0x43acb8));
    Point p;
    Pt_Set(&p, x, (D32(0x43accc) - (key - D32(0x43acc4)) * D32(0x43acd0)) - h / 2);
    D32(0x436180 + D32(0x43acf8) * 4) = Spr_Create(P(0x43acb8), p.x, p.y, D32(0x43acf8) + 2);
}

// 0x4142e1: event handler.
void FUN_004142e1(u16 ev) {
    if (FUN_0040394c(ev) == 0) {
        if (ev == 0x440) {
            D32(0x43ad04) = 1;
            return;
        }
        if (ev == 0xc) {
            FUN_00403f95();
        }
    }
}

// 0x414315: plays the tune back after a successful round.
void FUN_00414315() {
    Spr_SetImage(D32(0x43617c), nullptr);
    if (D32(0x43acbc) == 2 || D32(0x43acbc) == 3) {
        int i = 0;
        do {
            Spr_SetImage(D32(0x436174 + i * 4), ResFrame(P(0x43acac), i * 2));
            i = i + 1;
        } while (i < 2);
    }
    Timer_Set(4, 0);
    for (u32 n = 0; n < (u32)D32(0x43acec); n = n + 1) {
        FUN_00414457(TuneKey(n));
        while (!Timer_Expired(4)) {
            int ev = Event_Poll();
            FUN_0040394c((u16)ev);
        }
        Timer_Set(4, (u32)((u8*)P(0x43acf0))[1 + n * 2] * 0x7d);
        FUN_0041459b(TuneKey(n));
    }
    Spr_SetImage(D32(0x43618c), nullptr);
    FUN_004143ea();
}

// 0x4143ea: closing remarks.
void FUN_004143ea() {
    int r = Rand() % 2;
    FUN_00402e72(r + 0x1565, nullptr, 0, 1, 1);
    FUN_00402e72(D32(0x436238 + D32(0x43ad58) * 4) + 0x1568, nullptr, 0, 1, 1);
    if (D32(0x43aca4) < 2) {
        FUN_00402e72(0x1567, nullptr, 0, 1, 1);
    }
}

// 0x414457: slides the hand to key during playback.
void FUN_00414457(int key) {
    bool right = 9 < key;
    void* img;
    int target;
    if (right) {
        img = ResFrame(P(0x43acac), 2);
        target = key * D32(0x43611c) + D32(0x43acd4);
    } else {
        img = ResFrame(P(0x43acac), 0);
        target = ((key + 1) * D32(0x43611c) + D32(0x43acd4)) - Img_W(img);
    }
    Spr_SetImage(D32(0x436174 + right * 4), img);
    Point p;
    Spr_GetPos(D32(0x436174 + right * 4), &p);
    Timer_Set(3, 0);
    if (target < p.x) {
        do {
            while (!Timer_Expired(3)) {
                int ev = Event_Poll();
                FUN_0040394c((u16)ev);
            }
            Timer_Set(3, 100);
            p.x = p.x + -300;
            if (p.x < target) {
                p.x = target;
            }
            Spr_SetPos(D32(0x436174 + right * 4), p.x, p.y);
            Spr_Update(1);
        } while (target < p.x);
    } else {
        do {
            while (!Timer_Expired(3)) {
                int ev = Event_Poll();
                FUN_0040394c((u16)ev);
            }
            Timer_Set(3, 100);
            p.x = p.x + 300;
            if (target < p.x) {
                p.x = target;
            }
            Spr_SetPos(D32(0x436174 + right * 4), p.x, p.y);
            Spr_Update(1);
        } while (p.x < target);
    }
}

// 0x41459b: presses key during playback.
void FUN_0041459b(int key) {
    Spr_SetImage(D32(0x43617c), ResFrame(P(0x43acb4), key - D32(0x43acc4)));
    Spr_SetPos(D32(0x43617c), D32(0x436190 + key * 8), D32(0x436194 + key * 8));
    void* hand;
    if (9 < key) {
        hand = ResFrame(P(0x43acac), 3);
    } else {
        hand = ResFrame(P(0x43acac), 1);
    }
    Spr_SetImage(D32(0x436174 + (9 < key) * 4), hand);
    void* img = ResFrame(P(0x43aca8), 0);
    Spr_SetImage(D32(0x43618c), img);
    int h = Img_H(img);
    Spr_SetPos(D32(0x43618c), 0x134, (D32(0x43accc) - (key - D32(0x43acc4)) * D32(0x43acd0)) - h / 2);
    Spr_Update(1);
    FUN_004146d0(key);
    Timer_Set(2, 100);
    while (!Timer_Expired(2)) {
        int ev = Event_Poll();
        FUN_0040394c((u16)ev);
    }
    Spr_SetImage(D32(0x43617c), nullptr);
}

// 0x4146d0: note sound (key + 1; -1 plays the "miss" sound 0).
void FUN_004146d0(int key) { FUN_00402bad(key + 1, 6); }

// 0x4146e4: module intro narration (firstVisit = 1 plays the long version).
int FUN_004146e4(int firstVisit) {
    int r = 0;
    if (firstVisit == 0) {
        r = FUN_00402ec2(D32(0x436368) + 0x155e, 0, 1);
        D32(0x436368) = D32(0x436368) + 1;
        if (2 < D32(0x436368)) {
            D32(0x436368) = 0;
        }
    } else {
        if (FUN_00402ec2(0x155b, 0, 1) != 0) return 1;
        if (FUN_00402e72(0x155c, nullptr, 0, 0, 1) != 0) return 1;
        if (FUN_00402ec2(0x155d, 0, 1) != 0) return 1;
    }
    return r;
}
