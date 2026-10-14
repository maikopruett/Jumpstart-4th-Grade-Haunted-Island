# Porting guide: 4G.EXE game logic → C++

The game code of `4G.EXE` (addresses `0x401000..0x41c62c`) is ported function by
function from Ghidra's decompilation. The engine it calls (graphics, sprites,
sound, video, input...) was reimplemented separately in `src/engine/` and
keeps the original function semantics under readable names.

## Sources

| What | Where |
| --- | --- |
| Decompiled C (names applied) | `re/out/4G.EXE.c` |
| Print functions | `python3 -I re/tools/fn.py FUN_00402ec2 [...]` (`-r N` adds callees, `-x NAME` lists callers) |
| Disassembly | `re/out/4G.asm` (objdump, Intel syntax) |
| Function list / sizes | `re/out/4G.EXE.functions.tsv` |
| Module partition | `re/out/partition.json` |
| Code pointers stored in data | `re/out/coderefs.json` |
| Engine headers | `src/engine/*.h` |
| Shared game functions | `src/game/common.h` |
| Data segment access | `src/game/seg.h` |

Regenerate the decompilation with `re/decompile.sh` (after editing `re/names.tsv`).

## Ground rules

1. **Behaviour must match the original exactly.** Port control flow literally,
   including odd branches, quirks and dead stores that affect state. Do not
   "simplify" logic, change timings, or fix bugs unless they would crash.
2. Keep every function's original name (`FUN_00404f80`) and put its address in
   a comment. Functions are plain C++ (no classes); `extern` linkage.
3. When Ghidra's output is ambiguous (lost arguments, `in_stack_`, `unaff_`,
   `extraout_`, `CONCAT`, calls through pointers with no arguments, struct
   copies), check the disassembly. Callbacks passed to the engine are called
   with the 16-bit event code: `int cb(int ev)`.
4. Integer semantics: mirror the casts Ghidra shows (`(short)`, `(ushort)`,
   unsigned comparisons, `/` vs `>>` rounding). Use `s16/u16/s32/u32`.
   Event codes are compared as `(u16)ev` / `(s16)ev` exactly as decompiled.
5. `ftol()` is truncation: `(int)value`.

## Globals: the emulated data segment

All game globals live at fixed addresses in `0x42f000..0x443000`. Access them
through `seg.h` instead of declaring C++ variables:

```cpp
D32(0x432154)           // DAT_00432154 as int (s32&, assignable)
DU32 / D16 / DU16 / D8 / DS8  // other widths
DSTR(0x4319b4)          // s_Movies_M__s_smk_004319b4 (char*, read-only use)
DPTR(0x439620)          // u8* to a global array/struct in the segment
(&DAT_004395f8)[i]      // -> D32(0x4395f8 + i * 4)
*(int *)(&DAT_00439620 + a * 8 + b * 0x30)  // -> D32(0x439620 + a * 8 + b * 0x30)
```

**Pointers.** A global (or a field of a global table) that holds a *data
pointer* — a loaded resource, an allocated buffer, a pointer to a string —
must use `P(va)` (a 64-bit shadow slot), never `D32`:

```cpp
DAT_00431940 = Res_Load(id);         ->  P(0x431940) = Res_Load(id);
Res_Free(DAT_00431940);              ->  Res_Free(P(0x431940));
*(int *)(DAT_00431940 + i * 4) + DAT_00431940
                                     ->  (u8*)P(0x431940) + RdS32((u8*)P(0x431940) + i * 4)
PTR_s_ka_logo_00431988[i]            ->  (char*)P(0x431988 + i * 4)
```
Slots that are pointers in the EXE's initialized data (string tables etc.) are
pre-seeded, so `(char*)P(va)` works for both static and runtime pointers.
Decide pointer-ness from how a value is used, not from Ghidra's type. If a slot
is used as both an int and a pointer, use `P()` for pointer uses and tell me.

**Code pointers** in data (callbacks in tables, e.g. `(**(code **)(x + 0x20))()`)
keep their original x86 address: call them with `CODE(D32(addr))()` (cast to
the right function type if it takes arguments). Any of your functions whose
address appears in `re/out/coderefs.json` (or that the code stores into a
global) must be registered once in your file:
`REGISTER_CODE(0x402624, FUN_00402624);`

