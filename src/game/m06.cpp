// Module 6: the potion lab (0x411524..0x4133c8).
//
// The player pours measured amounts from three flasks (values in
// DAT_0043ac80[3], hundredths) into three beakers (DAT_0043ac98[3]) until each
// beaker holds the target amount of its potion (DAT_0043ac8c[3]). Three
// rounds (DAT_0043ac58) per visit.
//
// Data segment layout used here:
//   0x435dec..0x435df8  P  resources 0x386..0x389 (flask cursor images, ...)
//   0x435dfc[3]         P  resources 0x38a..0x38c (per-round animations)
//   0x435e08            P  talking animation (0x390 / 0x391)
//   0x435e0c[3]         P  resources 0x38d..0x38f (beaker fizz animations)
//   0x435e18[3][4][2]   P  decompressed flask images (Mem_Alloc'd)
//   0x435e78[3]         P  beaker label images (Img_Create)
//   0x435e84            P  resource 0x392 (timer clock frames)
//   0x435e88[4]         P  potion name strings
//   0x435e98..0x435ec8     sprite ids
#include "game/common.h"

// Image i of a resource whose header is a table of u32 offsets.
static inline void* ResFrame(void* res, int i) { return (u8*)res + RdS32((u8*)res + i * 4); }

// 0x411524: module entry.
void FUN_00411524() {
    FUN_00411733();
    D32(0x43ac58) = 0;
    bool done;
    do {
        FUN_00411551();
        D32(0x43ac58) = D32(0x43ac58) + 1;
        done = FUN_004115dc();
    } while (!done);
    FUN_004040f5(1);
}

// 0x411551: one round.
void FUN_00411551() {
    FUN_0041191f();
    FUN_004124bf();
    if (D32(0x43ac58) == 0) {
        FUN_0040658a();
    }
    Timer_Set(8, D32(0x43ac78));
    do {
        int ev = Event_Poll();
        FUN_0041297c((u16)ev);
        FUN_004127e0();
        FUN_00412efc();
        LipSync_Update();
        Spr_Update(1);
    } while (D32(0x43ac6c) == 0);
    int bonus = D32(0x43ac70) * -0x32 + 0x15e;
    if (0 < bonus) {
        FUN_00403e2c(bonus);
    }
    FUN_00419330(1);
    FUN_00413069();
    FUN_004116b4();
    Fx_CloseBlack();
}

// 0x4115dc
bool FUN_004115dc() { return 2 < D32(0x43ac58); }

// 0x4115ee: module cleanup.
void FUN_004115ee() {
    FUN_004116b4();
    Res_Free(P(0x435e08));
    P(0x435e08) = nullptr;
    Res_Free(P(0x435dec));
    P(0x435dec) = nullptr;
    Res_Free(P(0x435df0));
    P(0x435df0) = nullptr;
    Res_Free(P(0x435df4));
    P(0x435df4) = nullptr;
    Res_Free(P(0x435df8));
    P(0x435df8) = nullptr;
    int i = 0;
    do {
        Res_Free(P(0x435dfc + i * 4));
        P(0x435dfc + i * 4) = nullptr;
        i = i + 1;
    } while (i < 3);
    i = 0;
    do {
        Res_Free(P(0x435e0c + i * 4));
        P(0x435e0c + i * 4) = nullptr;
        i = i + 1;
    } while (i < 3);
    Res_Free(P(0x435e84));
    P(0x435e84) = nullptr;
    Hotspot_Clear();
    FUN_00405901();
}

// 0x4116b4: per-round cleanup.
void FUN_004116b4() {
    Spr_DeleteAll(0, 0);
    int k = 0;
    do {
        int j = 0;
        do {
            int i = 0;
            do {
                u32 slot = 0x435e18 + (k * 8 + j * 2 + i) * 4;
                Mem_Free(P(slot));
                P(slot) = nullptr;
                i = i + 1;
            } while (i < 2);
            j = j + 1;
        } while (j < 4);
        k = k + 1;
    } while (k < 3);
    k = 0;
    do {
        Mem_Free(P(0x435e78 + k * 4));
        P(0x435e78 + k * 4) = nullptr;
        k = k + 1;
    } while (k < 3);
    Cursor_Show(1);
}

