#pragma once
#include <cstdint>
#include <cstddef>

// Stub types — axs15231b.cpp is replaced on desktop, these only need to parse
typedef int spi_host_device_t;
typedef void* spi_device_handle_t;
#define SPI3_HOST 2
#define SPI2_HOST 1
#define SPI_MODE3 0

typedef struct { uint32_t flags; } spi_transaction_t;

typedef struct {
    int mosi_io_num;
    int miso_io_num;
    int sclk_io_num;
    int quadwp_io_num;
    int quadhd_io_num;
    int max_transfer_sz;
    int flags;
} spi_bus_config_t;

typedef struct {
    int command_bits;
    int address_bits;
    int dummy_bits;
    uint8_t mode;
    int clock_speed_hz;
    int spics_io_num;
    int flags;
    int queue_size;
    int clock_source;
} spi_device_interface_config_t;

typedef int esp_err_t;
#define ESP_OK 0
#define SPI_DEVICE_HALFDUPLEX     (1 << 2)
#define SPI_DEVICE_CLK_AS_CS      (1 << 5)
#define SPI_TRANS_MODE_QIO        (1 << 3)
#define SPI_TRANS_MULTILINE_CMD   (1 << 6)
#define SPI_TRANS_MULTILINE_ADDR  (1 << 7)
#define SPICOMMON_BUSFLAG_MASTER  (1 << 1)
#define SPICOMMON_BUSFLAG_QUAD    (1 << 6)
#define SPI_MASTER_FREQ_40M       40000000

inline esp_err_t spi_bus_initialize(spi_host_device_t, const spi_bus_config_t*, int) { return ESP_OK; }
inline esp_err_t spi_bus_add_device(spi_host_device_t, const spi_device_interface_config_t*, spi_device_handle_t*) { return ESP_OK; }
inline esp_err_t spi_device_polling_transmit(spi_device_handle_t, spi_transaction_t*) { return ESP_OK; }
inline esp_err_t spi_device_acquire_bus(spi_device_handle_t, int) { return ESP_OK; }
inline void      spi_device_release_bus(spi_device_handle_t) {}
