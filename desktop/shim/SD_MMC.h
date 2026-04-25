#pragma once
#include "FS.h"
#include <cstdint>
#include <string>

// Constants normally from ESP-IDF SD_MMC driver
#define SDMMC_FREQ_DEFAULT 20000
#define SDMMC_FREQ_PROBING 400
#define SDMMC_FREQ_52M     52000
#define SDMMC_FREQ_26M     26000

class SDMMCFS : public fs::FS {
public:
    // Maps SD paths (/books/...) to the local desktop books directory.
    bool setPins(int clk, int cmd, int d0) { (void)clk; (void)cmd; (void)d0; return true; }
    bool begin(const char* mountPoint, bool mode1bit = true, bool formatOnFail = false,
               int frequencyKhz = SDMMC_FREQ_DEFAULT, uint8_t maxFiles = 5);
    void end();
    uint64_t cardSize() const { return 4ULL * 1024ULL * 1024ULL * 1024ULL; }

    File open(const char* path, const char* mode = "r") override;
    File open(const std::string& path, const char* mode = "r") override {
        return open(path.c_str(), mode);
    }

private:
    std::string booksDir_;
    bool mounted_ = false;

    std::string mapPath(const char* sdPath) const;
};

extern SDMMCFS SD_MMC;
