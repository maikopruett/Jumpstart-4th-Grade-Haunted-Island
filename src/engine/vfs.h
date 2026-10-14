// Read-only access to the original game disc, either an extracted folder or
// an ISO 9660 image. Paths use the game's DOS conventions ("Movies\M\x.smk")
// and are resolved case-insensitively.
#pragma once
#include <cstddef>
#include <cstdint>
#include <string>

struct VFile;

bool Vfs_Mount(const std::string& pathToFolderOrIso);
bool Vfs_IsMounted();
const std::string& Vfs_RootDescription();

VFile* Vfs_Open(const char* path);
size_t Vfs_Read(VFile* f, void* buf, size_t n);
int64_t Vfs_Seek(VFile* f, int64_t off, int whence); // returns new position
int64_t Vfs_Tell(VFile* f);
int64_t Vfs_Size(VFile* f);
void Vfs_Close(VFile* f);
bool Vfs_Exists(const char* path);

// Reads a whole file; returns false if missing.
bool Vfs_ReadAll(const char* path, std::string& out);
