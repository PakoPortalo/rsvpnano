#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <chrono>
#include <cctype>

// ---------- Arduino macros ----------
#define PROGMEM
#define pgm_read_byte(addr)   (*(const uint8_t*)(addr))
#define pgm_read_word(addr)   (*(const uint16_t*)(addr))
#define pgm_read_dword(addr)  (*(const uint32_t*)(addr))
#define F(s) (s)
#define IRAM_ATTR
#define DRAM_ATTR

// ---------- fixed-width integer types ----------
using std::int8_t;
using std::int16_t;
using std::int32_t;
using std::int64_t;
using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

// ---------- GPIO constants ----------
constexpr int INPUT        = 0x00;
constexpr int INPUT_PULLUP = 0x02;
constexpr int OUTPUT       = 0x01;
constexpr int HIGH         = 0x01;
constexpr int LOW          = 0x00;

// ---------- GPIO functions ----------
extern int gDesktopPinState[64];
inline void     pinMode(int, int) {}
inline int      digitalRead(int pin) { return (pin >= 0 && pin < 64) ? gDesktopPinState[pin] : 0; }
inline void     digitalWrite(int, int) {}
inline void     analogWrite(int, int) {}
inline void     analogWriteResolution(int) {}
inline void     analogWriteFrequency(int) {}
inline int      analogRead(int) { return 0; }
inline uint32_t analogReadMilliVolts(int) { return 0; }
inline void     analogReadResolution(int) {}
inline void     analogSetPinAttenuation(int, int) {}
inline void     yield() {}

// ---------- timing ----------
uint32_t millis();
void     delay(uint32_t ms);
void     delayMicroseconds(uint32_t us);

// ---------- String class ----------
class String {
public:
    String() = default;
    String(const char* s) : s_(s ? s : "") {}
    String(char c) : s_(1, c) {}
    String(int v) : s_(std::to_string(v)) {}
    String(long v) : s_(std::to_string(v)) {}
    String(unsigned v) : s_(std::to_string(v)) {}
    String(unsigned long v) : s_(std::to_string(v)) {}
    String(uint16_t v) : s_(std::to_string(static_cast<unsigned>(v))) {}
    String(uint8_t v) : s_(std::to_string(static_cast<unsigned>(v))) {}
    String(float v, int decimals = 2);
    String(double v, int decimals = 2);
    explicit String(const std::string& s) : s_(s) {}
    String(const String&) = default;
    String(String&&) = default;
    String& operator=(const String& o) { s_ = o.s_; return *this; }
    String& operator=(String&& o) { s_ = std::move(o.s_); return *this; }
    String& operator=(const char* s) { s_ = (s ? s : ""); return *this; }

    const char* c_str() const { return s_.c_str(); }
    size_t      length() const { return s_.size(); }
    bool        isEmpty() const { return s_.empty(); }
    char        operator[](size_t i) const { return s_[i]; }
    char&       operator[](size_t i) { return s_[i]; }

    String& operator+=(const String& rhs) { s_ += rhs.s_; return *this; }
    String& operator+=(const char* rhs)   { if (rhs) s_ += rhs; return *this; }
    String& operator+=(char c)            { s_ += c; return *this; }
    String& operator+=(int v)             { s_ += std::to_string(v); return *this; }

    bool operator==(const String& rhs) const { return s_ == rhs.s_; }
    bool operator==(const char* rhs) const { return s_ == (rhs ? rhs : ""); }
    bool operator!=(const String& rhs) const { return s_ != rhs.s_; }
    bool operator!=(const char* rhs) const { return !(*this == rhs); }
    bool operator<(const String& rhs) const { return s_ < rhs.s_; }

    bool startsWith(const char* prefix) const {
        if (!prefix) return false;
        return s_.rfind(prefix, 0) == 0;
    }
    bool startsWith(const String& prefix) const { return startsWith(prefix.c_str()); }
    bool endsWith(const char* suffix) const {
        if (!suffix) return false;
        const size_t sl = std::strlen(suffix);
        return s_.size() >= sl && s_.compare(s_.size() - sl, sl, suffix) == 0;
    }
    bool endsWith(const String& suffix) const { return endsWith(suffix.c_str()); }

