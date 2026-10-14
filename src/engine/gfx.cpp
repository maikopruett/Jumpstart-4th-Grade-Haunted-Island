#include "engine/gfx.h"

#include "engine/input.h"
#include "engine/sound.h"
#include "engine/sys.h"

Surface g_target;
Surface g_screenSurf;
Rect g_clip;
Rect g_screenBufRect;
Rect g_viewRect;
int g_screenW = kScreenW, g_screenH = kScreenH;
u8* g_palette;

static u8* g_backBuffer;

// ---------------------------------------------------------------- blitters
// Ports of the hand-written asm at 0x40207c..0x40244c.

void Blit_Copy(const u8* src, u8* dst, int w, int h, int srcSkip, int dstSkip) {
    if (w == 0 || h == 0) return;
    while (h--) {
        memmove(dst, src, w);
        src += w + srcSkip;
        dst += w + dstSkip;
    }
}

static void Blit_CopyKey(const u8* src, u8* dst, int w, int h, int srcSkip, int dstSkip, u8 key) {
    if (w == 0 || h == 0) return;
    while (h--) {
        for (int i = 0; i < w; i++) {
            u8 c = *src++;
            if (c != key) *dst = c;
            dst++;
        }
        src += srcSkip;
        dst += dstSkip;
    }
}

static void Blit_Copy2x(const u8* src, u8* dst, int w, int h, int srcSkip, int dstSkip) {
    if (w == 0 || h == 0) return;
    while (h--) {
        u8* row = dst;
        for (int i = 0; i < w; i++) {
            row[2 * i] = src[i];
            row[2 * i + 1] = src[i];
        }
        dst += 2 * w + dstSkip;
        memcpy(dst, row, 2 * w);
        src += w + srcSkip;
        dst += 2 * w + dstSkip;
    }
}

static void RLE_Blit(const u8* src, u8* dst, int w, int h, int dstSkip, bool keyed, u8 key) {
    if (w == 0 || h == 0) return;
    while (h--) {
        int rem = w;
        while (rem > 0) {
            int c = *src++;
            if (c & 0x80) {
                int n = c & 0x7f;
                u8 v = *src++;
                rem -= n;
                if (!keyed || v != key) memset(dst, v, n);
                dst += n;
            } else {
                rem -= c;
                if (!keyed) {
                    memcpy(dst, src, c);
                    src += c;
                    dst += c;
                } else {
                    for (int i = 0; i < c; i++) {
                        u8 v = *src++;
                        if (v != key) *dst = v;
                        dst++;
                    }
                }
            }
        }
        dst += dstSkip;
    }
}

// Clipped RLE row decoder. A partially used run carries over between the
// skip/draw/skip phases as "rem" (count, 0x80 = fill run).
static void RLE_BlitClipped(const u8* src, u8* dst, int skipL, int w, int skipR, int h, int dstSkip,
                            bool keyed, u8 key) {
    if (w == 0 || h == 0) return;
    while (h--) {
        int rem = 0;
        int left = skipL;
        while (left > 0) {
            int c = *src++;
            if (c & 0x80) {
                int n = c & 0x7f;
                if (n > left) {
                    rem = (n - left) | 0x80; // src stays on the fill value
                    break;
                }
                left -= n;
                src++;
            } else {
                if (c > left) {
                    rem = c - left;
                    src += left;
                    break;
                }
                left -= c;
                src += c;
            }
        }
        int todo = w;
        int carry = 0;
        bool first = true;
        while (todo > 0) {
            int c;
            if (first && (rem & 0x7f)) c = rem;
            else c = *src++;
            first = false;
            if (c & 0x80) {
                int n = c & 0x7f;
                u8 v = *src;
                if (n > todo) {
                    carry = (n - todo) | 0x80;
                    n = todo;
                    todo = 0;
                } else {
                    src++;
                    carry = 0;
                    todo -= n;
                }
                if (!keyed || v != key) memset(dst, v, n);
                dst += n;
            } else {
                int n = c;
                if (n > todo) {
                    carry = n - todo;
                    n = todo;
                    todo = 0;
                } else {
                    carry = 0;
                    todo -= n;
                }
                for (int i = 0; i < n; i++) {
                    u8 v = *src++;
                    if (!keyed || v != key) *dst = v;
                    dst++;
                }
            }
        }
        int right = skipR;
        if (right > 0) {
            int c = carry;
            if ((c & 0x7f) == 0) c = *src++;
            for (;;) {
                int n;
                if (c & 0x80) {
                    n = c & 0x7f;
                    src++;
                } else {
                    n = c;
                    src += n;
                }
                right -= n;
                if (right <= 0) break;
                c = *src++;
            }
        }
        dst += dstSkip;
    }
}

