#include "engine/sprite.h"

#include "engine/gfx.h"
#include "engine/input.h"
#include "engine/sys.h"

List g_sprites;
static int g_nextSpriteId = 0; // 0x437970

// ---------------------------------------------------------------- sprites

void Spr_Init() { List_Init(&g_sprites, sizeof(Sprite)); }
void Spr_Shutdown() { List_Destroy(&g_sprites); }

Sprite* Spr_Find(int id) {
    for (List_First(&g_sprites); !List_AtEnd(&g_sprites); List_Next(&g_sprites)) {
        Sprite* s = (Sprite*)List_CurData(&g_sprites);
        if (s->id == id) return s;
    }
    return nullptr;
}

static int CmpZ(const void* node, const void* add) {
    return ((const Sprite*)node)->newZ - ((const Sprite*)add)->newZ;
}

int Spr_Create(const void* img, int x, int y, int z) {
    Sprite s;
    s.id = ++g_nextSpriteId;
    s.curImg = nullptr;
    s.newImg = img;
    Rect_Set(&s.cur, 0, 0, 0, 0);
    Rect_Set(&s.next, x, y, img ? Img_W(img) : 0, img ? Img_H(img) : 0);
    s.curZ = 0;
    s.newZ = z;
    s.flags = SPR_REDRAW | SPR_DIRTY;
    List_InsertSorted(&g_sprites, &s, CmpZ);
    return s.id;
}

bool Spr_Delete(int id, int deferred) {
    Sprite* s = Spr_Find(id);
    if (!s) return false;
    if (!deferred) {
        List_RemoveCur(&g_sprites);
    } else {
        s->newImg = nullptr;
        s->flags = SPR_ERASE | SPR_DELETE;
    }
    return true;
}

void Spr_DeleteAll(int deferred, int includeLocked) {
    List_First(&g_sprites);
    while (!List_AtEnd(&g_sprites)) {
        Sprite* s = (Sprite*)List_CurData(&g_sprites);
        if ((s->flags & SPR_LOCKED) == 0 || includeLocked) {
            if (!deferred) {
                List_RemoveCur(&g_sprites);
            } else {
                s->newImg = nullptr;
                s->flags = SPR_ERASE | SPR_DELETE;
                List_Next(&g_sprites);
            }
        } else {
            List_Next(&g_sprites);
        }
    }
}

bool Spr_SetImage(int id, const void* img) {
    Sprite* s = Spr_Find(id);
    if (!s) return false;
    if (img != s->curImg) {
        s->newImg = img;
        s->next.w = img ? Img_W(img) : 0;
        s->next.h = img ? Img_H(img) : 0;
        s->flags |= SPR_ERASE | SPR_REDRAW | SPR_DIRTY;
    }
    return true;
}

bool Spr_GetImage(int id, const void** out) {
    Sprite* s = Spr_Find(id);
    if (!s) return false;
    *out = s->curImg;
    return true;
}

bool Spr_SetPos(int id, int x, int y) {
    Sprite* s = Spr_Find(id);
    if (!s) return false;
    if (s->cur.x != x || s->cur.y != y) {
        s->next.x = x;
        s->next.y = y;
        s->flags |= SPR_ERASE | SPR_REDRAW | SPR_DIRTY;
    }
    return true;
}

bool Spr_GetPos(int id, Point* out) {
    Sprite* s = Spr_Find(id);
    if (s) {
        out->x = s->cur.x;
        out->y = s->cur.y;
    }
    return s != nullptr;
}

bool Spr_Move(int id, int dx, int dy) {
    Sprite* s = Spr_Find(id);
    if (!s) return false;
    if (dx != 0 || dy != 0) {
        s->next.x += dx;
        s->next.y += dy;
        s->flags |= SPR_ERASE | SPR_REDRAW | SPR_DIRTY;
    }
    return true;
}

bool Spr_SetZ(int id, int z) {
    Sprite* s = Spr_Find(id);
    if (!s) return false;
    if (s->curZ != z) {
        Sprite copy = *s;
        List_RemoveCur(&g_sprites);
        copy.newZ = z;
        copy.flags |= SPR_ERASE | SPR_REDRAW | SPR_DIRTY;
        List_InsertSorted(&g_sprites, &copy, CmpZ);
    }
    return true;
}