**Heap structures.** If game code allocates memory and stores pointers inside
it, define a proper C++ struct for it (document the original layout).

**Stack locals** copied from globals (`puVar3 = &DAT_00431968; for (...) *puVar5 = *puVar3`)
become `memcpy(local, DPTR(0x431968), n)` or explicit loads.

## Engine mapping

Most engine calls keep their decompiled names. Differences:

| Decompiled | C++ |
| --- | --- |
| `Rect_Set(&local_20, x, y, w, h)` with `int local_20, local_1c...` | `Rect r; Rect_Set(&r, x, y, w, h);` |
| `Rect_Set2` | `Rect_Set` |
| `Pt_Set`, `Pt_Set2`, `Pt_Set3`, `Pt_Set4` (`&local_10, x, y`) | `Point p; Pt_Set(&p, x, y);` |
| `Rect_Clip(&r, x, y, w, h)` | `Rect_Clip(&r, x, y, w, h)` (Rect*) |
| `Spr_GetPos(id, &local_10)` / `Mouse_GetPos(&pt)` | `Point*` |
| `Ang_FrameImageAt(ang, frame, &local_10)` / `Ang_FirstImage` / `Ang_FrameInfo` | `Point*` |
| `Img_Bounds(img, &local_20)` / `Img_FullRect` | `Rect*` |
| `Img_DrawOpaque(x, y, img)` | same (exists) |
| `Pal_FadeTo2(rgb, 1)` | `Pal_FadeTo((const u8*)rgb, 1)` |
| `Pal_Set(rgb, 1)` | `Pal_Set((const u8*)rgb, 1)` |
| `Cursor_Load(ids)` | `Cursor_Load((const u16*)ids)` |
| `Video_Play(name, x, y, FUN_xxx)` | same; callback `int FUN_xxx(int ev)` |
| `Anim_PlayFrames(id, FUN_xxx, ms)` | same |
| `longjmp(&g_jmpbuf, n)` | `Module_Jump(n)` |
| `App_Quit()` | same |
| `toupper(c)` | `Util_ToUpper(c)` (ASCII only, like Borland's) |
| `stricmp` | `strcasecmp` |
| `memset/memmove/strcpy/strcat/strlen/strchr/strstr/strtok/strncpy/strcmp/sprintf` | the C library ones |
| `time(0)` | `(int)time(nullptr)` |
| `ftol()` | `(int)` truncation of the floating value (check asm for the operand) |
| `Rand()` | `Rand()` (exact Borland LCG) |

Engine globals the game reads/writes (they are C++ variables now, NOT segment
memory):

| Decompiled | C++ |
| --- | --- |
| `g_viewX/Y/W/H` | `g_viewRect.x/.y/.w/.h` |
| `g_bufX/Y/W/H` | `g_screenBufRect.x/...` |
| `g_clipX/Y/W/H` | `g_clip.x/...` |
| `g_targetBits`, `g_targetX/Y/W/H`, `g_targetTopDown` | `g_target.bits`, `g_target.rect.x/...`, `g_target.topDown` |
| `g_screenBits` | `g_screenSurf.bits` |
| `g_screenW`, `g_screenH` | same |
| `g_palette` | `g_palette` (u8*, 768 RGB) |
| `g_mouseX`, `g_mouseY` | same |
| `g_userEvent0/1/2` | `g_userEvent[0..2]` |
| `g_curCursor`, `g_curFont`, `g_appInactive` | same |
| `g_exeDir`, `g_resDir` | same (char arrays) |
| `g_module`, `g_prevModule` | same |
| `g_soundEnabled` | `Sound_Enabled()` / `Sound_SetEnabled(on, save)` |
| `g_textOverflow`, `g_textFirstLine`, `g_textColumnGap` | same |
| `DAT_00436ee4`, `DAT_00436ee8`, `DAT_0043184c`, ... (other engine-range addresses) | segment memory: `D32(...)` |

Engine types: images and resources are `void*` / `const void*`; sprite and
sound handles are `int`; `AbortCallback` is `int (*)(int ev)`.

## Files

Each module goes in `src/game/<module>.cpp` with a header `src/game/<module>.h`
declaring the functions other files call (the module's entry/cleanup
functions at least). Include `"game/common.h"` (which includes the engine and
`seg.h`). Do not edit engine files or other modules; if something is missing
or wrong in a shared header, note it in your report.
