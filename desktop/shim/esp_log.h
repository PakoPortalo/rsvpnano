#pragma once
#include <cstdio>
#define ESP_LOGE(tag, fmt, ...) fprintf(stderr, "[E/%s] " fmt "\n", tag, ##__VA_ARGS__)
#define ESP_LOGW(tag, fmt, ...) fprintf(stderr, "[W/%s] " fmt "\n", tag, ##__VA_ARGS__)
#define ESP_LOGI(tag, fmt, ...) printf("[I/%s] " fmt "\n", tag, ##__VA_ARGS__)
#define ESP_LOGD(tag, fmt, ...) do {} while(0)
#define ESP_LOGV(tag, fmt, ...) do {} while(0)
inline void esp_log_level_set(const char*, int) {}
typedef int esp_log_level_t;
#define ESP_LOG_INFO  3
#define ESP_LOG_DEBUG 4
#define ESP_LOG_ERROR 1
