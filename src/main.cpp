// Entry point: find the original game data, then run the ported game.
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <string>
#include <vector>

#include "engine/engine.h"
#include "engine/hostfs.h"
#include "engine/vfs.h"
#include "game/game.h"

namespace {

bool LooksLikeGame() { return Vfs_Exists("BOSCO.DF") && Vfs_Exists("SUPPORT\\CMN\\4G\\4G.EXE"); }

bool TryMount(const std::string& path) {
    if (path.empty()) return false;
    if (!Vfs_Mount(path)) return false;
    if (!LooksLikeGame()) return false;
    SDL_Log("game data: %s", path.c_str());
    return true;
}

std::string StripSlash(std::string dir) {
    while (dir.size() > 1 && (dir.back() == '/' || dir.back() == '\\') && !(dir.size() == 3 && dir[1] == ':'))
        dir.pop_back();
    return dir;
}

// Look for "*.iso" files or a disc folder inside dir.
bool ScanDir(const std::string& dirIn) {
    std::string dir = StripSlash(dirIn);
    if (TryMount(dir)) return true;
    for (const HostDirEntry& e : Host_ListDir(dir)) {
        const std::string& n = e.name;
        if (e.isDir || n.size() <= 4) continue;
        std::string ext = n.substr(n.size() - 4);
        for (char& c : ext) c = (char)tolower((unsigned char)c);
        if (ext == ".iso" && TryMount(dir + "/" + n)) return true;
    }
    return false;
}

// Game data installed alongside the program by the installers
// (packaging/): next to the executable (Windows), in the app bundle's
// Resources (macOS) or in ../share/jumpstart4 (Linux .deb and AppImage).
bool FindBundledData() {
    const char* base = SDL_GetBasePath();
    if (!base) return false;
    std::string dir = StripSlash(base);
    for (const std::string& d : {dir, dir + "/Disc", dir + "/../share/jumpstart4", dir + "/../share/jumpstart4/Disc"})
        if (Host_IsDir(d) && ScanDir(d)) return true;
    return false;
}

// Mounted CDs and disc images.
std::vector<std::string> VolumeDirs() {
    std::vector<std::string> roots;
#if defined(_WIN32)
    for (char c = 'C'; c <= 'Z'; c++) roots.push_back(std::string(1, c) + ":\\");
    return roots;
#elif defined(__APPLE__)
    roots.push_back("/Volumes");
#else
    const char* user = SDL_getenv("USER");
    if (user) {
        roots.push_back(std::string("/media/") + user);
        roots.push_back(std::string("/run/media/") + user);
    }
    roots.push_back("/media");
    roots.push_back("/mnt");
#endif
    std::vector<std::string> out;
    for (const std::string& r : roots)
        for (const HostDirEntry& e : Host_ListDir(r))
            if (e.isDir && e.name[0] != '.') out.push_back(r + "/" + e.name);
    return out;
}

bool g_dialogDone;
std::string g_dialogPath;

void SDLCALL DialogCallback(void*, const char* const* files, int) {
    if (files && files[0]) g_dialogPath = files[0];
    g_dialogDone = true;
}

bool AskForData() {
    for (;;) {
        const SDL_MessageBoxButtonData buttons[] = {
            {0, 2, "Quit"},
            {0, 1, "Choose Folder..."},
            {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Choose Disc Image..."},
        };
        SDL_MessageBoxData box = {SDL_MESSAGEBOX_INFORMATION, nullptr, "JumpStart 4th Grade",
                                  "Please locate your JumpStart 4th Grade: Haunted Island CD. "
                                  "You can choose the .iso disc image or a folder containing the "
                                  "disc's files.",
                                  3, buttons, nullptr};
        int choice = 2;
        if (!SDL_ShowMessageBox(&box, &choice) || choice == 2) return false;
        g_dialogDone = false;
        g_dialogPath.clear();
        if (choice == 0) {
            static const SDL_DialogFileFilter filters[] = {{"Disc images", "iso;bin;img;cdr"}, {"All files", "*"}};
            SDL_ShowOpenFileDialog(DialogCallback, nullptr, nullptr, filters, 2, nullptr, false);
        } else {
            SDL_ShowOpenFolderDialog(DialogCallback, nullptr, nullptr, nullptr, false);
        }
        while (!g_dialogDone) {
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
            }
            SDL_Delay(10);
        }
        if (g_dialogPath.empty()) continue;
        if (TryMount(g_dialogPath)) {
            Ini_WriteString("Data", "Path", g_dialogPath.c_str());
            return true;
        }
        Sys_MessageBox("JumpStart 4th Grade", "That location doesn't contain the JumpStart 4th Grade disc files.");
    }
}

bool FindData(int argc, char** argv) {
    if (argc > 1 && TryMount(argv[1])) return true;
    if (FindBundledData()) return true;
    char saved[1024];
    Ini_GetString("Data", "Path", saved, sizeof saved);
    if (TryMount(saved)) return true;
    // Next to the app bundle / executable, and a few folders up from it
    // (the bundle may live in a build directory inside the game folder).
    if (const char* base = SDL_GetBasePath()) {
        std::string dir = base;
        for (int up = 0; up < 6 && dir.size() > 1; up++) {
            dir = StripSlash(dir);
            if (ScanDir(dir) || ScanDir(dir + "/game")) return true;
            size_t slash = dir.find_last_of("/\\");
            if (slash == std::string::npos) break;
            dir = slash == 0 ? "/" : dir.substr(0, slash + (dir[slash - 1] == ':' ? 1 : 0));
        }
    }
    for (const std::string& vol : VolumeDirs())
        if (TryMount(vol)) return true;
    return AskForData();
}

} // namespace

int main(int argc, char** argv) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) return 1;
    if (!FindData(argc, argv)) return 0;
    if (!ExeRes_Load()) {
        Sys_MessageBox("JumpStart 4th Grade", "Couldn't read 4G.EXE from the game disc.");
        return 1;
    }
    if (!Sys_Init()) return 1;
    App_InitEngine();
    int rc = Game_Main();
    App_ShutdownEngine();
    Sys_Shutdown();
    return rc;
}
