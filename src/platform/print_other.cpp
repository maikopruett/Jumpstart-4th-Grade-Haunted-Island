// Printing on Linux and other systems: there is no single print API, so the
// page is saved as a PNG and opened in the desktop's image viewer, from
// which it can be printed.
#include "platform/print.h"

#include <SDL3/SDL.h>

#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>

bool Print_DIB(const void* dib) {
    const uint8_t* p = (const uint8_t*)dib;
    int32_t w, h;
    memcpy(&w, p + 4, 4);
    memcpy(&h, p + 8, 4);
    if (w <= 0 || h == 0) return false;
    bool bottomUp = h > 0;
    if (h < 0) h = -h;
    const uint8_t* quads = p + 0x28;
    const uint8_t* bits = p + 0x28 + 0x400;
    int stride = (w + 3) & ~3;
    SDL_Surface* s = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_XRGB8888);
    if (!s) return false;
    for (int y = 0; y < h; y++) {
        const uint8_t* src = bits + (bottomUp ? (h - 1 - y) : y) * stride;
        uint32_t* dst = (uint32_t*)((uint8_t*)s->pixels + y * s->pitch);
        for (int x = 0; x < w; x++) {
            const uint8_t* q = quads + src[x] * 4;
            dst[x] = (uint32_t)q[2] << 16 | (uint32_t)q[1] << 8 | q[0];
        }
    }
    // Saved with the player's files so it is easy to find again.
    std::string dir;
    if (char* pref = SDL_GetPrefPath("Knowledge Adventure", "JumpStart 4th Grade")) {
        dir = pref;
        SDL_free(pref);
    }
    char name[64];
    time_t now = time(nullptr);
    strftime(name, sizeof name, "Printout %Y-%m-%d %H.%M.%S.png", localtime(&now));
    std::string path = dir + name;
    bool ok = SDL_SavePNG(s, path.c_str());
    SDL_DestroySurface(s);
    if (ok) {
        std::string url = "file://";
        for (unsigned char c : path) {
            if (isalnum(c) || strchr("/-_.~", c)) {
                url += (char)c;
            } else {
                char hex[4];
                snprintf(hex, sizeof hex, "%%%02X", c);
                url += hex;
            }
        }
        SDL_OpenURL(url.c_str());
    }
    return ok;
}
