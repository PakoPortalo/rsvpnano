#include "input/TouchHandler.h"
#include "platform/DesktopInput.h"

// Desktop replacement — reads from gPendingTouch set by mouse events in axs15231b_desktop.cpp.

bool TouchHandler::begin() {
    initialized_   = true;
    touchActive_   = false;
    lastX_         = 0;
    lastY_         = 0;
    return true;
}

void TouchHandler::end() {
    initialized_ = false;
}

void TouchHandler::cancel() {
    touchActive_ = false;
    gPendingTouch = {};
}

bool TouchHandler::poll(TouchEvent& event) {
    if (!initialized_) return false;
    if (!gPendingTouch.pending) return false;

    event.touched = gPendingTouch.touched;
    event.x       = gPendingTouch.x;
    event.y       = gPendingTouch.y;

    switch (gPendingTouch.phase) {
        case 0: event.phase = TouchPhase::Start; break;
        case 1: event.phase = TouchPhase::Move;  break;
        default: event.phase = TouchPhase::End;  break;
    }

    gPendingTouch.pending = false;
    return true;
}

bool TouchHandler::readTouchPacket(uint8_t*, size_t) { return false; }
