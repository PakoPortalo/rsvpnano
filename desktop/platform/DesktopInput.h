#pragma once
#include <cstdint>

// Shared desktop input state — read by ButtonHandler_desktop and TouchHandler_desktop

// Pin state array (indexed by pin number). Active-low: 0 = pressed (matches hardware).
extern int gDesktopPinState[64];  // defined in Arduino.cpp

// Touch event queue (single-slot; overwritten on every new event)
struct DesktopTouchEvent {
    bool     pending = false;
    bool     touched = false;
    uint16_t x = 0;
    uint16_t y = 0;
    int      phase = 0;  // 0=Start 1=Move 2=End
};
extern DesktopTouchEvent gPendingTouch;

// Called from delay() to pump SDL events
void desktopPumpEvents();
