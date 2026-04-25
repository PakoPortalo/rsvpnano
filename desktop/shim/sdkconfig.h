#pragma once
// Desktop build — disable all ESP32-specific feature flags
#define CONFIG_TINYUSB_MSC_ENABLED 0
#define CONFIG_TINYUSB_ENABLED     0
#undef  ARDUINO_USB_MODE
#undef  ARDUINO_USB_MSC_ON_BOOT
