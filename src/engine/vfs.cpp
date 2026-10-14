#include "engine/vfs.h"

#include <cctype>
#include <cstdio>
#include <cstring>
#include <map>
#include <memory>
#include <vector>

#include "engine/hostfs.h"

namespace {

std::string Lower(std::string s) {
    for (char& c : s) c = (char)tolower((unsigned char)c);
    return s;
}

// Split a DOS/Unix style relative path into lowercase components.
std::vector<std::string> SplitPath(const char* path) {
    std::vector<std::string> parts;
    std::string cur;
    for (const char* p = path; *p; p++) {
        if (*p == '\\' || *p == '/') {
            if (!cur.empty()) parts.push_back(Lower(cur));
            cur.clear();
        } else if (*p == ':' ) {
            cur.clear(); // drop drive letters
        } else {
            cur += *p;
        }
    }
    if (!cur.empty()) parts.push_back(Lower(cur));
    return parts;
}

struct Entry {
    std::string realName;
    bool isDir = false;
    uint32_t extent = 0; // ISO only
    uint32_t size = 0;
    bool loaded = false; // ISO dirs: children read
    std::map<std::string, std::unique_ptr<Entry>> children;
};

enum class Kind { None, Folder, Iso };
Kind g_kind = Kind::None;
std::string g_root;
std::string g_desc;
FILE* g_iso = nullptr;
Entry g_rootEntry;

bool IsoReadAt(uint64_t off, void* buf, size_t n) {
    if (Host_Seek(g_iso, (int64_t)off, SEEK_SET) != 0) return false;
    return fread(buf, 1, n, g_iso) == n;
}

void IsoLoadDir(Entry& dir) {
    if (dir.loaded) return;
    dir.loaded = true;
    std::vector<uint8_t> buf(dir.size);
    if (!IsoReadAt((uint64_t)dir.extent * 2048, buf.data(), buf.size())) return;
    size_t pos = 0;
    while (pos < buf.size()) {
        uint8_t len = buf[pos];
        if (len == 0) { // records never span sectors; skip to next sector
            pos = (pos / 2048 + 1) * 2048;
            continue;
        }
        if (pos + len > buf.size()) break;
        const uint8_t* r = &buf[pos];
        uint32_t extent = r[2] | r[3] << 8 | r[4] << 16 | (uint32_t)r[5] << 24;
        uint32_t size = r[10] | r[11] << 8 | r[12] << 16 | (uint32_t)r[13] << 24;
        uint8_t flags = r[25];
        uint8_t nameLen = r[32];
        std::string name((const char*)r + 33, nameLen);
        pos += len;
        if (nameLen == 1 && (name[0] == 0 || name[0] == 1)) continue; // . and ..
        size_t semi = name.find(';');
        if (semi != std::string::npos) name.resize(semi);
        if (!name.empty() && name.back() == '.') name.pop_back();
        auto e = std::make_unique<Entry>();
        e->realName = name;
        e->isDir = (flags & 2) != 0;
        e->extent = extent;
        e->size = size;
        dir.children[Lower(name)] = std::move(e);
    }
}

void FolderLoadDir(Entry& dir, const std::string& realPath) {
    if (dir.loaded) return;
    dir.loaded = true;
    for (const HostDirEntry& de : Host_ListDir(realPath)) {
        auto e = std::make_unique<Entry>();
        e->realName = de.name;
        e->isDir = de.isDir;
        e->size = (uint32_t)de.size;
        dir.children[Lower(de.name)] = std::move(e);
    }
}

// Resolve a game path; on success fills the entry and (for folders) real path.
Entry* Resolve(const char* path, std::string* realPath) {
    Entry* cur = &g_rootEntry;
    std::string real = g_root;
    for (const std::string& part : SplitPath(path)) {
        if (part == ".") continue;
        if (!cur->isDir) return nullptr;
        if (g_kind == Kind::Iso) IsoLoadDir(*cur);
        else FolderLoadDir(*cur, real);
        auto it = cur->children.find(part);
        if (it == cur->children.end()) return nullptr;
        cur = it->second.get();
        real += "/" + cur->realName;
    }
    if (realPath) *realPath = real;
    return cur;
}

} // namespace

