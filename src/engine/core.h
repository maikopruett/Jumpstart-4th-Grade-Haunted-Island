// Core types and helpers shared by the engine and the ported game code.
// Function comments give the address of the original routine in 4G.EXE.
#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;

struct Rect {
    int x, y, w, h;
};
struct Point {
    int x, y;
};

// ---- memory (0x4253b8 / 0x42540b) ----
void* Mem_Alloc(size_t n);
void Mem_Free(const void* p);

// ---- unaligned little-endian access into resource blobs ----
static inline u16 RdU16(const void* p) { u16 v; memcpy(&v, p, 2); return v; }
static inline s16 RdS16(const void* p) { s16 v; memcpy(&v, p, 2); return v; }
static inline u32 RdU32(const void* p) { u32 v; memcpy(&v, p, 4); return v; }
static inline s32 RdS32(const void* p) { s32 v; memcpy(&v, p, 4); return v; }
static inline void WrU16(void* p, u16 v) { memcpy(p, &v, 2); }
static inline void WrU32(void* p, u32 v) { memcpy(p, &v, 4); }

// ---- generic doubly linked list (0x41d254..0x41d666) ----
// Each node owns a copy of elemSize bytes, or stores the pointer itself when
// elemSize is 0. "cur" is the iteration cursor; head is a sentinel.
struct ListNode {
    void* data;
    ListNode* prev;
    ListNode* next;
};
struct List {
    ListNode* head;
    ListNode* cur;
    int elemSize;
    int count;
};

void List_Init(List* l, int elemSize);                 // 0x41d254
void List_Destroy(List* l);                            // 0x41d28c
bool List_IsEmpty(const List* l);                      // 0x41d2a9
int List_Count(const List* l);                         // 0x41d2bb
bool List_CurIsFirst(const List* l);                   // 0x41d2c6
bool List_CurIsLast(const List* l);                    // 0x41d2dc
bool List_AtEnd(const List* l);                        // 0x41d2f2
void List_First(List* l);                              // 0x41d305
void List_Last(List* l);                               // 0x41d315
void List_Next(List* l);                               // 0x41d325
void List_Prev(List* l);                               // 0x41d336
bool List_GetCur(List* l, void* out);                  // 0x41d347
void* List_CurData(List* l);                           // 0x41d37c
void List_InsertAfter(List* l, const void* data);      // 0x41d389
void List_InsertBefore(List* l, const void* data);     // 0x41d3eb
void List_PushFront(List* l, const void* data);        // 0x41d44d
void List_PushBack(List* l, const void* data);         // 0x41d4a8
typedef int (*ListCmp)(const void* nodeData, const void* newData);
void List_InsertSorted(List* l, const void* data, ListCmp cmp); // 0x41d503
bool List_RemoveCur(List* l);                          // 0x41d578
void List_Clear(List* l);                              // 0x41d5ca
bool List_PopFront(List* l, void* out);                // 0x41d62d

// ---- rectangles (0x41d6a0..0x41da0f) ----
static inline void Pt_Set(Point* p, int x, int y) { p->x = x; p->y = y; }
static inline void Rect_Set(Rect* r, int x, int y, int w, int h) { r->x = x; r->y = y; r->w = w; r->h = h; }
void Rect_FromPoints(Rect* r, int x1, int y1, int x2, int y2);   // 0x41d6f8
void Rect_FromLTRB(int l, int t, int r, int b, Rect* out);       // 0x41d756
void Rect_ToLTRB(int x, int y, int w, int h, int* out4);         // 0x41d77b
void Rect_SetLTRB(Rect* r, int l, int t, int rr, int b);         // 0x41d7bf
bool Pt_InRect(int px, int py, int x, int y, int w, int h);      // 0x41d7e4
bool Rect_Inside(int x, int y, int w, int h, int cx, int cy, int cw, int ch);       // 0x41d818
bool Rect_InsideStrict(int x, int y, int w, int h, int cx, int cy, int cw, int ch); // 0x41d856
bool Rect_Intersect(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2, Rect* out); // 0x41d8c0
bool Rect_Clip(Rect* r, int x, int y, int w, int h);             // 0x41d94e
void Rect_Union(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2, Rect* out);     // 0x41d976
bool Clip_Line(Rect* line, int x, int y, int w, int h);          // 0x41da0f
static inline bool Rect_Clip(Rect* r, const Rect& c) { return Rect_Clip(r, c.x, c.y, c.w, c.h); }

// ---- C runtime replacements used by the game ----
int Rand();                    // Borland rand(), 0x42bf68
void SRand(unsigned seed);     // 0x42bf34
// Shuffles in place; returns the new index of the element that was first.
unsigned Util_Shuffle(void* items, unsigned count, size_t size); // 0x427430
