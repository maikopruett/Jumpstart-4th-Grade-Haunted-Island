// Extracts artwork from the user's game data for packaging (app icon, disk
// image background). Nothing extracted is stored in the repository.
//
//   packtool <data> icon out.png                 32x32 icon from 4G.EXE
//   packtool <data> image <img> <pal> out.png    P800 image (key colour -> transparent)
//   packtool <data> survey outdir                every large image, with a guessed palette
#include <SDL3/SDL.h>

#include <cstdio>
#include <cstdlib>
#include <string>

#include "engine/exeres.h"
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

namespace {

const char* const kArchives[] = {"Text.DF", "Island.DF", "Map.DF", "Sound2.DF", "Sound.DF", "Bosco.DF"};

bool SavePNG(const u32* argb, int w, int h, const char* path) {
    SDL_Surface* s = SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_ARGB8888, (void*)argb, w * 4);
    if (!s) return false;
    bool ok = SDL_SavePNG(s, path);
    SDL_DestroySurface(s);
    if (!ok) fprintf(stderr, "packtool: can't write %s: %s\n", path, SDL_GetError());
    return ok;
}

bool SaveImage(u32 imgId, u32 palId, const char* path) {
    void* img = Res_Load(imgId);
    void* pal = Res_Load(palId);
    if (!img || memcmp(img, "P800", 4) != 0 || !pal) {
        fprintf(stderr, "packtool: no image %u / palette %u\n", imgId, palId);
        return false;
    }
    int w = Img_W(img), h = Img_H(img);
    u8 key = Img_Key(img);
    void* raw = Img_Enc(img) ? Img_Decompress(img) : img;
    const u8* px = (const u8*)raw + 12;
    const u8* rgb = (const u8*)pal;
    std::vector<u32> argb(w * h);
    for (int i = 0; i < w * h; i++) {
        u8 c = px[i];
        argb[i] = c == key ? 0 : 0xff000000u | (rgb[c * 3] << 16) | (rgb[c * 3 + 1] << 8) | rgb[c * 3 + 2];
    }
    if (raw != img) Mem_Free(raw);
    Res_Free(img);
    Res_Free(pal);
    return SavePNG(argb.data(), w, h, path);
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 4 || !Vfs_Mount(argv[1])) {
        fprintf(stderr, "usage: packtool <data> icon|image|survey ...\n");
        return 2;
    }
    std::string cmd = argv[2];
    if (cmd == "icon") {
        ExeCursor icon;
        if (!ExeRes_Load() || !ExeRes_LoadIcon("OURICONA32", &icon)) {
            fprintf(stderr, "packtool: no icon in 4G.EXE\n");
            return 1;
        }
        return SavePNG(icon.argb.data(), icon.w, icon.h, argv[3]) ? 0 : 1;
    }
    for (const char* n : kArchives) Res_OpenArchive(n);
    Gfx_Init();
    if (cmd == "image" && argc >= 6)
        return SaveImage((u32)strtoul(argv[3], nullptr, 0), (u32)strtoul(argv[4], nullptr, 0), argv[5]) ? 0 : 1;
    if (cmd == "survey") {
        // Large images, each paired with the next 768-byte resource (usually its palette).
        for (u32 id = 0; id < 14000; id++) {
            void* r = Res_Load(id);
            if (!r) continue;
            bool big = memcmp(r, "P800", 4) == 0 && Img_W(r) >= 400 && Img_H(r) >= 300;
            Res_Free(r);
            if (!big) continue;
            for (u32 p = id + 1; p < id + 8; p++) {
                void* pal = Res_Load(p);
                int size = pal ? Res_LastSize() : 0;
                if (pal) Res_Free(pal);
                if (size == 768 || size == 769) {
                    char path[512];
                    snprintf(path, sizeof path, "%s/%04x_%04x.png", argv[3], id, p);
                    SaveImage(id, p, path);
                    break;
                }
            }
        }
        return 0;
    }
    fprintf(stderr, "packtool: unknown command %s\n", cmd.c_str());
    return 2;
}
