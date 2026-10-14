# Task: port one or more game units of 4G.EXE to C++

You are porting part of the game logic of *JumpStart 4th Grade: Haunted Island*
(Knowledge Adventure, 1997) from its Ghidra decompilation to C++, so it runs
natively on Apple Silicon. The engine is already done; you port game logic.

1. **Read `docs/PORTING.md` completely before writing code.** It defines all
   conventions (data segment access with `D32`/`P`/`DSTR`, engine API mapping,
   callbacks, module jumps). Follow it exactly.
2. Dump your unit's code: `python3 -I re/tools/unit.py <unit> --dump > /tmp/<unit>.c`
   (`re/tools/unit.py <unit>` lists functions; skip the ones marked dead).
   Port **every live function** of the unit into `src/game/<unit>.cpp`.
3. Your functions are declared in `src/game/decl/<unit>.h` (generated, types
   guessed mechanically). You may correct signatures of *your own* functions in
   that file only (e.g. a parameter that is really a pointer, a callback that
   must be `int fn(int ev)`). Never edit any other file except your `.cpp` and
   your `decl/<unit>.h`.
4. Calls to functions outside your unit are declared in `game/funcs.h` (via
   `game/common.h`). Use `re/tools/fn.py NAME` to read their decompiled code when
   you need to understand what they do (and therefore how to call them), but
   don't port them. Main-unit helpers you will see often:
   - `FUN_00402ec2(resId, toolbar, abortable)` – play a narration with Ms.
     Grunkle lip sync; returns 1 if aborted.
   - `FUN_00402e72(resId, ang, sprite, toolbar, abortable)` – talking sound with
     a given animation/sprite; `FUN_00402cd2`/`FUN_00402dcf` variants take a
     loaded resource.
   - `FUN_00402c30/2c54(idx, prio)` – play UI sound idx (once/looped);
     `FUN_00402bad/2bd1(idx, prio)` – module sound bank; `FUN_00402b7e(resId)`
     loads that bank.
   - `FUN_0040394c(ev)` – toolbar hover/click handling.
   - `FUN_004032e3()` – build the toolbar for the current module.
5. Shared pointer globals (always via `P()`, never `D32()`):
   - `0x4318a4` (font bank resource) and `0x4318a8`..`0x431900` step 4 (fonts /
     sub-resources computed from it, e.g. `P(0x4318b0)` is a font);
   - `0x4342f4` and `0x4342fc` (animation resources used for Ms. Grunkle's
     talking/blinking); `0x4342f0` and `0x4342f8` are ints (sprite ids);
   - `0x4341d8` (heap array of 0x329-byte kid records).
   Ints shared with other units include `0x432154` (current location/state),
   `0x436ee4`/`0x436ee8` (screen origin, always 0).
6. Check your work compiles: `./tools/check.sh src/game/<unit>.cpp` (syntax and
   types against all headers). Do **not** run cmake or build the app – other
   agents are editing other units at the same time.
7. Fidelity over prettiness. Port literally; keep the original control flow,
   arithmetic, timings and resource IDs. Use the disassembly
   (`re/out/4G.asm`) when the decompilation is unclear.
8. Finish with a short report: number of functions ported, every signature
   you changed in `decl/<unit>.h`, globals you decided are pointers (`P`), and
   anything uncertain (with addresses) or any engine API you needed but was
   missing.
