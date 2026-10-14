// Loads every resource of every .DF archive and draws every 'P800' image
// (also partially off-screen) to exercise the decompressor and blitters.
#include <cstdio>

#include "engine/gfx.h"
#include "engine/resource.h"
#include "engine/vfs.h"

// Minimal stand-ins for the platform layer pieces gfx.cpp touches.
static u8 g_disp[640 * 480];
u8* Sys_Display() { return g_disp; }
void Sys_DisplayChanged() {}
void Sys_SetDisplayPalette(const u8*) {}
void Sound_Update() {}
u32 Time_Get() { return 0; }
void Timer_Set(int, int) {}
void Timer_Wait(int) {}
bool g_fastQuit = false;

int main(int argc, char** argv) {
    Vfs_Mount(argv[1]);
    for (const char* n : {"Text.DF", "Island.DF", "Map.DF", "Sound2.DF", "Sound.DF", "Bosco.DF"}) Res_OpenArchive(n);
    Gfx_Init();
    int loaded = 0, images = 0, rleImages = 0;
    for (u32 id = 0; id < 14000; id++) {
        void* r = Res_Load(id);
        if (!r) continue;
        loaded++;
        if (memcmp(r, "P800", 4) == 0) {
            images++;
            if (Img_Enc(r) == 2) rleImages++;
            const int pos[][2] = {{0, 0}, {-17, -9}, {600, 400}, {-300, 100}, {320, -200}};
            for (auto& p : pos) {
                Img_Draw(p[0], p[1], r, 0);
                Img_Draw(p[0], p[1], r, 1);
            }
            Rect b;
            Img_Bounds(r, &b);
            void* d = Img_Decompress(r);
            Mem_Free(d);
        }
        Res_Free(r);
    }
    printf("%d resources, %d images (%d RLE) drawn OK\n", loaded, images, rleImages);
    return 0;
}
