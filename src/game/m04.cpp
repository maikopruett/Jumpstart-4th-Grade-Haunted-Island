// Module 4 (0x406890..0x4084c0): parts-of-speech activity. A sentence with
// blanks (resource text, "\<n><letter>" markers) is shown; word bubbles fly in
// and the kid catches the one that fits the current blank before the timer
// (a 12-step clock) runs out.
//
// Globals (data segment):
//   P(0x434310)      word list resource 0x213: u32 count, then 0x40-byte records
//   D32(0x434314)    word count;  P(0x434318) first record (P(0x434310) + 4)
//                    record: +0 text, +0x10 u32 part-of-speech mask, +0x14 bitset
//   P(0x434320)      sentence text (format with 16 %s), compacted in place
//   P(0x43431c)      clock images, P(0x434328) bubble images, P(0x4343b0) labels
//   P(0x434324)      talking animation (res 0x1fa)
//   P(0x43432c)[17], P(0x434370)[16]   narration sounds of the final sentence
//   P(0x4343b4)[n]   per-bubble text images
//   P(0x4398e8)      label image (sub image of 0x4343b0, or Img_Create'd)
//   P(0x439a90), P(0x439a94)  fonts
//   P(0x439a0c)[16]  char* to the chosen word of each blank
//   D32(0x439a4c)[16] chosen word index;  0x43998c + i*8: {bit index, mask}
#include "game/common.h"

// Sub image idx of an offset-table resource: base + ((s32*)base)[idx].
static inline void* M04_Sub(void* base, int idx) {
    return (u8*)base + RdS32((u8*)base + idx * 4);
}
// Word record idx of the list.
static inline u8* M04_Word(int idx) { return (u8*)P(0x434318) + idx * 0x40; }

// 0x406890
void FUN_00406890() {
    FUN_00406b34();
    D32(0x4398f4) = 0;
    do {
        FUN_004068c9();
        D32(0x4398f4) = D32(0x4398f4) + 1;
    } while (!FUN_004069d6());
    Spr_DeleteAll(0, 0);
    FUN_004040f5(1);
}

// 0x4068c9
void FUN_004068c9() {
    bool first;
    u32 ev;

    FUN_00406d95();
    FUN_0040702e();
    if (D32(0x4398f4) == 0) {
        FUN_0040658a();
    }
    do {
        FUN_004070fb();
        D32(0x4398f0) = 0;
        first = true;
        do {
            FUN_0040763e();
            FUN_004076e5();
            Spr_Update(1);
            ev = (u32)Event_Poll();
            FUN_00407c47((u16)ev);
            if (-1 < D32(0x4398ec)) {
                if ((RdU32(M04_Word(D32(0x4398ec)) + 0x10) & DU32(0x439990 + D32(0x4398f8) * 8)) == 0) {
                    if (first) {
                        FUN_00404068(0);
                        first = false;
                    }
                    FUN_00407d48();
                } else {
                    FUN_00404068(1);
                    FUN_00407cc4();
                    FUN_00403e2c(0x28);
                }
            }
        } while (D32(0x4398f0) == 0 && D32(0x4398fc) < 0xc);
        FUN_00406b01();
    } while (D32(0x4398f8) < D32(0x439988) && D32(0x4398fc) < 0xc);
    if (D32(0x4398f8) < D32(0x439988)) {
        FUN_00408287(0);
    } else {
        FUN_00403e2c(100);
        FUN_00419330(1);
        FUN_00407dad();
    }
    FUN_00406a92();
    Fx_CloseBlack();
}

// 0x4069d6
bool FUN_004069d6() {
    return 2 < D32(0x4398f4);
}

// 0x4069e8
void FUN_004069e8() {
    FUN_00406a92();
    Res_Free(P(0x434310));
    P(0x434310) = nullptr;
    Res_Free(P(0x43431c));
    P(0x43431c) = nullptr;
    Res_Free(P(0x4343b0));
    P(0x4343b0) = nullptr;
    Res_Free(P(0x434328));
    P(0x434328) = nullptr;
    for (int i = 0; i < D32(0x439900); i++) {
        Mem_Free(P(0x4343b4 + i * 4));
        P(0x4343b4 + i * 4) = nullptr;
    }
    if (D32(0x439a98) != 0) {
        Mem_Free(P(0x4398e8));
        P(0x4398e8) = nullptr;
    }
    Spr_DeleteAll(0, 0);
    FUN_00405901();
}