bool Spr_GetZ(int id, int* out) {
    Sprite* s = Spr_Find(id);
    if (!s) return false;
    *out = s->curZ;
    return true;
}

bool Spr_SetLocked(int id, int locked) {
    Sprite* s = Spr_Find(id);
    if (!s) return false;
    if (locked) s->flags |= SPR_LOCKED;
    else s->flags &= ~SPR_LOCKED;
    return true;
}

bool Spr_IsLocked(int id, int* out) {
    Sprite* s = Spr_Find(id);
    if (!s) return false;
    *out = (s->flags & SPR_LOCKED) != 0;
    return true;
}

bool Spr_Invalidate(int id) {
    Sprite* s = Spr_Find(id);
    if (!s) return false;
    s->flags |= SPR_ERASE | SPR_REDRAW | SPR_DIRTY;
    return true;
}

void Spr_InvalidateAll() {
    for (List_First(&g_sprites); !List_AtEnd(&g_sprites); List_Next(&g_sprites))
        ((Sprite*)List_CurData(&g_sprites))->flags |= SPR_REDRAW | SPR_DIRTY;
}

bool Spr_Overlap(int id1, int id2) {
    Sprite* a = Spr_Find(id1);
    Sprite* b = Spr_Find(id2);
    if (!a || !b) return false;
    return Rect_Intersect(a->cur.x, a->cur.y, a->cur.w, a->cur.h, b->cur.x, b->cur.y, b->cur.w, b->cur.h,
                          nullptr);
}

static bool Overlaps(const Rect& a, const Rect& b) {
    return Rect_Intersect(a.x, a.y, a.w, a.h, b.x, b.y, b.w, b.h, nullptr);
}

void Spr_Update(int flush) {
    for (;;) {
        // Propagate redraws to sprites overlapping anything that changes.
        bool changed = false;
        for (List_First(&g_sprites); !List_AtEnd(&g_sprites); List_Next(&g_sprites)) {
            Sprite* a = (Sprite*)List_CurData(&g_sprites);
            if ((a->flags & (SPR_ERASE | SPR_REDRAW)) == 0) continue;
            ListNode* self = g_sprites.cur;
            bool after = false;
            for (ListNode* n = g_sprites.head->next; n != g_sprites.head; n = n->next) {
                if (n == self) {
                    after = true;
                    continue;
                }
                Sprite* b = (Sprite*)n->data;
                if (b->flags & SPR_REDRAW) continue;
                bool hit = false;
                if ((a->flags & SPR_ERASE) && Overlaps(b->cur, a->cur)) hit = true;
                else if ((a->flags & SPR_REDRAW) && after && Overlaps(b->cur, a->next)) hit = true;
                if (hit) {
                    b->flags |= SPR_REDRAW;
                    changed = true;
                }
            }
        }
        if (changed) continue;

        for (List_First(&g_sprites); !List_AtEnd(&g_sprites); List_Next(&g_sprites)) {
            Sprite* a = (Sprite*)List_CurData(&g_sprites);
            if (a->flags & SPR_ERASE) Gfx_RestoreBgDirty(a->cur.x, a->cur.y, a->cur.w, a->cur.h);
        }
        List_First(&g_sprites);
        while (!List_AtEnd(&g_sprites)) {
            Sprite* a = (Sprite*)List_CurData(&g_sprites);
            if (a->flags & SPR_DELETE) {
                List_RemoveCur(&g_sprites);
                continue;
            }
            if ((a->flags & SPR_REDRAW) && a->newImg) {
                if (a->flags & SPR_DIRTY) Img_DrawDirty(a->next.x, a->next.y, a->newImg, 1);
                else Img_Draw(a->next.x, a->next.y, a->newImg, 1);
            }
            a->curImg = a->newImg;
            a->cur = a->next;
            a->curZ = a->newZ;
            a->flags &= SPR_LOCKED;
            List_Next(&g_sprites);
        }
        if (flush) Dirty_Flush();
        return;
    }
}

// ---------------------------------------------------------------- animations

