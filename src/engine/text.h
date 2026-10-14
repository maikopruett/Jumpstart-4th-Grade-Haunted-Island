// Bitmap fonts and the word-wrapping text renderer (0x422d8c..0x423a30).
// Fonts are resources: +4 u16 height, +6 u16 widths[256],
// +0x206 u16 offsets[256], +0x406 1bpp glyph rows.
// Text may contain escapes: 0xff <code> ... 0xff (2 underline, 3 color+1,
// 4 tab stops, 5 toggle centering).
#pragma once
#include "engine/core.h"

typedef int (*FontWidthFn)(int ch);
typedef int (*FontHeightFn)();
typedef int (*FontDrawFn)(int x, int y, int ch, int color);

extern const void* g_curFont;   // 0x4379cc
extern int g_textOverflow;      // 0x43b7d0 (<0 skip if too long, >0 second column)
extern int g_textFirstLine;     // 0x43b7d4
extern int g_textColumnGap;     // 0x43b7d8

int Font_CharWidth(int ch, const void* font);                         // 0x42373c
int Font_Height(const void* font);                                    // 0x42374e
int Font_DrawChar(int x, int y, int ch, const void* font, int color); // 0x42375c

void Text_DrawBox(const char* text, int x, int y, int w, int h, int center, int color, FontWidthFn wfn,
                  FontHeightFn hfn, FontDrawFn dfn);                  // 0x422d8c
void Text_Measure(const char* text, FontWidthFn wfn, FontHeightFn hfn, u32* outW, int* outH); // 0x4235b7
int Text_MeasureHeight(const char* text, FontWidthFn wfn, FontHeightFn hfn, int width);       // 0x423686

// Current-font wrappers used by the game.
void Text_DrawFont(const char* text, int x, int y, int w, int h, const void* font, int color, int center); // 0x423903
void Text_DrawScreenFont(int x, int y, const char* text, const void* font, int color);  // 0x42393c
void Text_DrawBoxFont(int x, int y, int w, int h, const char* text, const void* font, int color); // 0x42396c
void Text_MeasureFont(const char* text, const void* font, u32* outW, int* outH); // 0x4239a2
int Text_MeasureHeightFont(const char* text, const void* font, int width);       // 0x4239ca
