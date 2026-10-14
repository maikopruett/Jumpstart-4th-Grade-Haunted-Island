// 8-bit graphics: back buffer, draw target, primitives, 'P800' images,
// palette, screen flushing, dirty-cell tracking and transitions.
// (0x41ec3a..0x4210b7, 0x42112c..0x42149a, 0x42255c..0x422ce7)
#pragma once
#include "engine/core.h"

// Draw target descriptor (0x43b754 layout: bits, rect, topDown).
struct Surface {
    u8* bits;
    Rect rect;    // rect.w is the pitch, rect.h the height
    int topDown;
};

extern Surface g_target;        // 0x43b754 current draw target
extern Surface g_screenSurf;    // 0x43b6d4 the back buffer (640 x 960)
extern Rect g_clip;             // 0x43b78c clip rect for the target
extern Rect g_screenBufRect;    // 0x43b77c whole back buffer
extern Rect g_viewRect;         // 0x43b76c visible screen (0,0,640,480)
extern int g_screenW, g_screenH;   // 0x437944 / 0x437948
extern u8* g_palette;           // 0x437098 current 768-byte RGB palette

void Gfx_Init();                // 0x41ec3a (+ dirty grid 0x42112c)
void Gfx_Shutdown();            // 0x41f26e

// Raw row copy used by a few game routines (asm at 0x40207c).
void Blit_Copy(const u8* src, u8* dst, int w, int h, int srcSkip, int dstSkip);

// ---- addressing (0x41fe7e / 0x41febd)
u8* Gfx_PixelPtr(int x, int y);
int Gfx_Offset(int x, int y);

// ---- copies inside the back buffer
void Gfx_CopyRect(int sx, int sy, int w, int h, int dx, int dy);  // 0x41feeb
void Gfx_SaveBg(int x, int y, int w, int h);      // 0x41ff64 visible -> background
void Gfx_RestoreBg(int x, int y, int w, int h);   // 0x41ffc9 background -> visible
void Gfx_Copy2x(int sx, int sy, int w, int h, int dx, int dy); // 0x420046

// ---- primitives on the current target
u8 Gfx_GetPixel(int x, int y);                    // 0x4200d1
void Gfx_SetPixel(int x, int y, u8 c);            // 0x420114
void Gfx_Line(int x, int y, int dx, int dy, u8 c);   // 0x420156
void Gfx_FrameRect(int x, int y, int w, int h, u8 c); // 0x420313
void Gfx_FillRect(int x, int y, int w, int h, u8 c);  // 0x4203e4
void Gfx_BlitRaw(int x, int y, const u8* src, int w, int h, int transparent, u8 key); // 0x420479
void Gfx_RemapRect(int x, int y, int w, int h, const u8* table); // 0x42055b
void Gfx_InvertRect(int x, int y, int w, int h);  // 0x4205fa
void Gfx_BlitRLE(int x, int y, const u8* data, int w, int h, int transparent, u8 key); // 0x4206d0

// ---- targets
void Surf_Init(Surface* s, u8* bits, int w, int h);  // 0x420638
void Gfx_SetTarget(u8* bits, int w, int h);       // 0x420664
void Gfx_ResetTarget();                           // 0x4206a5

// ---- 'P800' images: header {'P800', u16 w, u16 h, u8 key, u8 enc, u16},
// raw pixels or RLE rows/columns follow at +12.
static inline int Img_W(const void* img) { return RdU16((const u8*)img + 4); }   // 0x420875
static inline int Img_H(const void* img) { return RdU16((const u8*)img + 6); }   // 0x420883
static inline u8 Img_Key(const void* img) { return ((const u8*)img)[8]; }        // 0x420c31
static inline u8 Img_Enc(const void* img) { return ((const u8*)img)[9]; }
static inline u8* Img_Bits(void* img) { return (u8*)img + 12; }
int Img_Size(const void* img);                    // 0x420891
void Img_Bounds(const void* img, Rect* out);      // 0x4208ef (opaque pixel bbox)
void Img_FullRect(const void* img, Rect* out);    // 0x4208c6
void Img_Draw(int x, int y, const void* img, int transparent); // 0x42080b
void Img_Capture(int x, int y, int w, int h, void* img);       // 0x420c3c
int Img_RawSize(int x, int y, int w, int h);      // 0x420d10 (= w*h + 12)
void Img_DrawOpaque(int x, int y, const void* img); // 0x420cf8
// Builds a bottom-up BITMAPINFO (+256 RGBQUAD) + pixel DIB of img (0x421066).
void* Img_ToDIBAlloc(const void* img);
void Img_Remap(void* img, const u8* table);       // 0x420d1e
void* Img_Create(int w, int h);                   // 0x420e6d
void* Img_Decompress(const void* img);            // 0x420eae
void Surf_InitImg(Surface* s, void* img);         // 0x420f4a
void Gfx_SetTargetImg(void* img);                 // 0x420f6d

// ---- palette (0x41f544..0x41fd2b)
void Pal_Set(const u8* rgb, int mode);            // 0x41f544
void Pal_SetQuads(const u8* bgrx, int mode);      // 0x41f8c5
void Pal_QuadsToRGB(const u8* bgrx, u8* rgb);     // 0x41fa48
void Pal_RGBToQuads(const u8* rgb, u8* bgrx);     // 0x41fa86
void Pal_SetBlack(int mode);                      // 0x41fac1
void Pal_FadeTo(const u8* rgb, int mode);         // 0x41faf6
void Pal_FadeToBlack(int mode);                   // 0x41fcf6
void Pal_InitBlack();                             // 0x41d1a1

// ---- screen
void Screen_Flush(int x, int y, int w, int h);    // 0x41fd40
static inline void Screen_Flush(const Rect& r) { Screen_Flush(r.x, r.y, r.w, r.h); }
void Screen_FlushAll();                           // 0x41fda4
void Screen_Flush2x(int x, int y, int w, int h);  // 0x41fdc9

// ---- dirty grid of 16x10 cells (0x42112c..0x42149a)
void Dirty_Init();
void Dirty_Shutdown();
void Dirty_Mark(int x, int y, int w, int h);      // 0x421191
void Dirty_Clear();                               // 0x421291
void Dirty_Flush();                               // 0x4212c2
void Img_DrawDirty(int x, int y, const void* img, int transparent); // 0x421447
void Gfx_RestoreBgDirty(int x, int y, int w, int h); // 0x42149a

// ---- transitions (0x42255c..0x422ce7)
void Fx_FlushLine(int x1, int y1, int x2, int y2);         // 0x42255c / 0x4226d6
void Fx_Dissolve(int x, int y, int w, int h, int cw, int ch); // 0x422723
void Fx_WipeRight(int x, int y, int w, int h, int ms);     // 0x4227da
void Fx_WipeLeft(int x, int y, int w, int h, int ms);      // 0x422849
void Fx_WipeDown(int x, int y, int w, int h);              // 0x4228bc
void Fx_WipeUp(int x, int y, int w, int h);                // 0x422902
void Fx_WipeDiagonal(int x, int y, int w, int h);          // 0x422935
void Fx_WipeVCenterIn(int x, int y, int w, int h);         // 0x422a0b
void Fx_BoxOut(int x, int y, int w, int h, int ms);        // 0x422a67
void Fx_CloseBlack();                                      // 0x422c19
void Fx_OpenCenter();                                      // 0x422ce7