namespace {

const u32 kAngMagic = 0x474e41;  // "ANG\0"
const u32 kMtncMagic = 0x434e544d; // "MTNC" (composite; none ship with the game)

struct Anim {
    int id;
    int type; // 0 composite, 1 ANG
    u8* res;
    int z;
    int state; // 1 = playing
    int x, y;
    int counters[4];
    int count1, count2, numSprites;
    int reuseFirst;
    int sprites[32];
    const void** frames;
    const u8* scriptStart;
    const u8* script;
};

List g_anims; // 0x43b7b8
ResLoadFn g_load;
ResFreeFn g_free;

Anim* Find(int id) {
    for (List_First(&g_anims); !List_AtEnd(&g_anims); List_Next(&g_anims)) {
        Anim* a = (Anim*)List_CurData(&g_anims);
        if (a->id == id) return a;
    }
    return nullptr;
}

void StopSprites(Anim* a) {
    for (int i = a->reuseFirst ? 1 : 0; i < a->numSprites; i++) Spr_Delete(a->sprites[i], 1);
    a->state = 0;
}

} // namespace

void Anim_Init(ResLoadFn load, ResFreeFn free) {
    List_Init(&g_anims, sizeof(Anim));
    g_load = load;
    g_free = free;
}

void Anim_Shutdown() { List_Destroy(&g_anims); }

void* Anim_GetRes(int id) {
    Anim* a = Find(id);
    return a ? a->res : nullptr;
}

void Anim_Load(int resId) {
    u8* res = (u8*)g_load(resId);
    if (!res) return;
    Anim a;
    memset(&a, 0, sizeof a);
    for (int& c : a.counters) c = -1;
    a.res = res;
    if (RdU32(res) == kAngMagic) {
        a.id = resId;
        a.type = 1;
        a.count1 = RdS32(res + 0xc);
        a.count2 = RdS32(res + 0x10);
        a.numSprites = RdS32(res + 0x14);
        a.scriptStart = res + RdU32(res + 0x18);
        int n = a.count1 + a.count2;
        a.frames = (const void**)Mem_Alloc(sizeof(void*) * (n > 0 ? n : 1));
        for (int i = 0; i < n; i++) a.frames[i] = Ang_FrameImage(res, i);
        a.script = a.scriptStart;
        List_PushBack(&g_anims, &a);
    } else if (RdU32(res) == kMtncMagic) {
        // Composite: frames are separate resources listed at +0xc.
        a.id = resId;
        a.type = 0;
        const u8* ids = res + 0xc;
        const u8* hdr = ids + RdU32(res + 8) * 4;
        a.count1 = RdS32(hdr + 4);
        a.count2 = RdS32(hdr + 8);
        a.numSprites = RdS32(hdr + 0xc);
        int n = a.count1 + a.count2;
        a.frames = (const void**)Mem_Alloc(sizeof(void*) * (n > 0 ? n : 1));
        for (int i = 0; i < n; i++) a.frames[i] = g_load(RdU32(ids + i * 4));
        a.scriptStart = hdr + 0x20;
        a.script = a.scriptStart;
        List_PushBack(&g_anims, &a);
    } else {
        g_free(res);
    }
}

void Anim_Free(int resId) {
    Anim* a = Find(resId);
    if (!a) return;
    if (a->type == 0) {
        for (int i = 0; i < a->count1 + a->count2; i++) g_free(a->frames[i]);
    }
    g_free(a->res);
    Mem_Free(a->frames);
    List_RemoveCur(&g_anims);
}

bool Anim_IsPlaying(int resId) {
    Anim* a = Find(resId);
    return a && a->state == 1;
}

void Anim_StartWithSprite(int resId, int x, int y, int z, int spriteId) {
    Anim* a = Find(resId);
    if (!a) return;
    a->x = x;
    a->y = y;
    for (int& c : a->counters) c = -1;
    a->z = z;
    a->script = a->scriptStart;
    a->reuseFirst = 1;
    a->sprites[0] = spriteId;
    for (int i = 1; i < a->numSprites; i++) a->sprites[i] = Spr_Create(nullptr, x, y, z);
    a->state = 1;
}

