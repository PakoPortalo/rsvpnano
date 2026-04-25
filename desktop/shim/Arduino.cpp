#include "Arduino.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <thread>

// ---------- globals ----------
int gDesktopPinState[64] = {};
SerialClass Serial;

// ---------- timing ----------
static auto kEpoch = std::chrono::steady_clock::now();

uint32_t millis() {
    const auto now = std::chrono::steady_clock::now();
    return static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now - kEpoch).count());
}

void delay(uint32_t ms) {
    // Pump SDL events while waiting — defined weak here, overridden by axs15231b_desktop.cpp
    extern void desktopPumpEvents();
    const uint32_t end = millis() + ms;
    while (millis() < end) {
        desktopPumpEvents();
        if (ms > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

void delayMicroseconds(uint32_t us) {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

// ---------- String float constructors ----------
String::String(float v, int decimals) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%.*f", decimals, static_cast<double>(v));
    s_ = buf;
}

String::String(double v, int decimals) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%.*f", decimals, v);
    s_ = buf;
}