// ---------------------------------------------------------------- setup

void Surf_Init(Surface* s, u8* bits, int w, int h) {
    s->bits = bits;
    Rect_Set(&s->rect, 0, 0, w, h);
    s->topDown = 1;
}

void Gfx_Init() {
    int w = g_screenW, h = g_screenH * 2;
    if (!g_backBuffer) g_backBuffer = (u8*)Mem_Alloc(w * h);
    Rect_Set(&g_viewRect, 0, 0, g_screenW, g_screenH);
    Rect_Set(&g_screenBufRect, 0, 0, w, h);
    g_clip = g_screenBufRect;
    g_screenSurf.bits = g_backBuffer;
    g_screenSurf.rect = g_screenBufRect;
    g_screenSurf.topDown = 1;
    g_target = g_screenSurf;
    Gfx_FillRect(g_screenBufRect.x, g_screenBufRect.y, g_screenBufRect.w, g_screenBufRect.h, 0);
    Dirty_Init();
}

void Gfx_Shutdown() {
    Dirty_Shutdown();
    Mem_Free(g_backBuffer);
    g_backBuffer = nullptr;
    Mem_Free(g_palette);
    g_palette = nullptr;
}

// ---------------------------------------------------------------- addressing

int Gfx_Offset(int x, int y) {
    if (g_target.topDown) return y * g_target.rect.w + x;
    return ((g_target.rect.h - 1) - y) * g_target.rect.w + x;
}

u8* Gfx_PixelPtr(int x, int y) { return g_target.bits + Gfx_Offset(x, y); }

static int RowSkip(int w) { return g_target.topDown ? g_target.rect.w - w : -w - g_target.rect.w; }

void Gfx_CopyRect(int sx, int sy, int w, int h, int dx, int dy) {
    u8* src = g_target.bits + Gfx_Offset(sx, sy);
    u8* dst = g_target.bits + Gfx_Offset(dx, dy);
    int skip = RowSkip(w);
    Blit_Copy(src, dst, w, h, skip, skip);
}

void Gfx_SaveBg(int x, int y, int w, int h) {
    Rect r = {x, y, w, h};
    if (Rect_Clip(&r, g_clip)) Gfx_CopyRect(r.x, r.y, r.w, r.h, r.x, r.y + g_screenH);
}

void Gfx_RestoreBg(int x, int y, int w, int h) {
    Rect r = {x, y, w, h};
    if (Rect_Clip(&r, g_clip)) Gfx_CopyRect(r.x, r.y + g_screenH, r.w, r.h, r.x, r.y);
}

void Gfx_Copy2x(int sx, int sy, int w, int h, int dx, int dy) {
    u8* src = g_target.bits + Gfx_Offset(sx, sy);
    u8* dst = g_target.bits + Gfx_Offset(dx, dy);
    int srcSkip, dstSkip;
    if (g_target.topDown) {
        srcSkip = g_target.rect.w - w;
        dstSkip = g_target.rect.w - 2 * w;
    } else {
        srcSkip = -w - g_target.rect.w;
        dstSkip = -2 * w - g_target.rect.w;
    }
    Blit_Copy2x(src, dst, w, h, srcSkip, dstSkip);
}

// ---------------------------------------------------------------- primitives

u8 Gfx_GetPixel(int x, int y) {
    const Rect& t = g_target.rect;
    if (!Pt_InRect(x, y, t.x, t.y, t.w, t.h)) return 0;
    return *Gfx_PixelPtr(x, y);
}

void Gfx_SetPixel(int x, int y, u8 c) {
    if (Pt_InRect(x, y, g_clip.x, g_clip.y, g_clip.w, g_clip.h)) *Gfx_PixelPtr(x, y) = c;
}