void Anim_Start(int resId, int x, int y, int z) {
    Anim* a = Find(resId);
    if (!a) return;
    a->x = x;
    a->y = y;
    for (int& c : a->counters) c = -1;
    a->z = z;
    a->script = a->scriptStart;
    a->reuseFirst = 0;
    for (int i = 0; i < a->numSprites; i++) a->sprites[i] = Spr_Create(nullptr, x, y, z);
    a->state = 1;
}

void Anim_Stop(int resId) {
    Anim* a = Find(resId);
    if (a) StopSprites(a);
}

void Anim_SetZ(int resId, int z) {
    Anim* a = Find(resId);
    if (!a) return;
    for (int i = 0; i < a->numSprites; i++) Spr_SetZ(a->sprites[i], z);
}

void Anim_UpdateAll() {
    for (List_First(&g_anims); !List_AtEnd(&g_anims); List_Next(&g_anims)) {
        Anim* a = (Anim*)List_CurData(&g_anims);
        if (a->state != 1) continue;
        int n = 0;
        bool done = false;
        while (!done) {
            u16 cmd = RdU16(a->script);
            a->script += 2;
            switch (cmd) {
                case 0x7f00:
                    StopSprites(a);
                    done = true;
                    break;
                case 0x7f01:
                    done = true;
                    break;
                case 0x7f02: {
                    int dx = RdS16(a->script);
                    int dy = RdS16(a->script + 2);
                    int frame = RdS16(a->script + 4);
                    a->script += 8;
                    Spr_SetImage(a->sprites[n], a->frames[frame]);
                    Spr_SetPos(a->sprites[n], dx + a->x, dy + a->y);
                    n++;
                    break;
                }
                case 0x7f03:
                    a->script += 2;
                    break;
                case 0x7f04: {
                    int target = RdS16(a->script);
                    int count = RdS16(a->script + 2);
                    int idx = RdS16(a->script + 4);
                    a->script += 6;
                    if (a->counters[idx] == -1) a->counters[idx] = count;
                    a->counters[idx]--;
                    if (a->counters[idx] == 0) a->counters[idx] = -1;
                    else a->script = a->scriptStart + target * 2;
                    break;
                }
            }
        }
        if (a->state == 1) {
            for (; n < a->numSprites; n++) Spr_SetImage(a->sprites[n], nullptr);
        }
    }
}

int Anim_PlayBlocking(int resId, AbortCallback cb) {
    int aborted = 0;
    Anim_Load(resId);
    Anim_Start(resId, 0, 0, 0);
    while (Anim_IsPlaying(resId)) {
        Timer_Set(1, 100);
        Anim_UpdateAll();
        Spr_Update(1);
        do {
            int ev = Event_Poll();
            if (g_fastQuit) {
                Anim_Stop(resId);
                aborted = 1;
                break;
            }
            if (cb) {
                aborted = cb((u16)ev);
                if (aborted) {
                    Anim_Stop(resId);
                    break;
                }
            }
        } while (!Timer_Expired(1));
    }
    Anim_Free(resId);
    return aborted != 0;
}

int Anim_PlayFrames(u32 resId, AbortCallback cb, int msPerFrame) {
    int aborted = 0;
    void* ang = g_load(resId);
    int n = Ang_CountFrames(ang);
    Timer_Set(9, msPerFrame);
    for (int i = 0; i < n && !aborted; i++) {
        Point pos;
        const void* img = Ang_FrameImage(ang, Ang_FrameInfo(ang, i, &pos));
        if (img) {
            Rect b;
            Img_Bounds(img, &b);
            b.x += pos.x;
            b.y += pos.y;
            Img_Draw(pos.x, pos.y, img, 1);
            Screen_Flush(b);
        }
        do {
            int ev = Event_Poll();
            if (cb) aborted = cb((u16)ev);
            if (g_fastQuit) aborted = 1;
        } while (!Timer_Expired(9) && !aborted);
        Timer_Set(9, msPerFrame);
    }
    g_free(ang);
    return aborted;
}

