#include "engine/hostfs.h"

#include <cstring>
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

namespace {

fs::path ToPath(const std::string& utf8) {
#if defined(_WIN32)
    return fs::u8path(utf8);
#else
    return fs::path(utf8);
#endif
}

std::string FromPath(const fs::path& p) {
#if defined(_WIN32)
    return p.u8string();
#else
    return p.string();
#endif
}

} // namespace

FILE* Host_FOpen(const std::string& path, const char* mode) {
#if defined(_WIN32)
    std::wstring wmode(mode, mode + strlen(mode));
    return _wfopen(ToPath(path).c_str(), wmode.c_str());
#else
    return fopen(path.c_str(), mode);
#endif
}

int Host_Seek(FILE* f, int64_t off, int whence) {
#if defined(_WIN32)
    return _fseeki64(f, off, whence);
#else
    return fseeko(f, (off_t)off, whence);
#endif
}

int64_t Host_Tell(FILE* f) {
#if defined(_WIN32)
    return _ftelli64(f);
#else
    return ftello(f);
#endif
}

bool Host_Stat(const std::string& path, bool* isDir, uint64_t* size) {
    std::error_code ec;
    fs::file_status st = fs::status(ToPath(path), ec);
    if (ec || !fs::exists(st)) return false;
    bool dir = fs::is_directory(st);
    if (isDir) *isDir = dir;
    if (size) {
        *size = 0;
        if (!dir) {
            uintmax_t n = fs::file_size(ToPath(path), ec);
            if (!ec) *size = n;
        }
    }
    return true;
}

bool Host_IsDir(const std::string& path) {
    bool dir = false;
    return Host_Stat(path, &dir, nullptr) && dir;
}

bool Host_Exists(const std::string& path) { return Host_Stat(path, nullptr, nullptr); }

bool Host_Remove(const std::string& path) {
    std::error_code ec;
    return fs::remove(ToPath(path), ec);
}

std::vector<HostDirEntry> Host_ListDir(const std::string& path) {
    std::vector<HostDirEntry> out;
    std::error_code ec;
    fs::directory_iterator it(ToPath(path), fs::directory_options::skip_permission_denied, ec);
    for (; !ec && it != fs::directory_iterator(); it.increment(ec)) {
        HostDirEntry e;
        e.name = FromPath(it->path().filename());
        std::error_code ec2;
        e.isDir = it->is_directory(ec2);
        if (!e.isDir) {
            uintmax_t n = it->file_size(ec2);
            if (!ec2) e.size = n;
        }
        out.push_back(std::move(e));
    }
    return out;
}