// 0x406a92
void FUN_00406a92() {
    FUN_00402d81();
    Anim_Stop(0x1f9);
    Anim_Free(0x1f9);
    Res_Free(P(0x434320));
    P(0x434320) = nullptr;
    int i = 0;
    do {
        Res_Free(P(0x43432c + i * 4));
        P(0x43432c + i * 4) = nullptr;
        i++;
    } while (i < 0x11);
    i = 0;
    do {
        Res_Free(P(0x434370 + i * 4));
        P(0x434370 + i * 4) = nullptr;
        i++;
    } while (i < 0x10);
}

// 0x406b01
void FUN_00406b01() {
    Spr_SetImage(D32(0x4343c4), nullptr);
    for (int i = 0; i < D32(0x439900); i++) {
        FUN_00407c14(i);
    }
    Spr_Update(1);
}

// 0x406b34
void FUN_00406b34() {
    u32 level;
    int i;
    int h;
    u32 w;

    FUN_00402c78(0x7ea);
    FUN_00403e91();
    level = FUN_004040cb();
    if (level == 0) {
        D32(0x439900) = 1;
        D32(0x439978) = 30000;
        D32(0x439a8c) = 0x28;
    } else if (level == 1) {
        D32(0x439900) = 2;
        D32(0x439978) = 25000;
        D32(0x439a8c) = 0x1e;
    } else if (level == 2) {
        D32(0x439900) = 3;
        D32(0x439978) = 15000;
        D32(0x439a8c) = 0x14;
    }
    level = FUN_004040a2();
    if (level == 0) {
        D32(0x439a98) = 1;
    } else if (level - 1 < 2) {
        D32(0x439a98) = 0;
    }
    D32(0x439970) = 5;
    D32(0x439974) = 2000;
    i = 0;
    do {
        D32(0x439984 + i * 4) = i;
        i++;
    } while (i < 1);
    Util_Shuffle(DPTR(0x439984), 1, 4);
    P(0x434310) = Res_Load(0x213);
    D32(0x434314) = RdS32(P(0x434310));
    P(0x434318) = (u8*)P(0x434310) + 4;
    P(0x4343b0) = Res_Load(0x1f7);
    P(0x43431c) = Res_Load(0x1f6);
    P(0x434328) = Res_Load(0x1f8);
    if (D32(0x439a98) != 0) {
        P(0x4398e8) = Img_Create(D32(0x4343fc) + 2, D32(0x434400) + 2);
    }
    Spr_DeleteAll(0, 0);
    D32(0x4343c0) = Spr_Create(M04_Sub(P(0x43431c), 0), D32(0x4343e4), D32(0x4343e8), 10);
    if (D32(0x439a98) == 0) {
        D32(0x4343c4) = Spr_Create(nullptr, D32(0x4343dc), D32(0x4343e0), 10);
    } else {
        D32(0x4343c4) = Spr_Create(nullptr, D32(0x434404), D32(0x434408), 10);
    }
    for (i = 0; i < D32(0x439900); i++) {
        D32(0x4343c8 + i * 4) = Spr_Create(nullptr, D32(0x436ee4), D32(0x436ee8), 0xb);
    }
    P(0x439a90) = P(0x4318b8);
    P(0x439a94) = P(0x4318f4);
    Text_MeasureFont(DSTR(0x43465c), P(0x4318b8), &w, &h);
    for (i = 0; i < D32(0x439900); i++) {
        P(0x4343b4 + i * 4) = Img_Create(w, h);
    }
    FUN_00402b7e(0x803);
}

// 0x406d95
void FUN_00406d95() {
    D32(0x439980) = D32(0x439984);
    FUN_00406e0a();
    FUN_00406e76();
    D32(0x4398f8) = 0;
    D32(0x4398fc) = 0;
    Spr_SetImage(D32(0x4343c0), M04_Sub(P(0x43431c), 0));
    Timer_Set(8, D32(0x439978));
    for (int i = 0; i < D32(0x439900); i++) {
        D32(0x439904 + i * 4) = -1;
    }
    D32(0x43997c) = 0;
}