const void* Ang_FrameImage(const void* ang, int index) {
    const u8* p = (const u8*)ang;
    if (index < 0 || RdS32(p + 0x10) + RdS32(p + 0xc) <= index) return nullptr;
    return p + RdU32(p + 0x1c + index * 4);
}

int Ang_FrameInfo(const void* ang, int frame, Point* pos) {
    const u8* p = (const u8*)ang;
    pos->x = pos->y = 0;
    if (RdU32(p) != kAngMagic) return -1;
    const u8* s = p + RdU32(p + 0x18);
    int cur = 0;
    for (;;) {
        u16 cmd = RdU16(s);
        switch (cmd) {
            case 0x7f00: return -1;
            case 0x7f01: cur++; s += 2; break;
            case 0x7f02:
                if (frame == cur) {
                    pos->x = RdS16(s + 2);
                    pos->y = RdS16(s + 4);
                    return RdS16(s + 6);
                }
                s += 10;
                break;
            case 0x7f03: s += 4; break;
            case 0x7f04: s += 8; break;
            default: s += 2; break;
        }
    }
}

int Ang_CountFrames(const void* ang) {
    const u8* p = (const u8*)ang;
    if (RdU32(p) != kAngMagic) return 0;
    const u8* s = p + RdU32(p + 0x18);
    int n = 0;
    for (;;) {
        u16 cmd = RdU16(s);
        switch (cmd) {
            case 0x7f00: return n;
            case 0x7f01: n++; s += 2; break;
            case 0x7f02: s += 10; break;
            case 0x7f03: s += 4; break;
            case 0x7f04: s += 8; break;
            default: s += 2; break;
        }
    }
}

const void* Ang_FrameImageAt(const void* ang, int frame, Point* pos) {
    return Ang_FrameImage(ang, Ang_FrameInfo(ang, frame, pos));
}

const void* Ang_FirstImage(const void* ang, Point* pos) { return Ang_FrameImageAt(ang, 0, pos); }

// ---------------------------------------------------------------- lip sync

namespace {
struct LipSync {
    int id;
    const void* ang;
    const u8* frames;  // frame indices start at +4, terminated by 0xff
    int spriteId;
    int idx;
    u32 start;
};
List g_lipSync; // 0x43bed8
int g_nextLipSync = 1; // 0x437b24
} // namespace

void LipSync_Init() { List_Init(&g_lipSync, sizeof(LipSync)); }
void LipSync_Shutdown() { List_Destroy(&g_lipSync); }

static LipSync* LipFind(int id) {
    for (List_First(&g_lipSync); !List_AtEnd(&g_lipSync); List_Next(&g_lipSync)) {
        LipSync* l = (LipSync*)List_CurData(&g_lipSync);
        if (l->id == id) return l;
    }
    return nullptr;
}

int LipSync_Add(const void* ang, const void* frames, int spriteId) {
    LipSync l;
    l.id = g_nextLipSync++;
    l.ang = ang;
    l.frames = (const u8*)frames;
    l.spriteId = spriteId;
    l.idx = 0;
    l.start = Time_Get() + 250; // sounds start 0x1588 samples after the request
    List_PushBack(&g_lipSync, &l);
    return l.id;
}

void LipSync_Remove(int id) {
    if (LipFind(id)) List_RemoveCur(&g_lipSync);
}

void LipSync_Update() {
    for (List_First(&g_lipSync); !List_AtEnd(&g_lipSync); List_Next(&g_lipSync)) {
        LipSync* l = (LipSync*)List_CurData(&g_lipSync);
        u32 now = Time_Get();
        if ((u32)(l->idx * 100) + l->start <= now && l->frames[4 + l->idx] != 0xff) {
            Spr_SetImage(l->spriteId, Ang_FrameImage(l->ang, l->frames[4 + l->idx]));
            Spr_Invalidate(l->spriteId);
            l->idx++;
        }
    }
    List_First(&g_lipSync);
    while (!List_AtEnd(&g_lipSync)) {
        LipSync* l = (LipSync*)List_CurData(&g_lipSync);
        if (l->frames[4 + l->idx] == 0xff) List_RemoveCur(&g_lipSync);
        else List_Next(&g_lipSync);
    }
}

bool LipSync_Active(int id) { return LipFind(id) != nullptr; }
