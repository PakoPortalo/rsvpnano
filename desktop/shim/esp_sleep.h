#pragma once
#include <cstdlib>
#include <cstdint>

typedef int gpio_num_t;
typedef int esp_sleep_ext1_wakeup_mode_t;
typedef int esp_sleep_source_t;
#define ESP_SLEEP_WAKEUP_GPIO 5
#define ESP_SLEEP_WAKEUP_EXT0 2
#define ESP_SLEEP_WAKEUP_UNDEFINED 0
#define ESP_SLEEP_DISABLE_WAKEUP_SOURCE 0

inline esp_sleep_source_t esp_sleep_get_wakeup_cause() { return static_cast<esp_sleep_source_t>(ESP_SLEEP_WAKEUP_UNDEFINED); }
inline void esp_sleep_enable_ext0_wakeup(gpio_num_t, int) {}
inline void esp_sleep_enable_gpio_wakeup() {}
inline void esp_sleep_disable_wakeup_source(int) {}
inline void esp_light_sleep_start() {}
inline void esp_deep_sleep_start() { exit(0); }
