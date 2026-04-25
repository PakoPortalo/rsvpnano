#include "board/BoardConfig.h"
#include "platform/DesktopInput.h"
#include <cstdio>
#include <thread>
#include <chrono>

namespace BoardConfig {

void begin() {
    printf("[board] Desktop simulator — no hardware init\n");
}

void lightSleepUntilBootButton() {
    printf("[board] Waiting for Space (BOOT button) to wake...\n");
    // Spin in event pump until BOOT button (pin 0) is pressed
    gDesktopPinState[0] = 1;  // start released
    while (gDesktopPinState[0] != 0) {
        desktopPumpEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

bool readBatteryStatus(BatteryStatus& status) {
    status.present = true;
    status.voltage  = 4.1f;
    status.percent  = 85;
    return true;
}

bool releaseBatteryPowerHold() { return true; }

}  // namespace BoardConfig
