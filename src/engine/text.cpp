#include "engine/text.h"

#include "engine/gfx.h"

const void* g_curFont;
int g_textOverflow;
int g_textFirstLine;
int g_textColumnGap;

namespace {

// Layout state (the 0x54-byte struct the original passes around).
struct TextBox {
    FontWidthFn width;
    FontHeightFn height;
    FontDrawFn draw;
    FontDrawFn drawSel;
    const u8* text;
    int x, y, w, h;
    int color;
    int bgColor;
    int center;
    u16 maxLines;
    const u8** lineStart; // maxLines + 1
    s16* lineX;           // maxLines
    u16 capacity;
    int* charPos;
    u16 maxWidth;
    u16 numLines;
    const u8* selStart;
    const u8* selEnd;
};

int WordWidth(const u8* p, FontWidthFn wfn) { // 0x423211
    int w = 0;
    for (;;) {
        u8 c = *p;
        if (c == 0 || c == ' ' || c == '\t' || c == '\n' || c == '\r') break;
        if (c == 0xff) {
            const u8* q = p + (p[1] == 3 ? 3 : 2);
            while (*q++ != 0xff) {
            }
            p = q;
        } else {
            w += wfn(c);
            p++;
        }
    }
    return w;
}

void Layout(TextBox* t) { // 0x422ed3
    int lineW = 0;
    int* widths = (int*)Mem_Alloc(t->maxLines * sizeof(int));
    int* centered = (int*)Mem_Alloc(t->maxLines * sizeof(int));
    int center = t->center;
    for (int i = 0; i < t->maxLines; i++) {
        widths[i] = 0;
        centered[i] = center;
    }
    int space = t->width(' ');
    u16 tabs[80];
    u16 acc = 0;
    for (int i = 0; i < 80; i++) {
        acc += (s16)space * 4;
        tabs[i] = acc;
    }
    const u8* p = t->text;
    int line = 0;
    int x = 0;
    int brk = 0;
    t->lineStart[0] = p;
    t->maxWidth = 0;
    u8 prev = 0;
    const int first = g_textFirstLine;
    for (;;) {
        u8 c = *p;
        const u8* next = p + 1;
        if (c == 0) {
            if (first <= line && line - first < t->maxLines) {
                widths[line - first] = lineW;
                centered[line - first] = center;
            }
            if (brk == 0 || (prev != ' ' && prev != '\t')) line++;
            if (first <= line && line - first <= t->maxLines) t->lineStart[line - first] = p;
            t->numLines = (u16)(line - first);
            if ((int)t->maxWidth < lineW) t->maxWidth = (u16)lineW;
            for (int i = 0; i < t->numLines && i < t->maxLines; i++) {
                if (centered[i] == 0) t->lineX[i] = (s16)t->x;
                else t->lineX[i] = (s16)((t->w - widths[i]) / 2 + t->x);
            }
            Mem_Free(widths);
            Mem_Free(centered);
            return;
        }
        brk = 0;
        lineW = x;
        bool space_ = false, normal = false;
        if (c < 0xe) {
            if (c == 0xd) brk = 1;
            else if (c == 9) space_ = true;
            else if (c == 10) brk = (prev != 0xd); // LF right after CR is ignored
            else normal = true;
        } else if (c == ' ') {
            space_ = true;
        } else if (c != 0xff) {
            normal = true;
        }
        if (space_) {
            const u8* q = next;
            for (;;) {
                if (c == ' ') {
                    x += space;
                } else {
                    for (int i = 0; i < 80; i++) {
                        if (x < (int)tabs[i]) {
                            x = tabs[i];
                            break;
                        }
                    }
                }
                next = q;
                c = *q++;
                if (c != ' ' && c != 9) break;
            }
            int ww = WordWidth(next, t->width);
            if (t->w < ww + x) brk = 1;
            c = next[-1];
        } else if (c == 0xff) {
            u8 code = *next;
            next = p + 2;
            if (code == 3) {
                next = p + 3;
            } else if (code == 4) {
                int i = 0;
                for (; *next != 0xff && i < 80; i++) {
                    tabs[i] = RdU16(next);
                    next += 2;
                }
                for (; i < 80; i++) tabs[i] = (u16)t->w;
            } else if (code == 5) {
                center = center == 0;
            }
            while (*next++ != 0xff) {
            }
        } else if (normal) {
            lineW = x + t->width(c);
            x = lineW;
        }
        p = next;
        prev = c;
        if (brk) {
            if (first <= line && line - first < t->maxLines) {
                widths[line - first] = lineW;
                centered[line - first] = center;
            }
            line++;
            if (first <= line && line - first <= t->maxLines) t->lineStart[line - first] = next;
            x = 0;
            if ((int)t->maxWidth < lineW) t->maxWidth = (u16)lineW;
        }
    }
}

int DrawChar(int ch, int x, int y, int underline, int color, FontDrawFn dfn, FontHeightFn hfn) { // 0x4234c9
    int w = dfn(x, y, ch, color);
    if (underline) Gfx_Line(x, hfn() + y - 1, w - 1, 0, (u8)color);
    return w;
}

void Render(TextBox* t) { // 0x423264
    int underline = 0;
    int lineH = t->height();
    s16 tabStep = (s16)(t->width(' ') * 4);
    u16 tabs[80];
    u16 acc = 0;
    for (int i = 0; i < 80; i++) {
        acc += tabStep;
        tabs[i] = acc;
    }
    if (t->charPos) {
        for (int i = 0; i < t->capacity; i++) t->charPos[i * 2] = -10000;
    }
    int color = t->color;
    const u8* selLo = t->selStart < t->selEnd ? t->selStart : t->selEnd;
    const u8* selHi = t->selStart < t->selEnd ? t->selEnd : t->selStart;
    int y = t->y;
    const u8* last = t->text;
    int x = 0;
    for (int line = 0;; line++) {
        if (t->numLines <= line || t->maxLines <= line) {
            if (t->charPos) {
                t->charPos[(last - t->text) * 2] = x;
                t->charPos[(last - t->text) * 2 + 1] = y - t->height();
            }
            return;
        }
        x = t->lineX[line];
        const u8* p = t->lineStart[line];
        const u8* end = t->lineStart[line + 1];
        while ((last = p) != end) {
            if (t->charPos) {
                t->charPos[(p - t->text) * 2] = x;
                t->charPos[(p - t->text) * 2 + 1] = y;
            }
            u8 c = *p++;
            if (c == 9) {
                for (int i = 0; i < 80; i++) {
                    if (x < (int)tabs[i] + t->x) {
                        x = tabs[i] + t->x;
                        break;
                    }
                }
            } else if (c != 10 && c != 13) {
                if (c == 0xff) {
                    u8 code = *p;
                    const u8* q = p + 1;
                    if (code == 2) {
                        underline = underline == 0;
                    } else if (code == 3) {
                        color = *q - 1;
                        q++;
                    } else if (code == 4) {
                        for (int i = 0; *q != 0xff && i < 80; i++) {
                            tabs[i] = RdU16(q);
                            q += 2;
                        }
                    }
                    while (*q++ != 0xff) {
                    }
                    p = q;
                } else if (selLo < p && p <= selHi) {
                    Gfx_FillRect(x, y, t->width(c), t->height(), (u8)t->color);
                    x += DrawChar(c, x, y, underline, (s8)t->bgColor, t->drawSel, t->height);
                } else {
                    x += DrawChar(c, x, y, underline, (s8)color, t->draw, t->height);
                }
            }
        }
        y += lineH;
    }
}

void InitBox(TextBox* t, const char* text, int x, int y, int w, int h, int center, int color, FontWidthFn wfn,
             FontHeightFn hfn, FontDrawFn dfn, int maxLines) {
    memset(t, 0, sizeof *t);
    t->width = wfn;
    t->height = hfn;
    t->draw = dfn;
    t->drawSel = dfn;
    t->text = (const u8*)text;
    t->x = x;
    t->y = y;
    t->w = w;
    t->h = h;
    t->color = color;
    t->center = center;
    t->maxLines = (u16)maxLines;
    t->lineStart = (const u8**)Mem_Alloc((t->maxLines + 1) * sizeof(u8*));
    t->lineX = (s16*)Mem_Alloc((t->maxLines + 1) * sizeof(s16));
}

void FreeBox(TextBox* t) {
    Mem_Free(t->lineStart);
    t->lineStart = nullptr;
    Mem_Free(t->lineX);
    t->lineX = nullptr;
}

int CurWidth(int ch) { return Font_CharWidth(ch, g_curFont); }     // 0x4238bd
int CurHeight() { return Font_Height(g_curFont); }                  // 0x4238d3
int CurDraw(int x, int y, int ch, int color) {                      // 0x4238e4
    return Font_DrawChar(x, y, ch, g_curFont, color);
}

} // namespace

