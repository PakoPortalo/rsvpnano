#pragma once
#include <cstdint>
#include <cstdio>
#include <cstddef>
#include <string>

// DIR* is stored as void* to avoid forward-declaration conflicts with system headers.

namespace fs {

class File {
public:
    File() = default;
    ~File() { close(); }

    File(const File&) = delete;
    File& operator=(const File&) = delete;

    File(File&& o) noexcept
        : fp_(o.fp_), dir_(o.dir_),
          diskPath_(std::move(o.diskPath_)), name_(std::move(o.name_)),
          isDir_(o.isDir_), size_(o.size_) {
        o.fp_    = nullptr;
        o.dir_   = nullptr;
    }

    File& operator=(File&& o) {
        if (this != &o) {
            close();
            fp_      = o.fp_;
            dir_     = o.dir_;
            diskPath_= std::move(o.diskPath_);
            name_    = std::move(o.name_);
            isDir_   = o.isDir_;
            size_    = o.size_;
            o.fp_    = nullptr;
            o.dir_   = nullptr;
        }
        return *this;
    }

    explicit operator bool() const { return fp_ != nullptr || dir_ != nullptr; }
    bool isDirectory() const { return isDir_; }
    size_t size() const { return size_; }
    const char* name() const { return name_.c_str(); }

    int available() const {
        if (!fp_) return 0;
        const long cur = ftell(fp_);
        if (cur < 0) return 0;
        return static_cast<int>(size_) - static_cast<int>(cur);
    }

    int read() {
        if (!fp_) return -1;
        return fgetc(fp_);
    }

    size_t read(uint8_t* buf, size_t len) {
        if (!fp_) return 0;
        return fread(buf, 1, len, fp_);
    }

    bool seek(size_t pos) {
        if (!fp_) return false;
        return fseek(fp_, static_cast<long>(pos), SEEK_SET) == 0;
    }

    size_t position() const {
        if (!fp_) return 0;
        const long p = ftell(fp_);
        return p < 0 ? 0 : static_cast<size_t>(p);
    }

    void close();

    File openNextFile();

    // Internal factory helpers
    static File makeFile(FILE* fp, const std::string& diskPath, const std::string& name, size_t size);
    static File makeDir(void* d, const std::string& diskPath, const std::string& name);

private:
    FILE*       fp_      = nullptr;
    void*       dir_     = nullptr;  // actually DIR*
    std::string diskPath_;  // full disk path (for directory iteration)
    std::string name_;      // filename component only (as returned by name())
    bool        isDir_   = false;
    size_t      size_    = 0;
};

class FS {
public:
    virtual File open(const char* path, const char* mode = "r") = 0;
    virtual File open(const std::string& path, const char* mode = "r") {
        return open(path.c_str(), mode);
    }
};

}  // namespace fs

using File = fs::File;