void Gfx_Line(int x, int y, int dx, int dy, u8 c) {
    Rect l = {x, y, dx, dy};
    if (!Clip_Line(&l, g_clip.x, g_clip.y, g_clip.w, g_clip.h)) return;
    x = l.x;
    y = l.y;
    dx = l.w;
    dy = l.h;
    int step = g_target.topDown ? g_target.rect.w : -g_target.rect.w;
    if (dy == 0) {
        if (dx < 0) {
            x += dx;
            dx = -dx;
        }
        u8* p = Gfx_PixelPtr(x, y);
        for (int i = 0; i <= dx; i++) *p++ = c;
    } else if (dx == 0) {
        if (dy < 0) {
            y += dy;
            dy = -dy;
        }
        u8* p = Gfx_PixelPtr(x, y);
        for (int i = 0; i <= dy; i++, p += step) *p = c;
    } else {
        if (dx < 0) {
            x += dx;
            y += dy;
            dx = -dx;
            dy = -dy;
        }
        if (dy < 0) {
            dy = -dy;
            step = -step;
        }
        u8* p = Gfx_PixelPtr(x, y);
        if (dx < dy) {
            int inc1 = dx * 2, d = inc1 - dy, inc2 = d - dy;
            for (int i = 0; i <= dy; i++) {
                *p = c;
                p += step;
                int inc = inc1;
                if (d >= 0) {
                    p++;
                    inc = inc2;
                }
                d += inc;
            }
        } else {
            int inc1 = dy * 2, d = inc1 - dx, inc2 = d - dx;
            for (int i = 0; i <= dx; i++) {
                *p = c;
                p++;
                int inc = inc1;
                if (d >= 0) {
                    p += step;
                    inc = inc2;
                }
                d += inc;
            }
        }
    }
}

void Gfx_FrameRect(int x, int y, int w, int h, u8 c) {
    Gfx_Line(x, y, w - 1, 0, c);
    Gfx_Line(x, y, 0, h - 1, c);
    Gfx_Line(x, y + h - 1, w - 1, 0, c);
    Gfx_Line(x + w - 1, y, 0, h - 1, c);
}

void Gfx_FillRect(int x, int y, int w, int h, u8 c) {
    Rect r = {x, y, w, h};
    if (!Rect_Clip(&r, g_clip)) return;
    u8* p = Gfx_PixelPtr(r.x, r.y);
    int skip = RowSkip(r.w);
    for (int j = 0; j < r.h; j++) {
        memset(p, c, r.w);
        p += r.w + skip;
    }
}

void Gfx_BlitRaw(int x, int y, const u8* src, int w, int h, int transparent, u8 key) {
    Rect r = {x, y, w, h};
    if (!Rect_Clip(&r, g_clip)) return;
    const u8* s = src + (r.x - x) + (r.y - y) * w;
    int srcSkip = w - r.w;
    u8* dst = g_target.bits + Gfx_Offset(r.x, r.y);
    if (!transparent) Blit_Copy(s, dst, r.w, r.h, srcSkip, RowSkip(r.w));
    else Blit_CopyKey(s, dst, r.w, r.h, srcSkip, RowSkip(r.w), key);
}

void Gfx_RemapRect(int x, int y, int w, int h, const u8* table) {
    Rect r = {x, y, w, h};
    if (!Rect_Clip(&r, g_clip)) return;
    u8* p = Gfx_PixelPtr(r.x, r.y);
    int skip = RowSkip(r.w);
    for (int j = 0; j < r.h; j++) {
        for (int i = 0; i < r.w; i++, p++) *p = table[*p];
        p += skip;
    }
}

void Gfx_InvertRect(int x, int y, int w, int h) {
    u8 t[256];
    for (int i = 0; i < 256; i++) t[i] = (u8)(i ^ 0xff);
    Gfx_RemapRect(x, y, w, h, t);
}

void Gfx_BlitRLE(int x, int y, const u8* data, int w, int h, int transparent, u8 key) {
    Rect r = {x, y, w, h};
    if (!Rect_Clip(&r, g_clip)) return;
    const u8* src = data + RdU32(data + 4 + (r.y - y) * 4);
    int skipL = r.x - x;
    int skipR = (s16)((x + w) - (r.x + r.w));
    u8* dst = g_target.bits + Gfx_Offset(r.x, r.y);
    int dstSkip = RowSkip(r.w);
    if (skipL == 0 && skipR == 0) RLE_Blit(src, dst, r.w, r.h, dstSkip, transparent != 0, key);
    else RLE_BlitClipped(src, dst, skipL, r.w, skipR, r.h, dstSkip, transparent != 0, key);
}

void Gfx_SetTarget(u8* bits, int w, int h) {
    Surf_Init(&g_target, bits, w, h);
    g_clip = g_target.rect;
}

void Gfx_ResetTarget() {
    g_target = g_screenSurf;
    g_clip = g_screenBufRect;
}

// ---------------------------------------------------------------- images

