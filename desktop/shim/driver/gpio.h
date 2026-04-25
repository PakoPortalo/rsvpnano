#pragma once
#include <cstdint>

typedef int gpio_num_t;
typedef int gpio_int_type_t;
typedef int gpio_mode_t;
typedef int gpio_pull_mode_t;

#define GPIO_INTR_LOW_LEVEL  4
#define GPIO_INTR_ANYEDGE    3
#define GPIO_MODE_OUTPUT     2
#define GPIO_PULLDOWN_ONLY   2
#define GPIO_FLOATING        2

typedef int esp_err_t;
#define ESP_OK 0

inline esp_err_t gpio_wakeup_enable(gpio_num_t, gpio_int_type_t) { return ESP_OK; }
inline esp_err_t gpio_wakeup_disable(gpio_num_t) { return ESP_OK; }
inline esp_err_t gpio_set_direction(gpio_num_t, gpio_mode_t) { return ESP_OK; }
inline esp_err_t gpio_set_pull_mode(gpio_num_t, gpio_pull_mode_t) { return ESP_OK; }
inline esp_err_t gpio_set_level(gpio_num_t, uint32_t) { return ESP_OK; }
inline int       gpio_get_level(gpio_num_t) { return 0; }