// 0x406e0a
void FUN_00406e0a() {
    // EBX/ESI are left uninitialised by the original when the level is not
    // 0..2 (cannot happen).
    int base = 0;
    int range = 0;
    u32 level = FUN_004040a2();
    if (level == 0) {
        base = 600;
        range = 0x18;
    } else if (level == 1) {
        base = 0x270;
        range = 0x18;
    } else if (level == 2) {
        base = 0x288;
        range = 0x18;
    }
    do {
        D32(0x4398e4) = FUN_00403dd5(range, (int*)DPTR(0x43455c), 8);
    } while (D32(0x4398e4) == 0xf);
    P(0x434320) = Res_Load(base + D32(0x4398e4));
}

// 0x406e76: strips the "\<n><letter>?" blank markers from the sentence,
// recording each blank's word bit index and part-of-speech mask.
void FUN_00406e76() {
    u8* text = (u8*)P(0x434320);
    int len = (int)strlen((char*)text);
    u8* dst = (u8*)Mem_Alloc(len + 2);
    strcpy((char*)dst, (char*)text);
    int n = 0;
    u8* src = dst;
    u8* out = text;
    u8* p;
    while (*src != 0) {
        for (; *src != 0 && *src != 0x5c; src++) {
            *out = *src;
            out++;
        }
        if (*src == 0x5c) {
            D32(0x43998c + n * 8) = 0;
            while (p = src + 1, (D8(0x438ee9 + *p) & 2) != 0) {
                D32(0x43998c + n * 8) = D32(0x43998c + n * 8) * 10 + (*p - 0x30);
                src = p;
            }
            switch (*p) {
            case 0x41:
            case 0x61:
                D32(0x439990 + n * 8) = 4;
                break;
            default:
                Fatal_Error(DSTR(0x43466b));
                break;
            case 0x43:
            case 99:
                D32(0x439990 + n * 8) = 0x20;
                break;
            case 0x44:
            case 100:
                D32(0x439990 + n * 8) = 0x10;
                break;
            case 0x49:
            case 0x69:
                D32(0x439990 + n * 8) = 0x40;
                break;
            case 0x4e:
            case 0x6e:
                D32(0x439990 + n * 8) = 1;
                break;
            case 0x50:
            case 0x70:
                D32(0x439990 + n * 8) = 0x80;
                break;
            case 0x52:
            case 0x72:
                D32(0x439990 + n * 8) = 8;
                break;
            case 0x56:
            case 0x76:
                D32(0x439990 + n * 8) = 2;
            }
            if ((D8(0x438ee9 + *p) & 4) != 0) {
                DU32(0x439990 + n * 8) = DU32(0x439990 + n * 8) | 0x10000000;
            }
            n++;
            src = src + 3;
        }
    }
    Mem_Free(dst);
    *out = 0;
    D32(0x439988) = n;
}