int Img_Size(const void* img) {
    u8 enc = Img_Enc(img);
    if (enc < 2) return Img_W(img) * Img_H(img) + 12;
    if (enc == 2 || enc == 3) return (int)RdU32((const u8*)img + 12) + 12;
    return 0;
}

void Img_FullRect(const void* img, Rect* out) { Rect_Set(out, 0, 0, Img_W(img), Img_H(img)); }

void Img_Bounds(const void* img, Rect* out) {
    const u8* base = (const u8*)img;
    unsigned w = Img_W(img), h = Img_H(img);
    u8 key = Img_Key(img);
    unsigned minX = w, maxX = 0, minY = h, maxY = 0;
    const u8* p = base + 12;
    u8 enc = Img_Enc(img);
    if (enc == 0) {
        for (unsigned y = 0; y < h; y++) {
            bool any = false;
            for (unsigned x = 0; x < w; x++) {
                if (*p++ != key) {
                    any = true;
                    if (x < minX) minX = x;
                    if (maxX < x) maxX = x;
                }
            }
            if (any) {
                if (y < minY) minY = y;
                maxY = y;
            }
        }
    } else if (enc == 1) {
        for (unsigned x = 0; x < w; x++) {
            bool any = false;
            for (unsigned y = 0; y < h; y++) {
                if (*p++ != key) {
                    any = true;
                    if (y < minY) minY = y;
                    if (maxY < y) maxY = y;
                }
            }
            if (any) {
                if (x < minX) minX = x;
                maxX = x;
            }
        }
    } else if (enc == 2 || enc == 3) {
        bool rows = enc == 2;
        unsigned lines = rows ? h : w, len = rows ? w : h;
        const u8* q = base + 12 + RdU32(base + 16);
        for (unsigned line = 0; line < lines; line++) {
            bool any = false;
            unsigned pos = 0;
            while (pos < len) {
                u8 c = *q++;
                unsigned n;
                if (!(c & 0x80)) {
                    n = c;
                    for (unsigned k = 0; k < n; k++) {
                        if (*q++ != key) {
                            any = true;
                            unsigned at = pos + k;
                            if (rows) {
                                if (at < minX) minX = at;
                                if (maxX < at) maxX = at;
                            } else {
                                if (at < minY) minY = at;
                                if (maxY < at) maxY = at;
                            }
                        }
                    }
                } else {
                    n = c & 0x7f;
                    if (*q++ != key) {
                        any = true;
                        unsigned last = pos + n - 1;
                        if (rows) {
                            if (pos < minX) minX = pos;
                            if (maxX < last) maxX = last;
                        } else {
                            if (pos < minY) minY = pos;
                            if (maxY < last) maxY = last;
                        }
                    }
                }
                pos += n;
            }
            if (any) {
                if (rows) {
                    if (line < minY) minY = line;
                    maxY = line;
                } else {
                    if (line < minX) minX = line;
                    maxX = line;
                }
            }
        }
    }
    if (minX < w) Rect_Set(out, minX, minY, maxX - minX + 1, maxY - minY + 1);
    else Rect_Set(out, 0, 0, 0, 0);
}

void Img_Draw(int x, int y, const void* img, int transparent) {
    u8 enc = Img_Enc(img);
    const u8* data = (const u8*)img + 12;
    if (enc == 0) Gfx_BlitRaw(x, y, data, Img_W(img), Img_H(img), transparent, Img_Key(img));
    else if (enc == 2) Gfx_BlitRLE(x, y, data, Img_W(img), Img_H(img), transparent, Img_Key(img));
    Sound_Update();
}

void Img_Capture(int x, int y, int w, int h, void* img) {
    Rect r = {x, y, w, h};
    const Rect& t = g_target.rect;
    bool vis = Rect_Clip(&r, t.x, t.y, t.w, t.h);
    u8* p = (u8*)img;
    memcpy(p, "P800", 4);
    WrU16(p + 4, (u16)r.w);
    WrU16(p + 6, (u16)r.h);
    p[8] = p[9] = p[10] = p[11] = 0;
    if (vis) {
        const u8* src = g_target.bits + Gfx_Offset(r.x, r.y);
        Blit_Copy(src, p + 12, r.w, r.h, RowSkip(r.w), 0);
    }
}

int Img_RawSize(int, int, int w, int h) { return w * h + 12; }

void Img_DrawOpaque(int x, int y, const void* img) { Img_Draw(x, y, img, 0); }

