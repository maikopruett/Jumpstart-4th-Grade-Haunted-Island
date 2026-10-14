#include "engine/core.h"

#include <cstdio>

void* Mem_Alloc(size_t n) {
    void* p = calloc(1, n ? n : 1);
    if (!p) {
        fprintf(stderr, "out of memory (%zu bytes)\n", n);
        abort();
    }
    return p;
}

void Mem_Free(const void* p) { free(const_cast<void*>(p)); }

// ---------------------------------------------------------------- lists

void List_Init(List* l, int elemSize) {
    ListNode* h = (ListNode*)Mem_Alloc(sizeof(ListNode));
    h->data = nullptr;
    h->prev = h;
    h->next = h;
    l->head = h;
    l->cur = h;
    l->elemSize = elemSize;
    l->count = 0;
}

void List_Destroy(List* l) {
    if (!l->head) return;
    List_Clear(l);
    Mem_Free(l->head);
    l->head = nullptr;
}

bool List_IsEmpty(const List* l) { return l->count == 0; }
int List_Count(const List* l) { return l->count; }
bool List_CurIsFirst(const List* l) { return l->head->next == l->cur; }
bool List_CurIsLast(const List* l) { return l->head->prev == l->cur; }
bool List_AtEnd(const List* l) { return l->cur == l->head; }
void List_First(List* l) { l->cur = l->head->next; }
void List_Last(List* l) { l->cur = l->head->prev; }
void List_Next(List* l) { l->cur = l->cur->next; }
void List_Prev(List* l) { l->cur = l->cur->prev; }

bool List_GetCur(List* l, void* out) {
    if (l->cur == l->head || l->elemSize == 0) return false;
    memcpy(out, l->cur->data, l->elemSize);
    return true;
}

void* List_CurData(List* l) { return l->cur->data; }

static ListNode* NewNode(List* l, const void* data) {
    ListNode* n = (ListNode*)Mem_Alloc(sizeof(ListNode));
    if (l->elemSize == 0) {
        n->data = const_cast<void*>(data);
    } else {
        n->data = Mem_Alloc(l->elemSize);
        memcpy(n->data, data, l->elemSize);
    }
    return n;
}

void List_InsertAfter(List* l, const void* data) {
    ListNode* n = NewNode(l, data);
    n->prev = l->cur;
    n->next = l->cur->next;
    l->cur->next->prev = n;
    l->cur->next = n;
    l->cur = n;
    l->count++;
}

void List_InsertBefore(List* l, const void* data) {
    ListNode* n = NewNode(l, data);
    n->prev = l->cur->prev;
    n->next = l->cur;
    l->cur->prev->next = n;
    l->cur->prev = n;
    l->cur = n;
    l->count++;
}

void List_PushFront(List* l, const void* data) {
    ListNode* n = NewNode(l, data);
    n->prev = l->head;
    n->next = l->head->next;
    l->head->next->prev = n;
    l->head->next = n;
    l->count++;
}

void List_PushBack(List* l, const void* data) {
    ListNode* n = NewNode(l, data);
    n->prev = l->head->prev;
    n->next = l->head;
    l->head->prev->next = n;
    l->head->prev = n;
    l->count++;
}

void List_InsertSorted(List* l, const void* data, ListCmp cmp) {
    ListNode* n = NewNode(l, data);
    ListNode* at = l->head->next;
    while (at != l->head && cmp(at->data, data) < 0) at = at->next;
    n->prev = at->prev;
    n->next = at;
    at->prev->next = n;
    at->prev = n;
    l->cur = n;
    l->count++;
}

bool List_RemoveCur(List* l) {
    if (l->count == 0) return false;
    ListNode* n = l->cur;
    n->prev->next = n->next;
    n->next->prev = n->prev;
    l->cur = n->next;
    if (l->elemSize != 0) Mem_Free(n->data);
    Mem_Free(n);
    l->count--;
    return true;
}

void List_Clear(List* l) {
    if (l->count != 0) {
        ListNode* n = l->head->next;
        while (n != l->head) {
            ListNode* next = n->next;
            if (l->elemSize != 0) Mem_Free(n->data);
            Mem_Free(n);
            n = next;
        }
        l->head->prev = l->head;
        l->head->next = l->head;
        l->count = 0;
    }
    l->cur = l->head;
}

bool List_PopFront(List* l, void* out) {
    if (List_IsEmpty(l)) return false;
    List_First(l);
    List_GetCur(l, out);
    List_RemoveCur(l);
    return true;
}

// ---------------------------------------------------------------- rects

void Rect_FromPoints(Rect* r, int x1, int y1, int x2, int y2) {
    r->x = x1 < x2 ? x1 : x2;
    r->y = y1 < y2 ? y1 : y2;
    r->w = abs(x1 - x2) + 1;
    r->h = abs(y1 - y2) + 1;
}

void Rect_FromLTRB(int l, int t, int r, int b, Rect* out) {
    out->x = l;
    out->y = t;
    out->w = r - l;
    out->h = b - t;
}

void Rect_ToLTRB(int x, int y, int w, int h, int* out4) {
    out4[0] = x;
    out4[1] = y;
    out4[2] = x + w;
    out4[3] = y + h;
}

