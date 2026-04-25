#include "SD_MMC.h"

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

SDMMCFS SD_MMC;

// Resolve the books directory at startup:
// 1. $RSVP_BOOKS_DIR env var (absolute path)
// 2. <directory-of-this-binary>/books (set by begin())
// The default is set by begin() based on the mount point argument.

static std::string gBooksDir;

bool SDMMCFS::begin(const char* /*mountPoint*/, bool, bool, int, uint8_t) {
    // Determine where books live on the desktop.
    const char* envDir = std::getenv("RSVP_BOOKS_DIR");
    if (envDir && envDir[0]) {
        booksDir_ = envDir;
    } else {
        // Default: <binary-dir>/books
        // __FILE__ isn't useful here; use a fallback relative path.
        // The binary should be run from the desktop/ directory.
        char cwd[4096] = {};
        if (getcwd(cwd, sizeof(cwd))) {
            booksDir_ = std::string(cwd) + "/books";
        } else {
            booksDir_ = "books";
        }
    }
    gBooksDir = booksDir_;

    // Check the directory exists
    DIR* d = opendir(booksDir_.c_str());
    if (!d) {
        fprintf(stderr, "[SD_MMC] books dir not found: %s\n", booksDir_.c_str());
        fprintf(stderr, "[SD_MMC] set RSVP_BOOKS_DIR or place a books/ folder next to the binary\n");
        // Still mark as mounted; StorageManager will handle empty book list gracefully
    } else {
        closedir(d);
        printf("[SD_MMC] books dir: %s\n", booksDir_.c_str());
    }

    mounted_ = true;
    return true;
}

void SDMMCFS::end() {
    mounted_ = false;
}

// Map an SD-style path (/books/...) to the desktop books directory.
// SD_MMC on ESP32 prepends /sdcard internally, so StorageManager passes paths like:
//   "/books"           → list the books directory
//   "/books/foo.txt"   → open foo.txt in books directory
std::string SDMMCFS::mapPath(const char* sdPath) const {
    if (!sdPath) return booksDir_;

    // Strip leading /sdcard if somehow present
    if (strncmp(sdPath, "/sdcard", 7) == 0) {
        sdPath += 7;
    }

    // "/books" → booksDir_
    // "/books/X" → booksDir_ + "/X"
    if (strncmp(sdPath, "/books", 6) == 0) {
        const char* rest = sdPath + 6;
        if (rest[0] == '\0') {
            return booksDir_;
        }
        if (rest[0] == '/') {
            return booksDir_ + rest;
        }
    }

    // Fallback: treat as relative to books dir
    return booksDir_ + "/" + sdPath;
}

File SDMMCFS::open(const char* sdPath, const char* /*mode*/) {
    const std::string diskPath = mapPath(sdPath);

    struct stat st;
    if (stat(diskPath.c_str(), &st) != 0) {
        return File();  // not found
    }

    // Extract just the filename component for name()
    std::string name = diskPath;
    const auto slash = name.rfind('/');
    if (slash != std::string::npos) name = name.substr(slash + 1);

    if (S_ISDIR(st.st_mode)) {
        DIR* d = opendir(diskPath.c_str());
        if (!d) return File();
        return File::makeDir(d, diskPath, name);
    } else {
        FILE* fp = fopen(diskPath.c_str(), "rb");
        if (!fp) return File();
        return File::makeFile(fp, diskPath, name, static_cast<size_t>(st.st_size));
    }
}