// 0x411733: module init (resources, hotspots).
void FUN_00411733() {
    FUN_00402c78(0x7f2);
    FUN_00403e91();
    FUN_00402b7e(0x806);
    P(0x435dec) = Res_Load(0x386);
    P(0x435df0) = Res_Load(0x387);
    P(0x435df4) = Res_Load(0x388);
    P(0x435df8) = Res_Load(0x389);
    int i = 0;
    do {
        P(0x435dfc + i * 4) = Res_Load(i + 0x38a);
        i = i + 1;
    } while (i < 3);
    P(0x435e08) = Res_Load(0x390);
    i = 0;
    do {
        P(0x435e0c + i * 4) = Res_Load(i + 0x38d);
        i = i + 1;
    } while (i < 3);
    P(0x435e84) = Res_Load(0x392);
    Hotspot_Set(0x479, D32(0x435ee8), D32(0x435eec), D32(0x435ef0), D32(0x435ef4));
    i = 0;
    do {
        Hotspot_Set((s16)i + 0x480, D32(0x435ef8 + i * 16), D32(0x435efc + i * 16), D32(0x435f00 + i * 16),
                    D32(0x435f04 + i * 16));
        i = i + 1;
    } while (i < 3);
    Hotspot_Set(0x490, D32(0x435f40), D32(0x435f44), D32(0x435f48), D32(0x435f4c));
    Hotspot_Set(0x491, D32(0x435f50), D32(0x435f54), D32(0x435f58), D32(0x435f5c));
    i = 0;
    do {
        Hotspot_Set((s16)i + 0x4a0, D32(0x435f60 + i * 16), D32(0x435f64 + i * 16), D32(0x435f68 + i * 16),
                    D32(0x435f6c + i * 16));
        Hotspot_Set((s16)i + 0x4b0, D32(0x435f90 + i * 16), D32(0x435f94 + i * 16), D32(0x435f98 + i * 16),
                    D32(0x435f9c + i * 16));
        i = i + 1;
    } while (i < 3);
    Hotspot_Set(0x4c0, D32(0x435fc0), D32(0x435fc4), D32(0x435fc8), D32(0x435fcc));
    D32(0x43ac78) = 24000;
}

// 0x41191f: round init.
void FUN_0041191f() {
    FUN_004119ba();
    int i = 0;
    do {
        D32(0x43ac98 + i * 4) = 0;
        i = i + 1;
    } while (i < 3);
    D32(0x43ac68) = -1;
    D32(0x43ac5c) = 0;
    D32(0x43ac64) = 0;
    P(0x435e88) = DSTR(0x4360b8); // "Frog Potion"
    P(0x435e8c) = DSTR(0x4360c4); // "Bat bile"
    P(0x435e90) = DSTR(0x4360cd); // "Moss slime"
    P(0x435e94) = DSTR(0x4360d8); // "Frog gas"
    FUN_00412076();
    i = 0;
    do {
        P(0x435e78 + i * 4) = Img_Create(0x4c, 0x2c);
        i = i + 1;
    } while (i < 3);
    FUN_00412157();
    D32(0x43ac6c) = 0;
    D32(0x43ac70) = 0;
    D32(0x43ac74) = 0;
}

// Flask amounts DAT_0043ac80[0..2] (byte-indexed in the decompilation).
static inline s32u& Flask(int i) { return D32(0x43ac80 + i * 4); }
static inline s32u& Target(int i) { return D32(0x43ac8c + i * 4); }
static inline s32u& Beaker(int i) { return D32(0x43ac98 + i * 4); }

// Sorts the three flask values a, b, c: Flask(0) = max, Flask(1) = mid,
// Flask(2) = min (literal transcription of the original's compare chain).
static void SortFlasks(int a, int b, int c, bool firstIsLess) {
    int lo = b;
    if (firstIsLess) {
        lo = a;
        a = b;
    }
    Flask(1) = lo;
    Flask(0) = a;
    int last = c;
    if (lo < c) {
        Flask(1) = c;
        last = lo;
        if (a < c) {
            Flask(1) = a;
            Flask(0) = c;
        }
    }
    Flask(2) = last;
}