void Rect_SetLTRB(Rect* r, int l, int t, int rr, int b) {
    r->x = l;
    r->y = t;
    r->w = rr - l;
    r->h = b - t;
}

bool Pt_InRect(int px, int py, int x, int y, int w, int h) {
    return !(px < x || x + w <= px || py < y || y + h <= py);
}

bool Rect_Inside(int x, int y, int w, int h, int cx, int cy, int cw, int ch) {
    return !(x < cx || cx + cw < x + w || y < cy || cy + ch < y + h);
}

bool Rect_InsideStrict(int x, int y, int w, int h, int cx, int cy, int cw, int ch) {
    return !(x < cx || x + w < cx || cx + cw <= x || cx + cw <= x + w || y < cy || y + h < cy ||
             cy + ch <= y || cy + ch <= y + h);
}

bool Rect_Intersect(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2, Rect* out) {
    Rect r;
    r.x = x2 < x1 ? x1 : x2;
    r.y = y2 < y1 ? y1 : y2;
    int right = (x2 + w2 <= x1 + w1) ? x2 + w2 : x1 + w1;
    int bottom = (y2 + h2 <= y1 + h1) ? y2 + h2 : y1 + h1;
    r.w = right - r.x;
    r.h = bottom - r.y;
    if (out) *out = r;
    return r.w >= 1 && r.h >= 1;
}

bool Rect_Clip(Rect* r, int x, int y, int w, int h) {
    return Rect_Intersect(r->x, r->y, r->w, r->h, x, y, w, h, r);
}

void Rect_Union(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2, Rect* out) {
    out->x = x1 < x2 ? x1 : x2;
    out->y = y1 < y2 ? y1 : y2;
    int right = (x1 + w1 <= x2 + w2) ? x2 + w2 : x1 + w1;
    int bottom = (y1 + h1 <= y2 + h2) ? y2 + h2 : y1 + h1;
    out->w = right - out->x;
    out->h = bottom - out->y;
}

static int OutCode(int px, int py, int x, int y, int w, int h) {
    int c = px < x ? 1 : 0;
    if (py < y) c |= 2;
    if (x + w <= px) c |= 4;
    if (y + h <= py) c |= 8;
    return c;
}

// Cohen-Sutherland clip of the segment (line->x,y)-(x+w,y+h) to a rect.
bool Clip_Line(Rect* line, int x, int y, int w, int h) {
    int x0 = line->x, y0 = line->y;
    int x1 = line->x + line->w, y1 = line->y + line->h;
    int xmax = x + w - 1, ymax = y + h - 1;
    int c0 = OutCode(x0, y0, x, y, w, h);
    int c1 = OutCode(x1, y1, x, y, w, h);
    bool inside = (c0 == 0 && c1 == 0);
    while (!inside && (c0 & c1) == 0) {
        int c = c0;
        if (c0 == 0) {
            int tx = x0, ty = y0;
            x0 = x1; y0 = y1;
            x1 = tx; y1 = ty;
            c = c1;
            c1 = c0;
        }
        if (c & 1) {
            y0 = y0 + ((y1 - y0) * (x - x0)) / (x1 - x0);
            x0 = x;
        } else if (c & 2) {
            x0 = x0 + ((x1 - x0) * (y - y0)) / (y1 - y0);
            y0 = y;
        } else if (c & 4) {
            y0 = y0 + ((y1 - y0) * (xmax - x0)) / (x1 - x0);
            x0 = xmax;
        } else {
            x0 = x0 + ((x1 - x0) * (ymax - y0)) / (y1 - y0);
            y0 = ymax;
        }
        c0 = OutCode(x0, y0, x, y, w, h);
        inside = (c1 == 0 && c0 == 0);
    }
    Rect_SetLTRB(line, x0, y0, x1, y1);
    return inside;
}

// ---------------------------------------------------------------- rand

// Borland RTL rand() (0x42bf68): 64-bit LCG, multiplier 0x15A4_00004E35,
// returns the high dword masked to 31 bits. srand() sets lo=seed, hi=0.
static uint64_t g_randSeed = 1;
int Rand() {
    g_randSeed = g_randSeed * 0x15A400004E35ull + 1;
    return (int)((g_randSeed >> 32) & 0x7fffffff);
}
void SRand(unsigned seed) { g_randSeed = seed; }

// Swap item i with item i + rand()%(n-i), then report where the original
// first item ended up (0x427430).
unsigned Util_Shuffle(void* items, unsigned count, size_t size) {
    u8* base = (u8*)items;
    u8* tmp = (u8*)Mem_Alloc(size);
    u8* first = (u8*)Mem_Alloc(size);
    memcpy(first, base, size);
    for (unsigned i = 0; i < count; i++) {
        unsigned j = (count == i) ? 0 : (unsigned)Rand() % (count - i);
        if (j != 0) {
            memcpy(tmp, base + i * size, size);
            memcpy(base + i * size, base + (i + j) * size, size);
            memcpy(base + (i + j) * size, tmp, size);
        }
    }
    unsigned idx = 0;
    while (idx < count && memcmp(base + idx * size, first, size) != 0) idx++;
    Mem_Free(tmp);
    Mem_Free(first);
    return idx;
}
