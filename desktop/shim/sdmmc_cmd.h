#pragma once
#include "driver/sdmmc_types.h"
typedef int esp_err_t;
#define ESP_OK 0
inline esp_err_t sdmmc_read_sectors(sdmmc_card_t*, void*, size_t, size_t) { return 1; }
inline esp_err_t sdmmc_write_sectors(sdmmc_card_t*, const void*, size_t, size_t) { return 1; }
inline esp_err_t sdmmc_init_ocr(sdmmc_card_t*) { return 1; }
