#include "input/ButtonHandler.h"
#include "platform/DesktopInput.h"

// Desktop replacement — reads from gDesktopPinState instead of GPIO.
// Buttons are active-low on the hardware: pin LOW = pressed.
// gDesktopPinState[pin] = 0 means pressed, 1 means released.

ButtonHandler::ButtonHandler(int pin) : pin_(pin) {}

void ButtonHandler::begin() {
    // Ensure pin starts released (high)
    if (pin_ >= 0 && pin_ < 64) gDesktopPinState[pin_] = 1;
    held_             = false;
    pressedEvent_     = false;
    releasedEvent_    = false;
    lastEdgeMs_       = millis();
    pressStartedMs_   = 0;
    lastHoldDurationMs_ = 0;
}

void ButtonHandler::update(uint32_t nowMs) {
    pressedEvent_  = false;
    releasedEvent_ = false;

    const bool currentHeld = (pin_ >= 0 && pin_ < 64) ? (gDesktopPinState[pin_] == 0) : false;

    if (currentHeld != held_) {
        held_         = currentHeld;
        lastEdgeMs_   = nowMs;
        if (held_) {
            pressStartedMs_ = nowMs;
            pressedEvent_   = true;
        } else {
            lastHoldDurationMs_ = nowMs - pressStartedMs_;
            releasedEvent_      = true;
        }
    }
}

bool     ButtonHandler::isHeld()           const { return held_; }
bool     ButtonHandler::wasPressedEvent()  const { return pressedEvent_; }
bool     ButtonHandler::wasReleasedEvent() const { return releasedEvent_; }
uint32_t ButtonHandler::lastEdgeMs()       const { return lastEdgeMs_; }
uint32_t ButtonHandler::lastHoldDurationMs() const { return lastHoldDurationMs_; }
uint32_t ButtonHandler::heldDurationMs(uint32_t nowMs) const {
    return held_ ? nowMs - pressStartedMs_ : 0;
}
