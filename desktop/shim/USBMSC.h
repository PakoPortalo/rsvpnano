#pragma once
#include <cstdint>
// USBMSC stub — USB is disabled on desktop
class USBMSC {
public:
    void vendorID(const char*) {}
    void productID(const char*) {}
    void productRevision(const char*) {}
    bool begin() { return false; }
    void end() {}
    operator bool() const { return false; }
};