// 0x4119ba: picks flask sizes and target amounts for the current level.
void FUN_004119ba() {
    bool ok;
    int i, tries, a, b, c, v, k;
    u32 level = FUN_004040a2();
    switch (level) {
    case 0:
        D32(0x43ac7c) = 0;
        Flask(0) = 100;
        Flask(1) = 10;
        Flask(2) = 1;
        i = 0;
        do {
            tries = 0;
            do {
                a = Rand();
                a = a % 10;
                b = Rand();
                b = b % 10;
                c = Rand();
                c = c % 10;
                v = a * 100 + b * 10 + c;
                if ((b + a + c < 2) || (10 < b + a + c)) {
                    ok = false;
                } else {
                    ok = true;
                }
                tries = tries + 1;
                if (tries < 100 && 0 < i) {
                    k = 0;
                    do {
                        if (v == Target(k)) ok = false;
                        k = k + 1;
                    } while (k < i);
                }
            } while (!ok);
            Target(i) = v;
            i = i + 1;
        } while (i < 3);
        break;
    case 1: {
        D32(0x43ac7c) = 0;
        do {
            int r = Rand();
            int m = 1;
            if (Rand() % 2 == 0) m = 10;
            a = (r % 9 + 1) * m;
            r = Rand();
            m = 1;
            if (Rand() % 2 == 0) m = 10;
            b = (r % 9 + 1) * m;
            r = Rand();
            if (Rand() % 2 == 0) {
                c = 10;
            } else {
                c = 1;
            }
            c = (r % 9 + 1) * c;
            if (b - a == 0 || a - c == 0 || b - c == 0) {
                ok = false;
            } else {
                ok = true;
            }
        } while (!ok);
        SortFlasks(a, b, c, b - a != 0 && a <= b);
        i = 0;
        do {
            int r1, r2;
            tries = 0;
            do {
                r1 = Rand();
                r2 = Rand();
                ok = true;
                tries = tries + 1;
                if (tries < 100 && 0 < i) {
                    k = 0;
                    do {
                        if (Flask(r1 % 3) + Flask(r2 % 3) == Target(k)) ok = false;
                        k = k + 1;
                    } while (k < i);
                }
            } while (!ok);
            Target(i) = Flask(r1 % 3) + Flask(r2 % 3);
            i = i + 1;
        } while (i < 3);
        break;
    }
    case 2:
        D32(0x43ac7c) = 0;
        do {
            int r = Rand() % 8;
            int m = 1;
            if (Rand() % 2 == 0) m = 10;
            a = (r + 2) * m;
            r = Rand() % 8;
            m = 1;
            if (Rand() % 2 == 0) m = 10;
            b = (r + 2) * m;
            r = Rand() % 8;
            if (Rand() % 2 == 0) {
                c = 10;
            } else {
                c = 1;
            }
            c = (r + 2) * c;
            if (b - a == 0 || a - c == 0 || b - c == 0) {
                ok = false;
            } else {
                ok = true;
            }
        } while (!ok);
        SortFlasks(a, b, c, b - a != 0 && a <= b);
        i = 0;
        do {
            tries = 0;
            do {
                int r1 = Rand();
                int r2 = Rand();
                if (Rand() % 2 == 0) {
                    v = Flask(r1 % 3) - Flask(r2 % 3);
                } else {
                    v = Flask(r1 % 3) + Flask(r2 % 3);
                }
                ok = 0 < v;
                tries = tries + 1;
                if (tries < 100 && 0 < i) {
                    k = 0;
                    do {
                        if (v == Target(k)) ok = false;
                        k = k + 1;
                    } while (k < i);
                }
            } while (!ok);
            Target(i) = v;
            i = i + 1;
        } while (i < 3);
        break;
    case 3:
    case 4:
        D32(0x43ac7c) = 1;
        do {
            if (level == 3) {
                a = Rand();
                a = (a % 9 + 1) * 10;
                b = Rand();
                b = (b % 9 + 1) * 10;
                c = Rand();
                c = (c % 9 + 1) * 10;
            } else {
                a = Rand();
                a = (a % 0x13 + 1) * 5;
                b = Rand();
                b = (b % 0x13 + 1) * 5;
                c = Rand();
                c = (c % 0x13 + 1) * 5;
            }
            if (b == a || a == c || b == c) {
                ok = false;
            } else {
                ok = true;
            }
        } while (!ok);
        SortFlasks(a, b, c, a < b);
        i = 0;
        do {
            tries = 0;
            do {
                int r1 = Rand();
                int r2 = Rand();
                if (Rand() % 2 == 0) {
                    v = Flask(r1 % 3) - Flask(r2 % 3);
                } else {
                    v = Flask(r1 % 3) + Flask(r2 % 3);
                }
                if (v < 1 || 100 < v) {
                    ok = false;
                } else {
                    ok = true;
                }
                tries = tries + 1;
                if (tries < 100 && 0 < i) {
                    k = 0;
                    do {
                        if (v == Target(k)) ok = false;
                        k = k + 1;
                    } while (k < i);
                }
            } while (!ok);
            Target(i) = v;
            i = i + 1;
        } while (i < 3);
        break;
    }
}