void* Img_ToDIBAlloc(const void* img) {
    int w = Img_W(img), h = Img_H(img);
    int pad = (4 - (w & 3)) & 3;
    u8* p = (u8*)Mem_Alloc((w + pad) * h + 0x428);
    WrU32(p, 0x28);
    WrU32(p + 4, (u32)w);
    WrU32(p + 8, (u32)h);
    WrU16(p + 12, 1);
    WrU16(p + 14, 8);
    WrU32(p + 20, (u32)((w + pad) * h));
    Pal_RGBToQuads(g_palette, p + 0x28);
    const u8* src = (const u8*)img + 12 + (h - 1) * w;
    u8* dst = p + 0x428;
    for (int y = 0; y < h; y++) {
        memcpy(dst, src, w);
        src -= w;
        dst += w + pad;
    }
    return p;
}

void Img_Remap(void* img, const u8* table) {
    u8* p = Img_Bits(img);
    unsigned w = Img_W(img), h = Img_H(img);
    u8 enc = Img_Enc(img);
    if (enc < 2) {
        for (unsigned i = 0; i < w * h; i++) p[i] = table[p[i]];
        return;
    }
    if (enc != 2 && enc != 3) return;
    unsigned lines = enc == 2 ? h : w, len = enc == 2 ? w : h;
    u8* q = p + RdU32(p + 4);
    for (unsigned line = 0; line < lines; line++) {
        unsigned pos = 0;
        while (pos < len) {
            u8 c = *q++;
            if (!(c & 0x80)) {
                for (unsigned k = 0; k < c; k++, q++) *q = table[*q];
            } else {
                c &= 0x7f;
                *q = table[*q];
                q++;
            }
            pos += c;
        }
    }
}

void* Img_Create(int w, int h) {
    u8* p = (u8*)Mem_Alloc(w * h + 12);
    memcpy(p, "P800", 4);
    WrU16(p + 4, (u16)w);
    WrU16(p + 6, (u16)h);
    p[8] = p[9] = p[10] = p[11] = 0;
    return p;
}

void Surf_InitImg(Surface* s, void* img) { Surf_Init(s, Img_Bits(img), Img_W(img), Img_H(img)); }

void* Img_Decompress(const void* img) {
    void* out = Img_Create(Img_W(img), Img_H(img));
    ((u8*)out)[8] = Img_Key(img);
    Surface saved = g_target;
    Surf_InitImg(&g_target, out);
    Img_Draw(0, 0, img, 0);
    g_target = saved;
    return out;
}

void Gfx_SetTargetImg(void* img) { Gfx_SetTarget(Img_Bits(img), Img_W(img), Img_H(img)); }

// ---------------------------------------------------------------- palette

// Windows' 20 static colors, used when mode == 0 (palette with system colors).
static const u8 kStaticLow[10][3] = {{0, 0, 0},       {128, 0, 0},     {0, 128, 0},     {128, 128, 0},
                                     {0, 0, 128},     {128, 0, 128},   {0, 128, 128},   {192, 192, 192},
                                     {192, 220, 192}, {166, 202, 240}};
static const u8 kStaticHigh[10][3] = {{255, 251, 240}, {160, 160, 164}, {128, 128, 128}, {255, 0, 0},
                                      {0, 255, 0},     {255, 255, 0},   {0, 0, 255},     {255, 0, 255},
                                      {0, 255, 255},   {255, 255, 255}};

void Pal_Set(const u8* rgb, int mode) {
    if (!g_palette) g_palette = (u8*)Mem_Alloc(768);
    u8 pal[768];
    int reserved;
    if (mode == 0) {
        reserved = 20;
        memcpy(pal, kStaticLow, 30);
        memcpy(pal + 246 * 3, kStaticHigh, 30);
    } else {
        reserved = 2;
        pal[0] = pal[1] = pal[2] = 0;
        pal[765] = pal[766] = pal[767] = 255;
    }
    for (int i = reserved / 2; i < 256 - reserved / 2; i++) memcpy(pal + i * 3, rgb + i * 3, 3);
    memcpy(g_palette, pal, 768);
    Sys_SetDisplayPalette(g_palette);
}

void Pal_QuadsToRGB(const u8* q, u8* rgb) {
    for (int i = 0; i < 256; i++) {
        rgb[i * 3] = q[i * 4 + 2];
        rgb[i * 3 + 1] = q[i * 4 + 1];
        rgb[i * 3 + 2] = q[i * 4];
    }
}

