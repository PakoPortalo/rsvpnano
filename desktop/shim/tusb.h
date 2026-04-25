#pragma once
// TinyUSB stub — USB is disabled on desktop (RSVP_USB_TRANSFER_ENABLED=0)
typedef int tusb_speed_t;
inline bool tud_mounted() { return false; }
inline bool tud_ready() { return false; }