int Font_CharWidth(int ch, const void* font) { return RdU16((const u8*)font + 6 + (ch & 0xff) * 2); }
int Font_Height(const void* font) { return RdU16((const u8*)font + 4); }

int Font_DrawChar(int x, int y, int ch, const void* font, int color) {
    const u8* f = (const u8*)font;
    ch &= 0xff;
    int w = RdU16(f + 6 + ch * 2);
    if (w == 0) return 0;
    int h = RdU16(f + 4);
    const u8* bits = f + 0x406 + RdU16(f + 0x206 + ch * 2);
    if (!Rect_Inside(x, y, w, h, g_clip.x, g_clip.y, g_clip.w, g_clip.h)) return w;
    u8* dst = Gfx_PixelPtr(x, y);
    int skip = g_target.topDown ? g_target.rect.w - w : -w - g_target.rect.w;
    int full = w / 8, rest = w % 8;
    for (int row = 0; row < h; row++) {
        for (int b = 0; b < full; b++) {
            u8 v = *bits++;
            for (int i = 0; i < 8; i++, v <<= 1) {
                if (v & 0x80) *dst = (u8)color;
                dst++;
            }
        }
        if (rest) {
            u8 v = *bits++;
            for (int i = 0; i < rest; i++, v <<= 1) {
                if (v & 0x80) *dst = (u8)color;
                dst++;
            }
        }
        dst += skip;
    }
    return w;
}