void Pal_RGBToQuads(const u8* rgb, u8* q) {
    for (int i = 0; i < 256; i++) {
        q[i * 4 + 2] = rgb[i * 3];
        q[i * 4 + 1] = rgb[i * 3 + 1];
        q[i * 4] = rgb[i * 3 + 2];
        q[i * 4 + 3] = 0;
    }
}

void Pal_SetQuads(const u8* q, int mode) {
    u8 rgb[768];
    Pal_QuadsToRGB(q, rgb);
    Pal_Set(rgb, mode);
}

void Pal_SetBlack(int mode) {
    u8 z[768] = {0};
    Pal_Set(z, mode);
}

void Pal_InitBlack() { Pal_SetBlack(0); }

void Pal_FadeTo(const u8* target, int mode) {
    if (!g_palette) Pal_SetBlack(mode);
    if (g_fastQuit) {
        Pal_Set(target, mode);
        return;
    }
    for (int step = 0; step < 10; step++) {
        Timer_Set(0, 100);
        int k = 10 - step;
        u8 next[768];
        for (int i = 0; i < 768; i++) {
            int cur = g_palette[i], tgt = target[i];
            int v;
            if (cur < tgt) v = (tgt - k * 26 < cur) ? cur : tgt - k * 26;
            else v = (cur - 26 < tgt) ? tgt : cur - 26;
            next[i] = (u8)v;
        }
        Pal_Set(next, mode);
        Timer_Wait(0);
    }
    Pal_Set(target, mode);
}

void Pal_FadeToBlack(int mode) {
    u8 z[768] = {0};
    Pal_FadeTo(z, mode);
}

// ---------------------------------------------------------------- screen

void Screen_Flush(int x, int y, int w, int h) {
    Rect r = {x, y, w, h};
    if (!Rect_Clip(&r, g_viewRect)) return;
    u8* disp = Sys_Display();
    const u8* src = g_screenSurf.bits + r.y * g_screenSurf.rect.w + r.x;
    u8* dst = disp + r.y * kScreenW + r.x;
    for (int j = 0; j < r.h; j++) {
        memcpy(dst, src, r.w);
        src += g_screenSurf.rect.w;
        dst += kScreenW;
    }
    Sys_DisplayChanged();
}

void Screen_FlushAll() { Screen_Flush(g_viewRect); }

void Screen_Flush2x(int x, int y, int w, int h) {
    // Only reachable from the WM_PAINT path of the doubled-video mode, whose
    // offset rect (0x437954) is never set; flush the doubled area directly.
    Screen_Flush(x * 2, y * 2, w * 2, h * 2);
}

// ---------------------------------------------------------------- dirty grid

static u8* g_dirty;
static int g_dirtyCols, g_dirtyRows, g_dirtyRuns;

void Dirty_Init() {
    g_dirtyCols = (g_screenW + 15) >> 4;
    g_dirtyRows = (g_screenH + 9) / 10;
    if (!g_dirty) g_dirty = (u8*)Mem_Alloc(g_dirtyCols * g_dirtyRows);
    Dirty_Clear();
}

void Dirty_Shutdown() {
    Mem_Free(g_dirty);
    g_dirty = nullptr;
}

void Dirty_Mark(int x, int y, int w, int h) {
    if (!g_dirty) return;
    int c0 = x / 16, r0 = y / 10;
    int c1 = (x + w - 1) / 16, r1 = (y + h - 1) / 10;
    if (c0 < 0) c0 = 0;
    if (r0 < 0) r0 = 0;
    if (g_dirtyCols <= c1) c1 = g_dirtyCols - 1;
    if (g_dirtyRows <= r1) r1 = g_dirtyRows - 1;
    for (int r = r0; r <= r1; r++) {
        u8* row = g_dirty + r * g_dirtyCols;
        for (int c = c0; c <= c1; c++) {
            if (row[c] != 0) continue;
            row[c] |= 1;
            if (c != 0 && row[c - 1] != 0) {
                row[c] |= 4;
                row[c - 1] |= 2;
            }
            if (c != g_dirtyCols - 1 && row[c + 1] != 0) {
                row[c] |= 2;
                row[c + 1] |= 4;
            }
            if (row[c] == 1) g_dirtyRuns++;
            else if (row[c] == 7) g_dirtyRuns--;
        }
    }
}

void Dirty_Clear() {
    if (!g_dirty) return;
    g_dirtyRuns = 0;
    memset(g_dirty, 0, g_dirtyCols * g_dirtyRows);
}

