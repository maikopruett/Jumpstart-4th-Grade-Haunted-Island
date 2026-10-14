// Host file system access with UTF-8 paths on every platform (Windows needs
// wide-character APIs for non-ASCII paths, e.g. a user name with accents).
#pragma once
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

struct HostDirEntry {
    std::string name;
    bool isDir = false;
    uint64_t size = 0;
};

FILE* Host_FOpen(const std::string& path, const char* mode);
int Host_Seek(FILE* f, int64_t off, int whence);
int64_t Host_Tell(FILE* f);
bool Host_Stat(const std::string& path, bool* isDir, uint64_t* size);
bool Host_IsDir(const std::string& path);
bool Host_Exists(const std::string& path);
bool Host_Remove(const std::string& path);
std::vector<HostDirEntry> Host_ListDir(const std::string& path);