void Text_DrawBox(const char* text, int x, int y, int w, int h, int center, int color, FontWidthFn wfn,
                  FontHeightFn hfn, FontDrawFn dfn) {
    TextBox t;
    int lineH = hfn();
    InitBox(&t, text, x, y, w, h, center, color, wfn, hfn, dfn, h / lineH + 1);
    g_textFirstLine = 0;
    Layout(&t);
    if (center) t.y += (t.h - t.numLines * lineH) / 2;
    if (g_textOverflow < 0 && t.numLines >= t.maxLines) {
        g_textFirstLine = t.maxLines - 1;
        FreeBox(&t);
        return;
    }
    Render(&t);
    if (g_textOverflow != 0 && t.numLines >= t.maxLines) {
        g_textFirstLine = t.maxLines - 1;
        t.x += t.w + g_textColumnGap;
        Layout(&t);
        Render(&t);
    }
    FreeBox(&t);
}

void Text_Measure(const char* text, FontWidthFn wfn, FontHeightFn hfn, u32* outW, int* outH) {
    TextBox t;
    int lineH = hfn();
    extern int g_screenH;
    InitBox(&t, text, 0, 0, 0x7fffffff, 0x7fffffff, 0, 0, wfn, hfn, nullptr, g_screenH / lineH + 1);
    Layout(&t);
    if (outW) *outW = t.maxWidth;
    if (outH) *outH = t.numLines * lineH;
    FreeBox(&t);
}

int Text_MeasureHeight(const char* text, FontWidthFn wfn, FontHeightFn hfn, int width) {
    TextBox t;
    int lineH = hfn();
    extern int g_screenH;
    InitBox(&t, text, 0, 0, width, 0x7fffffff, 0, 0, wfn, hfn, nullptr, g_screenH / lineH + 1);
    Layout(&t);
    int h = t.numLines * lineH;
    FreeBox(&t);
    return h;
}

void Text_DrawFont(const char* text, int x, int y, int w, int h, const void* font, int color, int center) {
    g_curFont = font;
    Text_DrawBox(text, x, y, w, h, center, color, CurWidth, CurHeight, CurDraw);
}

void Text_DrawScreenFont(int x, int y, const char* text, const void* font, int color) {
    g_curFont = font;
    Text_DrawBox(text, x, y, g_screenW, g_screenH, 0, color, CurWidth, CurHeight, CurDraw);
}

void Text_DrawBoxFont(int x, int y, int w, int h, const char* text, const void* font, int color) {
    g_curFont = font;
    Text_DrawBox(text, x, y, w, h, 1, color, CurWidth, CurHeight, CurDraw);
}

void Text_MeasureFont(const char* text, const void* font, u32* outW, int* outH) {
    g_curFont = font;
    Text_Measure(text, CurWidth, CurHeight, outW, outH);
}

int Text_MeasureHeightFont(const char* text, const void* font, int width) {
    g_curFont = font;
    return Text_MeasureHeight(text, CurWidth, CurHeight, width);
}