void Dirty_Flush() {
    if (g_dirtyRuns == 0) return;
    if (g_dirtyRuns < 20) {
        const u8* p = g_dirty;
        Rect run = {0, 0, 0, 0};
        for (int r = 0; r < g_dirtyRows; r++) {
            for (int c = 0; c < g_dirtyCols; c++) {
                u8 v = *p++;
                if (v == 1) {
                    Rect_Set(&run, c << 4, r * 10, 16, 10);
                    Screen_Flush(run);
                } else if (v == 3) {
                    Rect_Set(&run, c << 4, r * 10, 16, 10);
                } else if (v == 5) {
                    run.w += 16;
                    Screen_Flush(run);
                } else if (v == 7) {
                    run.w += 16;
                }
            }
        }
    } else {
        int minC = g_dirtyCols, minR = g_dirtyRows, maxC = -1, maxR = -1;
        const u8* p = g_dirty;
        for (int r = 0; r < g_dirtyRows; r++) {
            for (int c = 0; c < g_dirtyCols; c++) {
                if (*p++ == 0) continue;
                if (c < minC) minC = c;
                if (maxC < c) maxC = c;
                if (r < minR) minR = r;
                if (maxR < r) maxR = r;
            }
        }
        Screen_Flush(minC << 4, minR * 10, (maxC - minC + 1) * 16, (maxR - minR + 1) * 10);
    }
    Dirty_Clear();
}

void Img_DrawDirty(int x, int y, const void* img, int transparent) {
    Img_Draw(x, y, img, transparent);
    Dirty_Mark(x, y, Img_W(img), Img_H(img));
}

void Gfx_RestoreBgDirty(int x, int y, int w, int h) {
    Gfx_RestoreBg(x, y, w, h);
    Dirty_Mark(x, y, w, h);
}

// ---------------------------------------------------------------- transitions

void Fx_FlushLine(int x1, int y1, int x2, int y2) {
    Rect l;
    Rect_SetLTRB(&l, x1, y1, x2, y2);
    if (l.h == 0) {
        Screen_Flush(l.x, l.y, l.w + 1, 1);
        return;
    }
    if (l.w == 0) {
        Screen_Flush(l.x, l.y, 1, l.h + 1);
        return;
    }
    int sy = 1;
    if (l.w < 0) {
        l.x += l.w;
        l.y += l.h;
        l.w = -l.w;
        l.h = -l.h;
    }
    if (l.h < 0) {
        l.h = -l.h;
        sy = -1;
    }
    int px = l.x, py = l.y;
    if (l.w < l.h) {
        int inc1 = l.w * 2, d = inc1 - l.h, inc2 = d - l.h;
        for (int i = 0; i <= l.h; i++) {
            Screen_Flush(px, py, 1, 1);
            py += sy;
            int inc = inc1;
            if (d >= 0) {
                px++;
                inc = inc2;
            }
            d += inc;
        }
    } else {
        int inc1 = l.h * 2, d = inc1 - l.w, inc2 = d - l.w;
        for (int i = 0; i <= l.w; i++) {
            Screen_Flush(px, py, 1, 1);
            px++;
            int inc = inc1;
            if (d >= 0) {
                py += sy;
                inc = inc2;
            }
            d += inc;
        }
    }
}

// Maximal-length LFSR tap masks indexed by bit count (0x43797c).
static const u32 kLfsrTaps[20] = {0x0,    0x1,    0x3,    0x6,    0xc,     0x14,   0x30,
                                  0x60,   0xb8,   0x110,  0x240,  0x500,   0xca0,  0x1b00,
                                  0x3500, 0x6000, 0xb400, 0x12000, 0x20400, 0x72000};

void Fx_Dissolve(int x, int y, int w, int h, int cw, int ch) {
    unsigned cols = (unsigned)(w / cw);
    int rows = h / ch;
    unsigned n = cols * rows;
    int bits = 0;
    for (unsigned v = n - 1; v; v >>= 1) bits++;
    u32 taps = kLfsrTaps[bits];
    // The original ran as fast as the blits allowed; pace it so it is visible.
    const u32 duration = 400;
    u32 start = Time_Get();
    unsigned done = 0;
    u32 v = 1;
    do {
        if ((int)(v / cols) < rows) {
            Screen_Flush((v % cols) * cw + x, (v / cols) * ch + y, cw, ch);
            if ((++done & 31) == 0) {
                u32 due = start + duration * done / n;
                while (Time_Get() < due) {
                }
            }
        }
        v = (v & 1) ? (v >> 1) ^ taps : v >> 1;
    } while (v != 1);
    Screen_Flush(x, y, cw, ch);
}

