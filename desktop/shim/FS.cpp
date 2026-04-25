#include "FS.h"

#include <dirent.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstring>
#include <string>

namespace fs {

// ---- factory helpers ----

File File::makeFile(FILE* fp, const std::string& diskPath, const std::string& name, size_t size) {
    File f;
    f.fp_      = fp;
    f.diskPath_= diskPath;
    f.name_    = name;
    f.isDir_   = false;
    f.size_    = size;
    return f;
}

File File::makeDir(void* d, const std::string& diskPath, const std::string& name) {
    File f;
    f.dir_     = d;
    f.diskPath_= diskPath;
    f.name_    = name;
    f.isDir_   = true;
    f.size_    = 0;
    return f;
}

void File::close() {
    if (fp_) { fclose(fp_); fp_ = nullptr; }
    if (dir_) { closedir(reinterpret_cast<DIR*>(dir_)); dir_ = nullptr; }
}

File File::openNextFile() {
    if (!dir_) return File();

    while (true) {
        struct dirent* entry = readdir(reinterpret_cast<DIR*>(dir_));
        if (!entry) return File();

        // Skip hidden files and . / ..
        if (entry->d_name[0] == '.') continue;

        const std::string childPath = diskPath_ + "/" + entry->d_name;

        struct stat st;
        if (stat(childPath.c_str(), &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            DIR* childDir = opendir(childPath.c_str());
            if (!childDir) continue;
            return File::makeDir(childDir, childPath, entry->d_name);
        } else {
            FILE* fp = fopen(childPath.c_str(), "rb");
            if (!fp) continue;
            return File::makeFile(fp, childPath, entry->d_name,
                                  static_cast<size_t>(st.st_size));
        }
    }
}

}  // namespace fs
