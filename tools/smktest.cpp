// Decodes every frame of every .SMK file on the disc to check the decoder.
#include <cstdio>
#include <dirent.h>
#include <string>
#include <vector>

#include "engine/smacker.h"
#include "engine/vfs.h"

static void Walk(const std::string& root, const std::string& rel, std::vector<std::string>& out) {
    DIR* d = opendir((root + "/" + rel).c_str());
    if (!d) return;
    while (dirent* e = readdir(d)) {
        std::string n = e->d_name;
        if (n == "." || n == "..") continue;
        std::string r = rel.empty() ? n : rel + "/" + n;
        if (e->d_type == DT_DIR) Walk(root, r, out);
        else if (n.size() > 4 && (n.substr(n.size() - 4) == ".SMK" || n.substr(n.size() - 4) == ".smk")) out.push_back(r);
    }
    closedir(d);
}

int main(int argc, char** argv) {
    std::string root = argv[1];
    Vfs_Mount(root);
    std::vector<std::string> files;
    Walk(root, "", files);
    long frames = 0;
    int bad = 0;
    for (auto& f : files) {
        SmkFile* s = Smk_Open(f.c_str());
        if (!s) {
            printf("open failed: %s\n", f.c_str());
            bad++;
            continue;
        }
        Rect r;
        bool pal;
        int n = 0;
        while (Smk_NextFrame(s, &r, &pal)) n++;
        if (n != Smk_Frames(s)) {
            printf("%s: decoded %d of %d frames\n", f.c_str(), n, Smk_Frames(s));
            bad++;
        }
        frames += n;
        Smk_Close(s);
    }
    printf("%zu files, %ld frames, %d problems\n", files.size(), frames, bad);
    return bad != 0;
}