struct VFile {
    FILE* fp = nullptr;  // folder mode
    uint64_t base = 0;   // iso mode
    int64_t size = 0;
    int64_t pos = 0;
};

bool Vfs_Mount(const std::string& path) {
    bool isDir = false;
    if (!Host_Stat(path, &isDir, nullptr)) return false;
    g_rootEntry = Entry();
    g_rootEntry.isDir = true;
    if (g_iso) { fclose(g_iso); g_iso = nullptr; }
    if (isDir) {
        g_kind = Kind::Folder;
        g_root = path;
        g_desc = path;
        return true;
    }
    g_iso = Host_FOpen(path, "rb");
    if (!g_iso) return false;
    uint8_t pvd[2048];
    if (!IsoReadAt(16 * 2048, pvd, sizeof pvd) || pvd[0] != 1 || memcmp(pvd + 1, "CD001", 5) != 0) {
        fclose(g_iso);
        g_iso = nullptr;
        return false;
    }
    const uint8_t* rootRec = pvd + 156;
    g_rootEntry.extent = rootRec[2] | rootRec[3] << 8 | rootRec[4] << 16 | (uint32_t)rootRec[5] << 24;
    g_rootEntry.size = rootRec[10] | rootRec[11] << 8 | rootRec[12] << 16 | (uint32_t)rootRec[13] << 24;
    g_kind = Kind::Iso;
    g_root.clear();
    g_desc = path;
    return true;
}

bool Vfs_IsMounted() { return g_kind != Kind::None; }
const std::string& Vfs_RootDescription() { return g_desc; }

VFile* Vfs_Open(const char* path) {
    std::string real;
    Entry* e = Resolve(path, &real);
    if (!e || e->isDir) return nullptr;
    VFile* f = new VFile;
    if (g_kind == Kind::Folder) {
        f->fp = Host_FOpen(real, "rb");
        if (!f->fp) { delete f; return nullptr; }
        Host_Seek(f->fp, 0, SEEK_END);
        f->size = Host_Tell(f->fp);
        Host_Seek(f->fp, 0, SEEK_SET);
    } else {
        f->base = (uint64_t)e->extent * 2048;
        f->size = e->size;
    }
    return f;
}

size_t Vfs_Read(VFile* f, void* buf, size_t n) {
    if (!f) return 0;
    if (f->pos >= f->size) return 0;
    if ((int64_t)n > f->size - f->pos) n = (size_t)(f->size - f->pos);
    size_t got;
    if (f->fp) {
        Host_Seek(f->fp, f->pos, SEEK_SET);
        got = fread(buf, 1, n, f->fp);
    } else {
        if (Host_Seek(g_iso, (int64_t)(f->base + f->pos), SEEK_SET) != 0) return 0;
        got = fread(buf, 1, n, g_iso);
    }
    f->pos += got;
    return got;
}

int64_t Vfs_Seek(VFile* f, int64_t off, int whence) {
    if (!f) return -1;
    int64_t np = whence == SEEK_SET ? off : whence == SEEK_CUR ? f->pos + off : f->size + off;
    if (np < 0) np = 0;
    f->pos = np;
    return np;
}

int64_t Vfs_Tell(VFile* f) { return f ? f->pos : -1; }
int64_t Vfs_Size(VFile* f) { return f ? f->size : -1; }

void Vfs_Close(VFile* f) {
    if (!f) return;
    if (f->fp) fclose(f->fp);
    delete f;
}

bool Vfs_Exists(const char* path) {
    Entry* e = Resolve(path, nullptr);
    return e && !e->isDir;
}

bool Vfs_ReadAll(const char* path, std::string& out) {
    VFile* f = Vfs_Open(path);
    if (!f) return false;
    out.resize((size_t)f->size);
    size_t got = Vfs_Read(f, out.data(), out.size());
    Vfs_Close(f);
    out.resize(got);
    return true;
}
