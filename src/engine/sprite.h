// Sprites (0x4214c8..0x42192f), 'ANG' animations (0x421b54..0x42255c) and
// lip-sync frame lists (0x424ec4..0x425049).
#pragma once
#include "engine/core.h"

enum {
    SPR_ERASE = 1,    // restore the background under the old rect
    SPR_REDRAW = 2,   // draw the new image
    SPR_DIRTY = 4,    // mark the dirty grid when drawing
    SPR_DELETE = 8,   // remove after the next update
    SPR_LOCKED = 0x10 // survives Spr_DeleteAll unless forced
};

struct Sprite {
    int id;
    const void* curImg;
    const void* newImg;
    Rect cur;
    Rect next;
    int curZ;
    int newZ;
    u32 flags;
};

extern List g_sprites;   // 0x43b7a8

void Spr_Init();                                     // 0x4214c8
void Spr_Shutdown();                                 // 0x4214dc
Sprite* Spr_Find(int id);                            // 0x4214ec
int Spr_Create(const void* img, int x, int y, int z); // 0x421536
bool Spr_Delete(int id, int deferred);               // 0x4215d9
void Spr_DeleteAll(int deferred, int includeLocked); // 0x421618
bool Spr_SetImage(int id, const void* img);          // 0x42167c
bool Spr_GetImage(int id, const void** out);         // 0x4216ce
bool Spr_SetPos(int id, int x, int y);               // 0x4216f6
bool Spr_GetPos(int id, Point* out);                 // 0x421736
bool Spr_Move(int id, int dx, int dy);               // 0x421765
bool Spr_SetZ(int id, int z);                        // 0x4217a1
bool Spr_GetZ(int id, int* out);                     // 0x421808
bool Spr_SetLocked(int id, int locked);              // 0x421830
bool Spr_IsLocked(int id, int* out);                 // 0x421860
bool Spr_Invalidate(int id);                         // 0x42188f
void Spr_InvalidateAll();                            // 0x4218b3
bool Spr_Overlap(int id1, int id2);                  // 0x4218e5
void Spr_Update(int flush);                          // 0x42192f

// Event-code callback used to abort blocking animation/video playback.
typedef int (*AbortCallback)(int eventCode);

typedef void* (*ResLoadFn)(u32 id);
typedef void (*ResFreeFn)(const void* p);

void Anim_Init(ResLoadFn load, ResFreeFn free);      // 0x421d8f
void Anim_Shutdown();                                // 0x421db7
void* Anim_GetRes(int id);                           // 0x421d76
void Anim_Load(int resId);                           // 0x421e00
void Anim_Free(int resId);                           // 0x421fc1
bool Anim_IsPlaying(int resId);                      // 0x422013
void Anim_StartWithSprite(int resId, int x, int y, int z, int spriteId); // 0x422035
void Anim_Start(int resId, int x, int y, int z);     // 0x4220ca
void Anim_Stop(int resId);                           // 0x422184
void Anim_SetZ(int resId, int z);                    // 0x42219d
void Anim_UpdateAll();                               // 0x4221d1
int Anim_PlayBlocking(int resId, AbortCallback cb);  // 0x4223b5
int Anim_PlayFrames(u32 resId, AbortCallback cb, int msPerFrame); // 0x42243e

// Helpers on raw 'ANG' resources.
const void* Ang_FrameImage(const void* ang, int index);     // 0x421bd4
int Ang_FrameInfo(const void* ang, int frame, Point* pos);  // 0x421c0f
int Ang_CountFrames(const void* ang);                       // 0x421ceb
const void* Ang_FrameImageAt(const void* ang, int frame, Point* pos); // 0x422524
const void* Ang_FirstImage(const void* ang, Point* pos);   // 0x422547

void LipSync_Init();                                 // 0x424ec4
void LipSync_Shutdown();                             // 0x424ed8
int LipSync_Add(const void* ang, const void* frames, int spriteId); // 0x424f21
void LipSync_Remove(int id);                         // 0x424f73
void LipSync_Update();                               // 0x424f90
bool LipSync_Active(int id);                         // 0x425049