// 0x412076: decompresses the flask images and prints their amounts on them.
void FUN_00412076() {
    int k = 0;
    do {
        int j = 0;
        do {
            int i = 0;
            do {
                int idx = k * 8 + j * 2 + i;
                void* img = Img_Decompress(ResFrame(P(0x435dec), idx));
                FUN_004120e7(img, k);
                P(0x435e18 + idx * 4) = img;
                i = i + 1;
            } while (i < 2);
            j = j + 1;
        } while (j < 4);
        k = k + 1;
    } while (k < 3);
}

// 0x4120e7: draws flask idx's amount into img.
void FUN_004120e7(void* img, int idx) {
    char text[32]; // 8 in the original
    Gfx_SetTargetImg(img);
    FUN_004122be(Flask(idx), text, nullptr, 0, 0);
    int w = Img_W(img);
    Rect r;
    Rect_Set(&r, 0, 0, w, w); // sic: width used for both
    Text_DrawBoxFont(r.x, r.y, r.w, r.h, text, P(0x4318ac), 0x33);
    Gfx_ResetTarget();
}

// 0x412157: creates the round's sprites.
void FUN_00412157() {
    D32(0x435e98) = Spr_Create(nullptr, D32(0x436ee4), D32(0x436ee8), 1000);
    int i = 0;
    do {
        D32(0x435e9c + i * 4) =
            Spr_Create(P(0x435e30 + i * 0x20), D32(0x435f28 + i * 8), D32(0x435f2c + i * 8), i + 0x1e);
        i = i + 1;
    } while (i < 3);
    D32(0x435ea8) = Spr_Create(ResFrame(P(0x435df4), 0), D32(0x436040), D32(0x436044), 0x14);
    i = 0;
    do {
        FUN_0041264b(i);
        D32(0x435eac + i * 4) =
            Spr_Create(P(0x435e78 + i * 4), D32(0x436048 + i * 8), D32(0x43604c + i * 8), i + 0x37);
        i = i + 1;
    } while (i < 3);
    D32(0x435ebc) =
        Spr_Create(ResFrame(P(0x435dfc + D32(0x43ac58) * 4), 0), D32(0x436068), D32(0x43606c), 10);
    D32(0x435ec0) = Spr_Create(nullptr, D32(0x436070), D32(0x436074), 0xb);
    D32(0x435ec4) = Spr_Create(nullptr, D32(0x436ee4), D32(0x436ee8), 0x28);
    D32(0x435eb8) = Spr_Create(nullptr, D32(0x436060), D32(0x436064), 0x28);
    D32(0x435ec8) = Spr_Create(nullptr, D32(0x436098), D32(0x43609c), 0x32);
}

// 0x4122be: formats an amount in hundredths as a decimal ("1.05", ".5") or,
// with asFraction, as a reduced fraction value/100 into out and outDen.
// param_4 != 0 keeps trailing zeros of the decimals.
void FUN_004122be(int value, char* out, char* outDen, int param_4, int asFraction) {
    if (asFraction == 0) {
        int whole = value / 100;
        int frac = value % 100;
        if (param_4 == 0) {
            if (whole == 0) {
                if (frac == 0) {
                    sprintf(out, "%s", DSTR(0x43610f)); // "0"
                } else if (frac < 10) {
                    sprintf(out, DSTR(0x436111), frac); // ".0%d"
                } else {
                    if (frac % 10 == 0) frac = frac / 10;
                    sprintf(out, DSTR(0x436116), frac); // ".%d"
                }
            } else if (frac == 0) {
                sprintf(out, DSTR(0x4360ff), whole); // "%d"
            } else if (frac < 10) {
                sprintf(out, DSTR(0x436102), whole, frac); // "%d.0%d"
            } else {
                if (frac % 10 == 0) frac = frac / 10;
                sprintf(out, DSTR(0x436109), whole, frac); // "%d.%d"
            }
        } else if (whole == 0) {
            if (frac == 0) {
                sprintf(out, "%s", DSTR(0x4360f4)); // "0"
            } else if (frac < 10) {
                sprintf(out, DSTR(0x4360f6), frac); // ".0%d"
            } else {
                sprintf(out, DSTR(0x4360fb), frac); // ".%d"
            }
        } else if (frac < 10) {
            sprintf(out, DSTR(0x4360e7), whole, frac); // "%d.0%d"
        } else {
            sprintf(out, DSTR(0x4360ee), whole, frac); // "%d.%d"
        }
    } else {
        int den = 100;
        while (true) {
            if (value % 2 != 0) break;
            if (den % 2 != 0) break;
            value = value / 2;
            den = den / 2;
        }
        for (; value % 5 == 0 && den % 5 == 0; den = den / 5) {
            value = value / 5;
        }
        sprintf(out, DSTR(0x4360e1), value); // "%d"
        sprintf(outDen, DSTR(0x4360e4), den); // "%d"
    }
}

