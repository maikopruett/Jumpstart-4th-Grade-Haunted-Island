// Module 15: location info screen (0x41a6e8..0x41a8c8).
#include "game/common.h"

// 0x41a6e8: module 15 entry. Shows the info text for the current location and
// waits for event 0x20.
void FUN_0041a6e8() {
    void* pvVar1;
    char* text;
    int uVar2;

    pvVar1 = Res_Load(0x6c2);
    Img_Draw(D32(0x436ee4), D32(0x436ee8), pvVar1, 0);
    Res_Free(pvVar1);
    text = FUN_0041a7e4();
    Text_DrawFont(text, D32(0x436640), D32(0x436644), D32(0x436648), D32(0x43664c), P(0x4318b0),
                  0x24, 0);
    Res_Free(text);
    Gfx_SaveBg(g_viewRect.x, g_viewRect.y, g_viewRect.w, g_viewRect.h);
    FUN_004032e3();
    Screen_FlushAll();
    pvVar1 = Res_Load(0x6c3);
    Pal_FadeTo((const u8*)pvVar1, 1);
    Res_Free(pvVar1);
    Hotspot_Set(0x20, D32(0x43188d), D32(0x431891), D32(0x431895), D32(0x431899));
    Event_Flush();
    do {
        uVar2 = Event_Poll();
        FUN_0040394c((u16)uVar2);
    } while ((u16)uVar2 != 0x20);
    Hotspot_Remove(0xd); // sic: removes 0xd, not the 0x20 set above
}

// 0x41a7df: module 15 cleanup.
void FUN_0041a7df() {
}

// 0x41a7e4: loads the info text resource for location D32(0x432154).
char* FUN_0041a7e4() {
    int unaff_EBX = 0;
    int iVar1;

    switch (D32(0x432154)) {
    case 3:
    case 0x36:
        unaff_EBX = 0;
        break;
    case 0x12:
    case 0x3b:
        unaff_EBX = 1;
        break;
    case 5:
    case 0x3d:
        unaff_EBX = 2;
        break;
    case 0xc:
    case 0x43:
        unaff_EBX = 3;
        break;
    default:
        Fatal_Error(DSTR(0x436650)); // "No Info Place here!"
    }
    if ((s16)D16(0x43b510) + unaff_EBX < 0x33) {
        iVar1 = (s16)D16(0x43b510) + unaff_EBX;
    } else {
        iVar1 = Rand();
        iVar1 = iVar1 % 0x33;
    }
    return (char*)Res_Load(iVar1 * 2 + 0x61b);
}
