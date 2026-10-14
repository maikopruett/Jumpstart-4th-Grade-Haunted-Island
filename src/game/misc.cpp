// Unit "misc" (0x41862c..0x4186af): assignment bookkeeping helpers.
#include "game/common.h"

// 0x41862c: is the current location the one the assignment asks for?
bool FUN_0041862c() {
    if (D32(0x436554) == 0 && D32(0x43b414) != 0)
        return D32(0x436558 + D32(0x43b424 + D32(0x43b41c) * 8) * 4) == D32(0x432154);
    return false;
}

void FUN_00418669() { D32(0x436554) = 1; } // 0x418669

void FUN_00418678() { // 0x418678
    FUN_00402c30(9, 10);
    FUN_00403e2c(0x32);
    Spr_Update(1);
    D32(0x436554) = 1;
}

void FUN_004186a3() { D32(0x436554) = 0; } // 0x4186a3