void Fx_WipeRight(int x, int y, int w, int h, int ms) {
    u32 t0 = Time_Get();
    int cx = x;
    for (int i = 0; i < w; i++) {
        Fx_FlushLine(cx, y, cx, y + h - 1);
        while (Time_Get() < (u32)((ms * i) / w) + t0) {
        }
        cx++;
    }
}

void Fx_WipeLeft(int x, int y, int w, int h, int ms) {
    u32 t0 = Time_Get();
    int cx = x + w;
    for (int i = 0; cx--, i < w; i++) {
        Fx_FlushLine(cx, y, cx, y + h - 1);
        while (Time_Get() < (u32)((ms * i) / w) + t0) {
        }
    }
}

void Fx_WipeDown(int x, int y, int w, int h) {
    for (int i = 0; i < h; i++) {
        Fx_FlushLine(x, y, x + w - 1, y);
        y++;
    }
}

void Fx_WipeUp(int x, int y, int w, int h) {
    int yy = y + h;
    for (int i = 0; yy--, i < h; i++) Fx_FlushLine(x, yy, x + w - 1, yy);
}

void Fx_WipeDiagonal(int x, int y, int w, int h) {
    int m = h < w ? h : w;
    int cx = x + w - 1;
    int i = 0;
    int len = 1;
    for (; i < m; i++, len++) {
        Fx_FlushLine(cx, y, len - 1 + cx, len - 1 + y);
        cx--;
    }
    for (; i < w; i++) {
        Fx_FlushLine(cx, y, m - 1 + cx, m - 1 + y);
        cx--;
    }
    cx++;
    for (; i < h; i++) {
        Fx_FlushLine(cx, y, m - 1 + cx, m - 1 + y);
        y++;
    }
    len = m;
    for (int k = 0; k < m; k++, len--) {
        Fx_FlushLine(cx, y, len - 1 + cx, len - 1 + y);
        y++;
    }
}

void Fx_WipeVCenterIn(int x, int y, int w, int h) {
    int bottom = y + h;
    for (int i = 0;; i++) {
        bottom--;
        if ((h / 2 + (h & 1)) <= i) break;
        Fx_FlushLine(x, y, x + w - 1, y);
        Fx_FlushLine(x, bottom, x + w - 1, bottom);
        y++;
    }
}

void Fx_BoxOut(int x, int y, int w, int h, int ms) {
    int steps = (h < w) ? (w + 1) / 2 : (h + 1) / 2;
    u32 t0 = Time_Get();
    Rect r = {x + w / 2, y + h / 2, 1, 1};
    Screen_Flush(r);
    for (int i = 1;; i++) {
        if (x < r.x) r.x--;
        if (y < r.y) r.y--;
        r.w += 2;
        if (w < r.w) r.w = w;
        r.h += 2;
        if (h < r.h) r.h = h;
        Screen_Flush(r.x, r.y, r.w, 1);
        Screen_Flush(r.x, r.y, 1, r.h);
        Screen_Flush(r.x, r.y + r.h - 1, r.w, 1);
        Screen_Flush(r.x + r.w - 1, r.y, 1, r.h);
        while (Time_Get() < (u32)((ms * i) / steps) + t0) {
        }
        if (r.x == x && r.y == y && r.w == w && r.h == h) break;
    }
}

void Fx_CloseBlack() {
    u32 t0 = Time_Get();
    int w = g_screenW;
    unsigned half = (unsigned)g_screenH >> 1;
    int bottom = g_screenH, top = 0;
    for (unsigned i = 0; i <= half; i++) {
        Gfx_FillRect(0, bottom, w, 1, 0);
        Gfx_FillRect(0, top, w, 1, 0);
        Screen_Flush(0, top, w, 1);
        Screen_Flush(0, bottom, w, 1);
        bottom--;
        top++;
        while (Time_Get() < (i * 1000) / half + t0) {
        }
    }
}

void Fx_OpenCenter() {
    u32 t0 = Time_Get();
    int w = g_screenW;
    unsigned half = (unsigned)g_screenH >> 1;
    int down = (int)half, up = (int)half;
    for (unsigned i = 0; up--, i <= half; i++) {
        Screen_Flush(0, down, w, 1);
        Screen_Flush(0, up, w, 1);
        down++;
        while (Time_Get() < (i * 1000) / half + t0) {
        }
    }
}
