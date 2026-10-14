// Unit "print" (0x4015d0..0x40189b): printing a progress report. The original
// opened the default Windows printer, started a document and drew a DIB
// scaled to fit the page (0x4017cb). The port hands the DIB to the macOS
// print panel, which does the same fitting.
#include "game/common.h"
#include "platform/print.h"

// 0x4015d0: run a print job; page content came from a callback.
int FUN_004015d0(void* param_1) {
    (void)param_1;
    return 0;
}

int FUN_004016e5() { return 0; } // 0x4016e5: default printer DC (unused)

// 0x4017b3: print a DIB. Returns nonzero on success like 0x4015d0.
int FUN_004017b3(void* dib) {
    Event_Flush();
    Sound_Pause();
    bool ok = Print_DIB(dib);
    Sound_Resume();
    Event_Flush();
    return ok ? 1 : 0;
}

// 0x4017cb: page drawing callback; folded into Print_DIB.
void FUN_004017cb(int, int, int) {}