// 0x4124bf: draws the round's background and fades in.
void FUN_004124bf() {
    void* img = Res_Load(900);
    Img_Draw(D32(0x436ee4), D32(0x436ee8), img, 0);
    Res_Free(img);
    FUN_0041255d();
    FUN_00412603();
    Gfx_SaveBg(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    FUN_004032e3();
    Spr_Update(0);
    if (D32(0x43ac58) == 0) {
        Screen_FlushAll();
        void* pal = Res_Load(0x385);
        Pal_FadeTo((const u8*)pal, 1);
        Res_Free(pal);
        return;
    }
    Fx_OpenCenter();
}

// 0x41255d: recipe card (title, target amounts and ingredient names).
void FUN_0041255d() {
    char text[32]; // 8 in the original
    Text_DrawBoxFont(D32(0x435fd0), D32(0x435fd4), D32(0x435fd8), D32(0x435fdc), (char*)P(0x435e88),
                     P(0x4318b8), 0x24);
    int i = 0;
    do {
        FUN_004122be(Target(i), text, nullptr, 1, 0);
        Text_DrawScreenFont(D32(0x435fe0 + i * 8), D32(0x435fe4 + i * 8), text, P(0x4318a8), 0x24);
        Text_DrawScreenFont(D32(0x435ff8 + i * 8), D32(0x435ffc + i * 8), (char*)P(0x435e8c + i * 4),
                            P(0x4318a8), D32(0x435ecc + i * 4));
        i = i + 1;
    } while (i < 3);
}

// 0x412603: ingredient names under the beakers.
void FUN_00412603() {
    int i = 0;
    do {
        Text_DrawBoxFont(D32(0x436010 + i * 16), D32(0x436014 + i * 16), D32(0x436018 + i * 16),
                         D32(0x43601c + i * 16), (char*)P(0x435e8c + i * 4), P(0x4318ac), D32(0x435ecc + i * 4));
        i = i + 1;
    } while (i < 3);
}

// 0x41264b: redraws beaker idx's label (current amount).
void FUN_0041264b(int idx) {
    char num[32]; // 8 in the original
    char den[32]; // 8 in the original
    u32 wDen, wNum;
    Rect r;
    void* img = P(0x435e78 + idx * 4);
    Img_FullRect(img, &r);
    Gfx_SetTargetImg(img);
    u8 key = Img_Key(img);
    Gfx_FillRect(r.x, r.y, r.w, r.h, key);
    FUN_004122be(Beaker(idx), num, den, 1, D32(0x43ac7c));
    if (D32(0x43ac7c) == 0) {
        Text_DrawBoxFont(r.x, r.y, r.w, r.h, num, P(0x4318b0), 0x33);
    } else {
        r.h = r.h / 2;
        Text_DrawBoxFont(r.x, r.y, r.w, r.h, num, P(0x4318a8), 0x33);
        r.y = r.y + r.h;
        Text_DrawBoxFont(r.x, r.y, r.w, r.h, den, P(0x4318a8), 0x33);
        Text_MeasureFont(num, P(0x4318a8), &wNum, nullptr);
        Text_MeasureFont(den, P(0x4318a8), &wDen, nullptr);
        if ((int)wDen < (int)wNum) {
            wDen = wNum;
        }
        int x = (int)(r.w - wDen) / 2;
        Rect line;
        Rect_Set(&line, x, r.y + -2, wDen - 1, 0);
        Gfx_Line(line.x, line.y, line.w, line.h, 0x33);
    }
    Gfx_ResetTarget();
}

// 0x4127c2
void FUN_004127c2(int idx) {
    FUN_0041264b(idx);
    Spr_Invalidate(D32(0x435eac + idx * 4));
}

// 0x4127e0: the flask follows the mouse while one is picked up.
void FUN_004127e0() {
    int mx = g_mouseX;
    int my = g_mouseY;
    if (!Pt_InRect(g_mouseX, g_mouseY, D32(0x435ed8), D32(0x435edc), D32(0x435ee0), D32(0x435ee4))) {
        Spr_SetImage(D32(0x435e98), nullptr);
        Cursor_Show(1);
    } else if (D32(0x43ac68) == -1) {
        Spr_SetImage(D32(0x435e98), nullptr);
        Cursor_Show(1);
    } else {
        if (!Pt_InRect(mx, my, D32(0x435f50), D32(0x435f54), D32(0x435f58), D32(0x435f5c)) &&
            !Pt_InRect(mx, my, D32(0x435fc0), D32(0x435fc4), D32(0x435fc8), D32(0x435fcc))) {
            Cursor_Show(0);
            int pressed = (s16)Mouse_Buttons() != 0;
            void* img;
            if (D32(0x43ac5c) == 1) {
                img = P(0x435e18 + (D32(0x43ac68) * 8 + D32(0x43ac60) * 2 + pressed) * 4);
            } else {
                img = P(0x435e30 + (D32(0x43ac68) * 8 + pressed) * 4);
            }
            Spr_SetImage(D32(0x435e98), img);
            int y = my - Img_H(img);
            Point p;
            Pt_Set(&p, mx - Img_W(img) / 2, y);
            Spr_SetPos(D32(0x435e98), p.x, p.y);
            return;
        }
        Spr_SetImage(D32(0x435e98), nullptr);
        Cursor_Show(1);
    }
}

// Ms. Grunkle-style remark through the module's talking animation.
static inline void Talk(u32 resId) { FUN_00402d42(resId, P(0x435e08), D32(0x435ec0)); }

// 0x41297c: event handler. (The original returns whatever was left in EAX;
// the only caller ignores it.)
int FUN_0041297c(u16 ev) {
    if (FUN_0040394c(ev) != 0) return 0;
    u32 code = ev;
    if (code < 0x491) {
        if (code == 0x490) {
            // pour flask back / stir
            if (D32(0x43ac68) < 0) {
                FUN_00402d81();
                FUN_00402bad(0xc, 1);
                Talk(0x154d);
                return 0;
            }
            if (D32(0x43ac5c) == 0) {
                D32(0x43ac60) = D32(0x43ac64);
                D32(0x43ac5c) = 1;
                FUN_00402bad(3, 2);
                return 0;
            }
            if (D32(0x43ac60) == D32(0x43ac64)) {
                D32(0x43ac5c) = 0;
                FUN_00402bad(4, 2);
                return 0;
            }
            FUN_00402d81();
            FUN_00402bad(0xb, 1);
            Talk(0x154c);
            return 0;
        }
        if (code == 0xc) {
            FUN_00403f95();
        } else {
            if (code == 0x20) {
                FUN_00402d81();
                return 0;
            }
            if (code != 0x479) {
                if (2 < code - 0x480) {
                    return 0;
                }
                // pick up / put down a flask
                if (-1 < D32(0x43ac68)) {
                    Spr_SetImage(D32(0x435e9c + D32(0x43ac68) * 4), P(0x435e30 + D32(0x43ac68) * 0x20));
                }
                if ((int)ev - 0x480 != D32(0x43ac68)) {
                    if (D32(0x43ac68) < 0) {
                        FUN_00402bad(0, 1);
                    } else {
                        FUN_00402bad(2, 1);
                    }
                    D32(0x43ac68) = ev - 0x480;
                    D32(0x43ac5c) = 0;
                    Spr_SetImage(D32(0x435e9c + D32(0x43ac68) * 4), nullptr);
                    return 0;
                }
                FUN_00402bad(1, 1);
                D32(0x43ac68) = -1;
                D32(0x43ac5c) = 0;
                return 0;
            }
            if (-1 < D32(0x43ac68)) {
                FUN_00402bad(1, 1);
                Spr_SetImage(D32(0x435e9c + D32(0x43ac68) * 4), P(0x435e30 + D32(0x43ac68) * 0x20));
                D32(0x43ac68) = -1;
                D32(0x43ac5c) = 0;
                return 0;
            }
        }
    } else {
        if (code == 0x491) {
            FUN_00412de7();
            return 0;
        }
        if (code - 0x4a0 < 3 || code - 0x4b0 < 3) {
            if (-1 < D32(0x43ac68)) {
                int b;
                if (ev < 0x4a0 || 0x4a2 < ev) {
                    b = ev - 0x4b0;
                } else {
                    b = ev - 0x4a0;
                }
                if (D32(0x43ac5c) == 1) {
                    if (b == D32(0x43ac60)) {
                        Beaker(b) = Beaker(b) + Flask(D32(0x43ac68));
                        FUN_004127c2(b);
                        D32(0x43ac5c) = 0;
                        FUN_00402bad(4, 1);
                        return 0;
                    }
                    FUN_00402d81();
                    FUN_00402bad(0xb, 1);
                    void* ang = P(0x435e08);
                    int spr = D32(0x435ec0);
                    int r = Rand() % 2;
                    FUN_00402d42(r + 0x1551, ang, spr);
                    return 0;
                }
                if (Flask(D32(0x43ac68)) <= Beaker(b)) {
                    Beaker(b) = Beaker(b) - Flask(D32(0x43ac68));
                    FUN_004127c2(b);
                    D32(0x43ac5c) = 1;
                    D32(0x43ac60) = b;
                    FUN_00402bad(3, 2);
                    return 0;
                }
                FUN_00402d81();
                FUN_00402bad(0xb, 1);
                void* ang = P(0x435e08);
                int spr = D32(0x435ec0);
                int r = Rand();
                FUN_00402d42(r % 3 + 0x154e, ang, spr);
                return 0;
            }
        } else {
            if (code - 0x4c0 != 0) {
                return 0;
            }
            // "done" button: check all beakers
            bool empty = true;
            int i = 0;
            do {
                if (Beaker(i) != 0) {
                    empty = false;
                    break;
                }
                i = i + 1;
            } while (i < 3);
            if (empty) {
                FUN_00402d81();
                FUN_00402bad(0xb, 1);
                Talk(0x1553);
                return 0;
            }
            D32(0x43ac6c) = 1;
            i = 0;
            do {
                if (Beaker(i) != Target(i)) {
                    D32(0x43ac6c) = 0;
                    FUN_00412fbd(i);
                    Beaker(i) = 0;
                    FUN_004127c2(i);
                }
                i = i + 1;
            } while (i < 3);
            if (D32(0x43ac6c) == 0) {
                FUN_00402d81();
                void* ang = P(0x435e08);
                int spr = D32(0x435ec0);
                int r = Rand();
                FUN_00402d42(r % 3 + 0x1554, ang, spr);
            }
            FUN_00404068(D32(0x43ac6c));
            if (D32(0x43ac6c) == 0) {
                D32(0x43ac70) = D32(0x43ac70) + 1;
                return 0;
            }
        }
    }
    return 0;
}

// 0x412de7: turns the stirring wheel to the next position.
void FUN_00412de7() {
    int stops[4];
    memcpy(stops, DPTR(0x4360a0), sizeof(stops)); // {0, 7, 13, 0}
    bool abort = false;
    FUN_00402bad(5, 2);
    int frame = stops[D32(0x43ac64)] + 1;
    int endFrame = stops[D32(0x43ac64) + 1];
    int last;
    if (endFrame < frame) {
        last = (int)((RdU32(P(0x435df4)) >> 2) - 2);
    } else {
        last = endFrame + -1;
    }
    while (frame <= last && !abort) {
        Spr_SetImage(D32(0x435ea8), ResFrame(P(0x435df4), frame));
        Spr_Update(1);
        Timer_Set(1, 100);
        while (!Timer_Expired(1)) {
            int ev = Event_Poll();
            FUN_0040394c((u16)ev);
            if ((u16)ev == 0x20) {
                abort = true;
            }
            FUN_004127e0();
            Spr_Update(1);
        }
        frame = frame + 1;
    }
    Spr_SetImage(D32(0x435ea8), ResFrame(P(0x435df4), endFrame));
    Spr_Update(1);
    D32(0x43ac64) = D32(0x43ac64) + 1;
    if (2 < D32(0x43ac64)) {
        D32(0x43ac64) = 0;
    }
}

// 0x412efc: the hint clock.
void FUN_00412efc() {
    if (Timer_Expired(8)) {
        Timer_Set(8, D32(0x43ac78));
        D32(0x43ac74) = D32(0x43ac74) + 1;
        Spr_SetImage(D32(0x435ec8), ResFrame(P(0x435e84), D32(0x43ac74)));
        if (4 < D32(0x43ac74)) {
            FUN_00412f78();
            D32(0x43ac74) = 0;
            Spr_SetImage(D32(0x435ec8), ResFrame(P(0x435e84), 0));
        }
    }
}

// 0x412f78: time is up: empty all beakers.
void FUN_00412f78() {
    Talk(0x1557);
    int i = 0;
    do {
        FUN_00412fbd(i);
        Beaker(i) = 0;
        FUN_004127c2(i);
        i = i + 1;
    } while (i < 3);
    D32(0x43ac70) = D32(0x43ac70) + 1;
}

// 0x412fbd: fizz animation on beaker idx.
void FUN_00412fbd(int idx) {
    FUN_00402bad(6, 2);
    u8* anim = (u8*)P(0x435e0c + idx * 4);
    Spr_SetPos(D32(0x435ec4), D32(0x436080 + idx * 8), D32(0x436084 + idx * 8));
    for (u32 f = 0; f < (RdU32(anim) >> 2) - 1; f = f + 1) {
        Spr_SetImage(D32(0x435ec4), anim + RdU32(anim + f * 4));
        Spr_Update(1);
        Timer_Set(1, 100);
        while (!Timer_Expired(1)) {
            int ev = Event_Poll();
            FUN_0040394c((u16)ev);
            FUN_004127e0();
            Spr_Update(1);
        }
    }
    Spr_SetImage(D32(0x435ec4), nullptr);
    Spr_Update(0);
}

// 0x413069: success animation at the end of a round.
void FUN_00413069() {
    bool abort = false;
    FUN_00402d81();
    int snd = FUN_00402bd1(7, 1); // looped; handle stopped below
    int loop = 0;
    while (loop < 3 && !abort) {
        u32 f = 0;
        while (f < (RdU32(P(0x435df8)) >> 2) - 1 && !abort) {
            Spr_SetImage(D32(0x435eb8), (u8*)P(0x435df8) + RdU32((u8*)P(0x435df8) + f * 4));
            Spr_Update(1);
            Timer_Set(1, 100);
            while (!Timer_Expired(1)) {
                int ev = Event_Poll();
                FUN_0040394c((u16)ev);
                if ((u16)ev == 0x20) {
                    abort = true;
                }
                FUN_004127e0();
                Spr_Update(1);
            }
            f = f + 1;
        }
        loop = loop + 1;
    }
    Spr_SetImage(D32(0x435eb8), nullptr);
    FUN_00402bf5(snd);
    Spr_SetImage(D32(0x435ec0), nullptr);
    FUN_00402bad(6, 2);
    u8* anim = (u8*)P(0x435dfc + D32(0x43ac58) * 4);
    for (u32 f = 0; f < (RdU32(anim) >> 2) - 1; f = f + 1) {
        Spr_SetImage(D32(0x435ebc), anim + RdU32(anim + f * 4));
        Spr_Update(1);
        Timer_Set(1, 100);
        while (!Timer_Expired(1)) {
            int ev = Event_Poll();
            FUN_0040394c((u16)ev);
            FUN_004127e0();
            Spr_Update(1);
        }
    }
    if (D32(0x43ac58) == 2) {
        Res_Free(P(0x435e08));
        P(0x435e08) = nullptr;
        P(0x435e08) = Res_Load(0x391);
        Spr_SetPos(D32(0x435ec0), D32(0x436078), D32(0x43607c));
    }
    FUN_00402e72(D32(0x43ac58) + 0x1558, P(0x435e08), D32(0x435ec0), 1, 1);
}

// 0x413233: module intro narration (firstVisit = 1 plays the long version).
int FUN_00413233(int firstVisit) {
    int r = 0;
    if (firstVisit == 0) {
        r = FUN_00402ec2(D32(0x4360b0) + 0x1544, 0, 1);
        D32(0x4360b0) = D32(0x4360b0) + 1;
        if (2 < D32(0x4360b0)) {
            D32(0x4360b0) = 0;
        }
    } else {
        if (FUN_00402ec2(0x153f, 0, 1) != 0) return 1;
        if (FUN_00402e72(0x1540, P(0x435e08), D32(0x435ec0), 0, 1) != 0) return 1;
        if (FUN_00402ec2(0x1541, 0, 1) != 0) return 1;
        if (FUN_00402e72(0x1542, P(0x435e08), D32(0x435ec0), 0, 1) != 0) return 1;
        if (FUN_00402ec2(0x1543, 0, 1) != 0) return 1;
    }
    return r;
}
