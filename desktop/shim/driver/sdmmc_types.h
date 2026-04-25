#pragma once
#include <cstdint>

// Minimal stub — UsbMassStorageManager needs sdmmc_card_t as a member
typedef struct {
    uint32_t num_sectors;
    uint16_t sector_size;
} sdmmc_csd_t;

typedef struct {
    uint32_t flags;
    uint32_t rca;
    uint16_t max_freq_khz;
    sdmmc_csd_t csd;
} sdmmc_card_t;

typedef struct { uint32_t clk; uint32_t cmd; } sdmmc_host_t;
typedef struct { uint32_t width; uint32_t flags; } sdmmc_slot_config_t;
