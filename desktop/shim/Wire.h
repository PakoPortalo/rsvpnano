#pragma once
#include <cstdint>
#include <cstddef>

struct TwoWire {
    void begin() {}
    void begin(int, int) {}
    void begin(uint8_t, int, int) {}
    void setClock(uint32_t) {}
    void setTimeOut(uint32_t) {}
    void beginTransmission(uint8_t) {}
    uint8_t endTransmission(bool = true) { return 1; }  // 1 = error → TouchHandler sets initialized_=false
    size_t  requestFrom(uint8_t, size_t, bool = true) { return 0; }
    size_t  write(const uint8_t*, size_t) { return 0; }
    size_t  write(uint8_t) { return 0; }
    int     read() { return -1; }
    void    end() {}
};
extern TwoWire Wire;
extern TwoWire Wire1;
