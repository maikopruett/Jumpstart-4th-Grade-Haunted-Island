# Task: review ported game units against the original binary

The game logic of *JumpStart 4th Grade: Haunted Island* (4G.EXE, 1997) was
ported to C++ from Ghidra's decompilation (see `docs/PORTING.md` for the
conventions: `D32`/`P`/`DSTR` data-segment access, engine API mapping, etc.).
The port compiles and every module runs. Your job is to find and fix places
where the port does **not** behave exactly like the original.

Ghidra's decompilation is known to be wrong in places that look plausible:
it can show the wrong variable as a call argument (e.g. an image pointer
where the real argument is a computed color), drop arguments of variadic or
forwarding calls, mis-order or merge locals, hide struct copies, and garble
jump tables and loops. The disassembly (`re/out/4G.asm`, objdump Intel syntax)
is the ground truth.

For every function of your unit(s):

1. Compare the C++ in `src/game/<unit>.cpp` with the decompilation
   (`python3 -I re/tools/unit.py <unit> --dump`, or `re/tools/fn.py NAME`) **and**
   with the disassembly of that function. Find its lines with
   `grep -n "^  ADDR:" re/out/4G.asm` (ADDR without `0x`, e.g. `40394c`).
2. Check especially:
   - every call's arguments against the `push` sequence before the `call`
     (cdecl: last push = first argument), including constants, which global
     is passed, and computed values;
   - branch conditions and their signedness (`jl/jg` signed vs `jb/ja`
     unsigned), loop bounds, off-by-one;
   - the width of each global access (byte/word/dword) and whether a slot is
     really a pointer (`P`) or an int (`D32`);
   - return values that callers use;
   - code Ghidra dropped as "unreachable" or merged.
3. Fix real discrepancies directly in your unit's `.cpp` (and its
   `src/game/decl/<unit>.h` if a signature is wrong). Keep the original's
   quirks and bugs unless they would crash or corrupt memory on arm64.
   Do not edit files of other units, the engine, or shared headers.
4. Run `./tools/check.sh src/game/<unit>.cpp` after editing. Do not run cmake
   (other reviewers are editing other files).

Finish with a concise report: the bugs you fixed (function, address, what was
wrong), anything suspicious you could not resolve, and anything that needs a
change outside your unit.
