#include "engine/file.h"

#include <cstdio>
#include <string>

#include "engine/hostfs.h"
#include "engine/vfs.h"

char g_exeDir[260];

namespace {

struct Handle {
    FILE* fp = nullptr;
    VFile* vf = nullptr;
};
Handle g_handles[32];

bool IsSavePath(const char* path) {
    size_t n = strlen(g_exeDir);
    return n && strncmp(path, g_exeDir, n) == 0;
}

std::string HostPath(const char* path) {
    std::string s = path;
    for (char& c : s)
        if (c == '\\') c = '/';
    return s;
}

int Alloc() {
    for (int i = 0; i < 32; i++)
        if (!g_handles[i].fp && !g_handles[i].vf) return i;
    return -1;
}

} // namespace

int File_Open(const char* path, int mode) {
    int h = Alloc();
    if (h < 0) return -1;
    if (IsSavePath(path)) {
        std::string p = HostPath(path);
        const char* m = mode == FILE_READ ? "rb" : mode == FILE_CREATE ? "wb" : "r+b";
        FILE* fp = Host_FOpen(p, m);
        if (!fp) return -1;
        if (mode == FILE_APPEND) fseek(fp, 0, SEEK_END);
        g_handles[h].fp = fp;
        return h;
    }
    if (mode != FILE_READ) return -1;
    VFile* vf = Vfs_Open(path);
    if (!vf) return -1;
    g_handles[h].vf = vf;
    return h;
}

void File_Close(int h) {
    if (h < 0 || h >= 32) return;
    if (g_handles[h].fp) fclose(g_handles[h].fp);
    if (g_handles[h].vf) Vfs_Close(g_handles[h].vf);
    g_handles[h] = Handle();
}

bool File_Delete(const char* path) {
    if (!IsSavePath(path)) return false;
    return Host_Remove(HostPath(path));
}

int File_Read(int h, void* buf, int n) {
    if (h < 0 || h >= 32) return -1;
    if (g_handles[h].fp) return (int)fread(buf, 1, n, g_handles[h].fp);
    if (g_handles[h].vf) return (int)Vfs_Read(g_handles[h].vf, buf, n);
    return -1;
}

int File_Write(int h, const void* buf, int n) {
    if (h < 0 || h >= 32 || !g_handles[h].fp) return -1;
    return (int)fwrite(buf, 1, n, g_handles[h].fp);
}

int File_Seek(int h, int off, int whence) {
    if (h < 0 || h >= 32) return -1;
    if (g_handles[h].fp) {
        fseek(g_handles[h].fp, off, whence);
        return (int)ftell(g_handles[h].fp);
    }
    if (g_handles[h].vf) return (int)Vfs_Seek(g_handles[h].vf, off, whence);
    return -1;
}

int File_Size(int h) {
    int cur = File_Seek(h, 0, SEEK_CUR);
    int end = File_Seek(h, 0, SEEK_END);
    File_Seek(h, cur, SEEK_SET);
    return end;
}

bool File_Exists(const char* path) {
    if (IsSavePath(path)) return Host_Exists(HostPath(path));
    return Vfs_Exists(path);
}

void* File_LoadAll(const char* path) {
    int h = File_Open(path, FILE_READ);
    if (h < 0) return nullptr;
    int n = File_Size(h);
    u8* buf = (u8*)Mem_Alloc(n + 1);
    File_Read(h, buf, n);
    File_Close(h);
    buf[n] = 0;
    return buf;
}

int File_Copy(const char* src, const char* dst, int confirm) {
    (void)confirm;
    int in = File_Open(src, FILE_READ);
    if (in < 0) return -2;
    int n = File_Size(in);
    u8* buf = nullptr;
    if (n > 0) {
        buf = (u8*)Mem_Alloc(n);
        if (File_Read(in, buf, n) != n) {
            Mem_Free(buf);
            File_Close(in);
            return -2;
        }
    }
    File_Close(in);
    int out = File_Open(dst, FILE_CREATE);
    if (out < 0) {
        Mem_Free(buf);
        return -3;
    }
    if (n > 0 && File_Write(out, buf, n) != n) {
        Mem_Free(buf);
        File_Close(out);
        return -3;
    }
    Mem_Free(buf);
    File_Close(out);
    return 0;
}
