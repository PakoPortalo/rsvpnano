#pragma once
#include <cstdio>
typedef int esp_err_t;
#define ESP_OK    0
#define ESP_FAIL  (-1)
#define ESP_ERR_INVALID_ARG   0x102
#define ESP_ERR_INVALID_STATE 0x103
#define ESP_ERR_NOT_FOUND     0x105
#define ESP_ERR_NO_MEM        0x101
#define ESP_ERROR_CHECK(x) do { esp_err_t __r = (x); if (__r != ESP_OK) { fprintf(stderr, "[ESP_ERROR_CHECK] err=%d\n", __r); } } while(0)