// 0x40702e
void FUN_0040702e() {
    void* img = Res_Load(D32(0x439980) + 500);
    Img_Draw(D32(0x436ee4), D32(0x436ee8), img, 0);
    Res_Free(img);
    img = Res_Load(D32(0x4398e4) + 0x1fb);
    Img_Draw(D32(0x4343ec), D32(0x4343f0), img, 1);
    Res_Free(img);
    Gfx_SaveBg(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    FUN_004032e3();
    Spr_Update(0);
    if (D32(0x4398f4) == 0) {
        Screen_FlushAll();
        img = Res_Load(0x1f5);
        Pal_FadeTo((const u8*)img, 1);
        Res_Free(img);
        return;
    }
    Fx_OpenCenter();
}

// Label index of a part-of-speech mask byte, as the cascades of 0x4070fb.
// mode 0: fallback leaves esi unchanged unless bit 8 is set;
// mode 1: fallback is always 7.
static void M04_LabelIndex(u8 m, int* esi, int mode) {
    if (m & 1) *esi = 0;
    else if (m & 2) *esi = 1;
    else if (m & 4) *esi = 2;
    else if (m & 0x10) *esi = 3;
    else if (m & 0x20) *esi = 4;
    else if (m & 0x40) *esi = 5;
    else if (m & 0x80) *esi = 6;
    else if (mode == 1) *esi = 7;
    else if (m & 8) *esi = 7;
}

// 0x4070fb: shows the part of speech wanted for the current blank (label
// sprite), or in the easy level the whole sentence with labels in the blanks.
void FUN_004070fb() {
    Rect r;
    char words[16][31];
    char* buf;
    int i;
    // ESI is not initialised by the original before the cascades; every mask
    // has one of the tested bits set, so the value is never used unset.
    int esi = 0;

    FUN_00402bad(0, 2);
    if (D32(0x439a98) == 0) {
        M04_LabelIndex(D8(0x439990 + D32(0x4398f8) * 8), &esi, 0);
        P(0x4398e8) = M04_Sub(P(0x4343b0), esi);
        Spr_SetImage(D32(0x4343c4), P(0x4398e8));
    } else {
        i = (int)strlen((char*)P(0x434320));
        buf = (char*)Mem_Alloc(i + D32(0x439988) * 0x15 + 100);
        Gfx_RestoreBg(D32(0x4343f4), D32(0x4343f8), D32(0x4343fc), D32(0x434400));
        Gfx_SetTargetImg(P(0x4398e8));
        Img_FullRect(P(0x4398e8), &r);
        u8 key = Img_Key(P(0x4398e8));
        Gfx_FillRect(r.x, r.y, r.w, r.h, key);
        r.x = D32(0x4343f4);
        r.y = D32(0x4343f8);
        r.w = D32(0x4343fc);
        r.h = D32(0x434400);
        r.y = 0;
        r.x = 0;
        for (i = 0; i < D32(0x439988); i++) {
            if (i < D32(0x4398f8)) {
                sprintf(words[i], DSTR(0x434683), (char*)P(0x439a0c + i * 4));
            } else {
                M04_LabelIndex(D8(0x439990 + i * 8), &esi, 1);
                if (i == D32(0x4398f8)) {
                    sprintf(words[i], DSTR(0x43468e), (char*)P(0x43453c + esi * 4));
                } else {
                    sprintf(words[i], DSTR(0x434699), (char*)P(0x43453c + esi * 4));
                }
            }
        }
        sprintf(buf, (char*)P(0x434320), words[0], words[1], words[2], words[3], words[4], words[5],
                words[6], words[7], words[8], words[9], words[10], words[11], words[12], words[13],
                words[14], words[15]);
        r.x = 2;
        r.y = 2;
        Text_DrawFont(buf, 2, 2, r.w, r.h, P(0x439a94), 0x26, 0);
        for (i = 0; i < D32(0x439988); i++) {
            if (i < D32(0x4398f8)) {
                sprintf(words[i], DSTR(0x4346a4), (char*)P(0x439a0c + i * 4));
            } else {
                M04_LabelIndex(D8(0x439990 + i * 8), &esi, 0);
                if (i == D32(0x4398f8)) {
                    sprintf(words[i], DSTR(0x4346af), (char*)P(0x43453c + esi * 4));
                } else {
                    sprintf(words[i], DSTR(0x4346ba), (char*)P(0x43453c + esi * 4));
                }
            }
        }
        sprintf(buf, (char*)P(0x434320), words[0], words[1], words[2], words[3], words[4], words[5],
                words[6], words[7], words[8], words[9], words[10], words[11], words[12], words[13],
                words[14], words[15]);
        r.x = 1;
        r.y = 1;
        Text_DrawFont(buf, 1, 1, r.w, r.h, P(0x439a94), 0x2e, 0);
        Gfx_ResetTarget();
        Mem_Free(buf);
        Spr_SetImage(D32(0x4343c4), P(0x4398e8));
        Spr_Invalidate(D32(0x4343c4));
    }
}

// 0x40763e: advances the clock (9 frames per step, 12 steps).
void FUN_0040763e() {
    if (Timer_Expired(8)) {
        if (D32(0x43997c) < 9) {
            Timer_Set(8, 100);
            Spr_SetImage(D32(0x4343c0), M04_Sub(P(0x43431c), D32(0x4398fc) * 9 + D32(0x43997c)));
            D32(0x43997c) = D32(0x43997c) + 1;
            return;
        }
        Timer_Set(8, D32(0x439978) + -900);
        D32(0x43997c) = 0;
        D32(0x4398fc) = D32(0x4398fc) + 1;
        Spr_SetImage(D32(0x4343c0), M04_Sub(P(0x43431c), D32(0x4398fc) * 9));
    }
}

// 0x4076e5: per-bubble state machine (timer slot = bubble index).
//   0x439904[i] word (-1 = none), 0x439940[i] bubble size, 0x439964[i] phase
void FUN_004076e5() {
    int r;
    for (int i = 0; i < D32(0x439900); i++) {
        if (Timer_Expired(i)) {
            if (D32(0x439904 + i * 4) < 0) {
                if (D32(0x439970) == 0) {
                    r = 0;
                } else {
                    r = Rand();
                    r = r % D32(0x439970);
                }
                if (r == 0) {
                    D32(0x439904 + i * 4) = (s32)FUN_0040788c();
                    D32(0x439940 + i * 4) = FUN_00407955(i);
                    FUN_004079b7(i);
                    D32(0x439964 + i * 4) = 1;
                    FUN_00402bad(3, 1);
                } else {
                    Timer_Set(i, 100);
                }
            } else if (D32(0x439964 + i * 4) < 1) {
                if (D32(0x439964 + i * 4) == 0) {
                    D32(0x439964 + i * 4) = -3;
                    FUN_00402bad(4, 1);
                } else {
                    u32 st = DU32(0x439964 + i * 4);
                    u32 sign = (u32)((s32)st >> 0x1f);
                    Spr_SetImage(D32(0x4343c8 + i * 4),
                                 M04_Sub(P(0x434328), (int)((st ^ sign) - sign) + D32(0x439940 + i * 4) * 3 - 1));
                    Spr_SetPos(D32(0x4343c8 + i * 4), D32(0x43994c + i * 8), D32(0x439950 + i * 8));
                    D32(0x439964 + i * 4) = D32(0x439964 + i * 4) + 1;
                    if (D32(0x439964 + i * 4) < 0) {
                        Timer_Set(i, 100);
                    } else {
                        FUN_00407c14(i);
                    }
                }
            } else {
                Spr_SetImage(D32(0x4343c8 + i * 4),
                             M04_Sub(P(0x434328), D32(0x439964 + i * 4) + D32(0x439940 + i * 4) * 3 - 1));
                Spr_SetPos(D32(0x4343c8 + i * 4), D32(0x43994c + i * 8), D32(0x439950 + i * 8));
                D32(0x439964 + i * 4) = D32(0x439964 + i * 4) + 1;
                if (D32(0x439964 + i * 4) < 4) {
                    Timer_Set(i, 100);
                } else {
                    D32(0x439964 + i * 4) = 0;
                    FUN_00407b0d(i);
                    Timer_Set(i, D32(0x439974));
                }
            }
        }
    }
}

// 0x40788c: picks a word: with probability 0x439a8c% one whose bitset contains
// the current blank (a right answer), else one without the wanted part of
// speech. Returns the word index (EAX), the last candidate if none matched.
u32 FUN_0040788c() {
    int r = Rand();
    bool right = r % 100 < D32(0x439a8c);
    u32 idx;
    if (D32(0x434314) == 0) {
        idx = 0;
    } else {
        idx = (u32)Rand();
        idx = idx % DU32(0x434314);
    }
    u32 n = 0;
    while (true) {
        if (DU32(0x434314) <= n) {
            return idx;
        }
        if (right) {
            s32 bit = D32(0x43998c + D32(0x4398f8) * 8);
            u32 b = (u32)bit & 0x80000007;
            if ((s32)b < 0) {
                b = (b - 1 | 0xfffffff8) + 1;
            }
            if (bit < 0) {
                bit = bit + 7;
            }
            if (((1u << (b & 0x1f)) & (u32)M04_Word((int)idx)[0x14 + (bit >> 3)]) != 0) {
                return idx;
            }
        } else if ((RdU32(M04_Word((int)idx) + 0x10) & DU32(0x439990 + D32(0x4398f8) * 8)) == 0) {
            return idx;
        }
        idx = idx + 1;
        if (DU32(0x434314) <= idx) {
            idx = 0;
        }
        n = n + 1;
    }
}

// 0x407955: smallest bubble size (0..4) wide enough for the word.
int FUN_00407955(int param_1) {
    u32 w;
    int i;
    Text_MeasureFont((char*)M04_Word(D32(0x439904 + param_1 * 4)), P(0x439a90), &w, nullptr);
    i = 0;
    do {
        int iw = Img_W(M04_Sub(P(0x434328), i * 3));
        if ((int)w < iw) break;
        i++;
    } while (i < 5);
    if (4 < i) {
        i = 4;
    }
    return i;
}

// 0x4079b7: random non-overlapping position for bubble param_1.
//   0x439910 + i*16: text rect;  0x43994c + i*8: bubble image position
void FUN_004079b7(int param_1) {
    bool ok = false;
    Rect r;
    Rect b;
    int x;
    int y;
    Img_FullRect(P(0x4343b4 + param_1 * 4), &r);
    while (true) {
        if (ok) {
            D32(0x439910 + param_1 * 16) = r.x;
            D32(0x439914 + param_1 * 16) = r.y;
            D32(0x439918 + param_1 * 16) = r.w;
            D32(0x43991c + param_1 * 16) = r.h;
            Img_FullRect(M04_Sub(P(0x434328), D32(0x439940 + param_1 * 4) * 3), &b);
            D32(0x43994c + param_1 * 8) = (r.w - b.w) / 2 + r.x;
            D32(0x439950 + param_1 * 8) = (r.h - b.h) / 2 + r.y;
            return;
        }
        if (Rand() % 2 == 0) {
            x = 0x1e0;
        } else {
            x = 0;
        }
        r.x = x + (int)(0xa0u - (u32)r.w) / 2;
        if (r.h == 0xbe) {
            y = 0;
        } else {
            y = Rand();
            y = y % (0xbe - r.h);
        }
        r.y = y + 0xaa;
        ok = true;
        for (int j = 0; j < D32(0x439900); j++) {
            if (param_1 != j && -1 < D32(0x439904 + j * 4)) {
                if (Rect_Intersect(r.x, r.y, r.w, r.h, D32(0x439910 + j * 16), D32(0x439914 + j * 16),
                                   D32(0x439918 + j * 16), D32(0x43991c + j * 16), nullptr)) {
                    ok = false;
                    break;
                }
            }
        }
    }
}

// 0x407b0d: shows the word in bubble param_1 and makes it clickable.
void FUN_00407b0d(int param_1) {
    Rect r;
    Point pt;
    void* img = P(0x4343b4 + param_1 * 4);
    Gfx_SetTargetImg(img);
    Img_FullRect(img, &r);
    u8 key = Img_Key(img);
    Gfx_FillRect(r.x, r.y, r.w, r.h, key);
    Text_DrawBoxFont(r.x, r.y, r.w, r.h, (char*)M04_Word(D32(0x439904 + param_1 * 4)), P(0x439a90), 0x33);
    Gfx_ResetTarget();
    Spr_SetImage(D32(0x4343c8 + param_1 * 4), img);
    Pt_Set(&pt, D32(0x439910 + param_1 * 16), D32(0x439914 + param_1 * 16));
    Spr_SetPos(D32(0x4343c8 + param_1 * 4), pt.x, pt.y);
    Hotspot_Set((s16)param_1 + 0x400, D32(0x439910 + param_1 * 16), D32(0x439914 + param_1 * 16),
                D32(0x439918 + param_1 * 16), D32(0x43991c + param_1 * 16));
}

// 0x407c14
void FUN_00407c14(int param_1) {
    D32(0x439904 + param_1 * 4) = -1;
    Spr_SetImage(D32(0x4343c8 + param_1 * 4), nullptr);
    Hotspot_Remove((s16)param_1 + 0x400);
}

// 0x407c47: event handling; sets 0x4398ec to the clicked word (or -1).
void FUN_00407c47(u16 param_1) {
    D32(0x4398ec) = -1;
    int r = FUN_0040394c(param_1);
    if (r == 0) {
        if (param_1 < 0x400 || D32(0x439900) + 0x400 <= (int)(u32)param_1) {
            if (param_1 == 0xc) {
                FUN_00403f95();
            }
        } else {
            D32(0x4398ec) = D32(0x439904 + (param_1 - 0x400) * 4);
            for (int i = 0; i < D32(0x439900); i++) {
                if (param_1 - 0x400 != i) {
                    FUN_00407c14(i);
                }
            }
            Spr_Update(1);
        }
    }
}

// 0x407cc4: right answer: store the word in the current blank.
void FUN_00407cc4() {
    FUN_00402bad(1, 4);
    D32(0x4398f0) = 1;
    P(0x439a0c + D32(0x4398f8) * 4) = M04_Word(D32(0x4398ec));
    if ((D8(0x439990 + D32(0x4398f8) * 8 + 3) & 0x10) != 0) {
        u8* w = (u8*)P(0x439a0c + D32(0x4398f8) * 4);
        *w = (u8)Util_ToUpper(*w);
    }
    D32(0x439a4c + D32(0x4398f8) * 4) = D32(0x4398ec);
    D32(0x4398f8) = D32(0x4398f8) + 1;
}

// 0x407d48: wrong answer: 3 s penalty with the clock running.
void FUN_00407d48() {
    FUN_00402bad(2, 4);
    Timer_Set(9, 3000);
    while (!Timer_Expired(9)) {
        u32 ev = (u32)Event_Poll();
        FUN_0040394c((u16)ev);
        FUN_0040763e();
        Spr_Update(1);
    }
    for (int i = 0; i < D32(0x439900); i++) {
        FUN_00407c14(i);
    }
    Spr_Update(1);
}

// 0x407dad: sentence completed.
void FUN_00407dad() {
    FUN_00407e00();
    FUN_00408076();
    int i = 0;
    do {
        Res_Free(P(0x43432c + i * 4));
        P(0x43432c + i * 4) = nullptr;
        i++;
    } while (i < 0x11);
    i = 0;
    do {
        Res_Free(P(0x434370 + i * 4));
        P(0x434370 + i * 4) = nullptr;
        i++;
    } while (i < 0x10);
    FUN_00408287(1);
}

// 0x407e00: draws the completed sentence on the screen.
void FUN_00407e00() {
    Rect r;
    char words[16][31];
    int i;
    int len;

    r.x = D32(0x4343f4);
    r.y = D32(0x4343f8);
    r.w = D32(0x4343fc);
    r.h = D32(0x434400);
    len = (int)strlen((char*)P(0x434320));
    len = len + 1;
    for (i = 0; i < D32(0x439988); i++) {
        int l = (int)strlen((char*)P(0x439a0c + i * 4));
        len = l + len + 6;
    }
    char* buf = (char*)Mem_Alloc(len + 100);
    for (i = 0; i < D32(0x439988); i++) {
        sprintf(words[i], DSTR(0x4346c5), (char*)P(0x439a0c + i * 4));
    }
    sprintf(buf, (char*)P(0x434320), words[0], words[1], words[2], words[3], words[4], words[5], words[6],
            words[7], words[8], words[9], words[10], words[11], words[12], words[13], words[14], words[15]);
    r.x = r.x + 2;
    r.y = r.y + 2;
    Text_DrawFont(buf, r.x, r.y, r.w, r.h, P(0x439a94), 0x26, 0);
    for (i = 0; i < D32(0x439988); i++) {
        sprintf(words[i], DSTR(0x4346d0), (char*)P(0x439a0c + i * 4));
    }
    sprintf(buf, (char*)P(0x434320), words[0], words[1], words[2], words[3], words[4], words[5], words[6],
            words[7], words[8], words[9], words[10], words[11], words[12], words[13], words[14], words[15]);
    r.x = r.x + -1;
    r.y = r.y + -1;
    Text_DrawFont(buf, r.x, r.y, r.w, r.h, P(0x439a94), 0x2e, 0);
    Mem_Free(buf);
    r.x = r.x + -1;
    r.y = r.y + -1;
    r.w = r.w + 2;
    r.h = r.h + 2;
    Gfx_SaveBg(r.x, r.y, r.w, r.h);
    Screen_Flush(r.x, r.y, r.w, r.h);
    FUN_00402bad(6, 1);
}

// 0x408076: reads the completed sentence aloud: sentence pieces
// (0x43432c, skipped where bit i of the mask table 0x43457c is set) alternate
// with the chosen words (0x434370, sound 0xdf9 + word).
void FUN_00408076() {
    u16 masks[72];
    int piece;
    int k;
    int i;
    int j;
    int base;
    u16 mask;
    u32 level;

    memcpy(masks, DPTR(0x43457c), 0x24 * 4);
    piece = 0;
    k = 0;
    Hotspot_Set(0x20, D32(0x43188d), D32(0x431891), D32(0x431895), D32(0x431899));
    level = FUN_004040a2();
    mask = masks[D32(0x4398e4) * 3 + level];
    base = D32(0x43441c + level * 4 + D32(0x4398e4) * 0xc);
    for (i = 0; i < D32(0x439988); i++) {
        if (((1u << ((u8)i & 0x1f)) & (u32)mask) == 0) {
            P(0x43432c + piece * 4) = Res_Load(base + piece);
            piece++;
        }
        P(0x434370 + k * 4) = Res_Load(D32(0x439a4c + k * 4) + 0xdf9);
        k++;
    }
    if (((1u << ((u8)i & 0x1f)) & (u32)mask) == 0) {
        P(0x43432c + piece * 4) = Res_Load(base + piece);
    }
    piece = 0;
    i = 0;
    j = 0;
    while (true) {
        u8 b = (u8)i;
        if (D32(0x439988) <= i) {
            if (((1u << (b & 0x1f)) & (u32)mask) == 0 &&
                FUN_00402dcf(P(0x43432c + piece * 4), nullptr, 0, 1, 1) != 0) {
                return;
            }
            Hotspot_Remove(0x20);
            return;
        }
        if (((1u << (b & 0x1f)) & (u32)mask) == 0) {
            int ab = FUN_00402dcf(P(0x43432c + piece * 4), nullptr, 0, 1, 1);
            piece++;
            if (ab != 0) {
                return;
            }
        }
        if ((D8(0x439990 + j * 8 + 3) & 0x10) != 0 && ((1u << (b & 0x1f)) & (u32)mask) != 0) {
            Timer_Set(0, 0xfa);
            while (!Timer_Expired(0)) {
                u32 ev = (u32)Event_Poll();
                if ((u16)ev == 0x20) {
                    return;
                }
                FUN_0040394c((u16)ev);
            }
        }
        if (FUN_00402dcf(P(0x434370 + j * 4), nullptr, 0, 1, 1) != 0) break;
        i++;
        j++;
    }
}

// 0x408287: end of round: animation 0x1f9 then a comment
// (0xaca on time-out, 0xacb..0xace when completed).
void FUN_00408287(int param_1) {
    Anim_Load(0x1f9);
    D32(0x4343d4) = Spr_Create(nullptr, D32(0x436ee4), D32(0x436ee8), 0xc);
    Anim_StartWithSprite(0x1f9, D32(0x43440c), D32(0x434410), 0xc, D32(0x4343d4));
    Timer_Set(2, 0);
    while (Anim_IsPlaying(0x1f9)) {
        u32 ev = (u32)Event_Poll();
        FUN_0040394c((u16)ev);
        if (Timer_Expired(2)) {
            Timer_Set(2, 100);
            Anim_UpdateAll();
            Spr_Update(1);
        }
    }
    Hotspot_Set(0x20, D32(0x43188d), D32(0x431891), D32(0x431895), D32(0x431899));
    P(0x434324) = Res_Load(0x1fa);
    D32(0x4343d8) = Spr_Create(nullptr, D32(0x434414), D32(0x434418), 0xd);
    if (param_1 == 0) {
        FUN_00402e72(0xaca, P(0x434324), D32(0x4343d8), 1, 1);
    } else {
        void* ang = P(0x434324);
        int spr = D32(0x4343d8);
        u32 r = (u32)Rand();
        r = r & 0x80000003;
        if ((s32)r < 0) {
            r = (r - 1 | 0xfffffffc) + 1;
        }
        FUN_00402e72(r + 0xacb, ang, spr, 1, 1);
    }
    Hotspot_Remove(0x20);
    Spr_Delete(D32(0x4343d4), 0);
    Anim_Free(0x1f9);
    Spr_Delete(D32(0x4343d8), 0);
    Res_Free(P(0x434324));
    P(0x434324) = nullptr;
}

// 0x40840c
void FUN_0040840c(int param_1) {
    if (param_1 == 0) {
        FUN_00402ec2(D32(0x43460c) + 0xac4, 0, 1);
        D32(0x43460c) = D32(0x43460c) + 1;
        if (2 < D32(0x43460c)) {
            D32(0x43460c) = 0;
        }
    } else {
        FUN_00402ec2(0xac3, 0, 1);
    }
}