    int indexOf(char c) const {
        const auto pos = s_.find(c);
        return (pos == std::string::npos) ? -1 : static_cast<int>(pos);
    }
    int indexOf(const char* str) const {
        const auto pos = s_.find(str ? str : "");
        return (pos == std::string::npos) ? -1 : static_cast<int>(pos);
    }
    int lastIndexOf(char c) const {
        const auto pos = s_.rfind(c);
        return (pos == std::string::npos) ? -1 : static_cast<int>(pos);
    }
    int lastIndexOf(const char* str) const {
        const auto pos = s_.rfind(str ? str : "");
        return (pos == std::string::npos) ? -1 : static_cast<int>(pos);
    }

    String substring(size_t start, size_t end) const {
        if (start >= s_.size()) return String();
        return String(s_.substr(start, end - start).c_str());
    }
    String substring(size_t start) const {
        if (start >= s_.size()) return String();
        return String(s_.substr(start).c_str());
    }

    void remove(size_t index, size_t count) {
        if (index < s_.size()) s_.erase(index, count);
    }
    void remove(size_t index) {
        if (index < s_.size()) s_.erase(index);
    }

    void trim() {
        auto notspace = [](unsigned char c) -> bool { return !std::isspace(c); };
        s_.erase(s_.begin(), std::find_if(s_.begin(), s_.end(), notspace));
        s_.erase(std::find_if(s_.rbegin(), s_.rend(), notspace).base(), s_.end());
    }
    void toLowerCase() {
        for (char& c : s_) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    void toUpperCase() {
        for (char& c : s_) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }

    long  toInt() const { return std::strtol(s_.c_str(), nullptr, 10); }
    float toFloat() const { return std::strtof(s_.c_str(), nullptr); }

    void reserve(size_t n) { s_.reserve(n); }

    const std::string& std_str() const { return s_; }
    operator std::string() const { return s_; }

private:
    std::string s_;
};

inline String operator+(String lhs, const String& rhs) { lhs += rhs; return lhs; }
inline String operator+(String lhs, const char* rhs)   { lhs += rhs; return lhs; }
inline String operator+(String lhs, char rhs)          { lhs += rhs; return lhs; }
inline String operator+(String lhs, int rhs)           { lhs += String(rhs); return lhs; }
inline String operator+(const char* lhs, const String& rhs) {
    String s(lhs); s += rhs; return s;
}

inline bool operator==(const char* lhs, const String& rhs) { return rhs == lhs; }
inline bool operator!=(const char* lhs, const String& rhs) { return rhs != lhs; }

// ---------- Serial ----------
struct SerialClass {
    void begin(int) {}
    explicit operator bool() const { return true; }

    void println() { puts(""); }
    void println(const char* s) { puts(s ? s : ""); }
    void println(const String& s) { puts(s.c_str()); }
    void println(int v) { printf("%d\n", v); }
    void println(unsigned v) { printf("%u\n", v); }
    void println(float v) { printf("%f\n", v); }

    void print(const char* s) { fputs(s ? s : "", stdout); }
    void print(const String& s) { fputs(s.c_str(), stdout); }
    void print(int v) { printf("%d", v); }
    void print(unsigned v) { printf("%u", v); }

    template<typename... A>
    void printf(const char* fmt, A... args) { ::printf(fmt, args...); }

    void flush() { fflush(stdout); }
};
extern SerialClass Serial;

// ---------- misc Arduino API ----------
inline void randomSeed(unsigned long) {}
inline long random(long min, long max) { return min + (std::rand() % (max - min)); }
inline long random(long max) { return random(0, max); }

// Prevent inclusion of real Arduino headers
#define ARDUINO_H
