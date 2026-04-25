#include "Preferences.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>

static std::string getHomeDir() {
    const char* home = std::getenv("HOME");
    return home ? std::string(home) : std::string(".");
}

std::string Preferences::prefsFilePath() const {
    return getHomeDir() + "/.rsvpnano_prefs";
}

std::string Preferences::qualifiedKey(const char* key) const {
    return ns_ + ":" + (key ? key : "");
}

void Preferences::load() {
    store_.clear();
    std::ifstream f(prefsFilePath());
    if (!f.is_open()) return;

    std::string line;
    while (std::getline(f, line)) {
        const auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string k = line.substr(0, eq);
        std::string v = line.substr(eq + 1);
        store_[k] = v;
    }
}

void Preferences::save() const {
    if (readOnly_) return;
    std::ofstream f(prefsFilePath());
    for (const auto& kv : store_) {
        f << kv.first << "=" << kv.second << "\n";
    }
}

bool Preferences::begin(const char* name, bool readOnly, const char*) {
    ns_ = name ? name : "default";
    readOnly_ = readOnly;
    load();
    return true;
}

void Preferences::end() {
    save();
    store_.clear();
    ns_.clear();
}

void Preferences::clear() {
    const std::string prefix = ns_ + ":";
    for (auto it = store_.begin(); it != store_.end(); ) {
        if (it->first.rfind(prefix, 0) == 0) it = store_.erase(it);
        else ++it;
    }
    save();
}

bool Preferences::isKey(const char* key) const {
    return store_.count(qualifiedKey(key)) > 0;
}

bool Preferences::remove(const char* key) {
    const auto it = store_.find(qualifiedKey(key));
    if (it == store_.end()) return false;
    store_.erase(it);
    save();
    return true;
}

// ---- generic put/get helpers ----
template<typename T>
static bool putValue(std::map<std::string,std::string>& store,
                     const std::string& k, T v) {
    store[k] = std::to_string(v);
    return true;
}

template<typename T>
static T getValue(const std::map<std::string,std::string>& store,
                  const std::string& k, T def) {
    const auto it = store.find(k);
    if (it == store.end()) return def;
    try {
        if constexpr (std::is_floating_point_v<T>) {
            return static_cast<T>(std::stod(it->second));
        } else if constexpr (std::is_signed_v<T>) {
            return static_cast<T>(std::stoll(it->second));
        } else {
            return static_cast<T>(std::stoull(it->second));
        }
    } catch (...) { return def; }
}

#define IMPL_PUT(method, type) \
    bool Preferences::method(const char* key, type value) { \
        store_[qualifiedKey(key)] = std::to_string(value); \
        save(); return true; }

#define IMPL_GET(method, type, conv) \
    type Preferences::method(const char* key, type dv) const { \
        return getValue<type>(store_, qualifiedKey(key), dv); }

IMPL_PUT(putBool,   bool)
IMPL_PUT(putChar,   int8_t)
IMPL_PUT(putUChar,  uint8_t)
IMPL_PUT(putShort,  int16_t)
IMPL_PUT(putUShort, uint16_t)
IMPL_PUT(putInt,    int32_t)
IMPL_PUT(putUInt,   uint32_t)
IMPL_PUT(putLong64, int64_t)
IMPL_PUT(putULong64,uint64_t)
IMPL_PUT(putFloat,  float)
IMPL_PUT(putDouble, double)

IMPL_GET(getBool,   bool,     stoll)
IMPL_GET(getChar,   int8_t,   stoll)
IMPL_GET(getUChar,  uint8_t,  stoull)
IMPL_GET(getShort,  int16_t,  stoll)
IMPL_GET(getUShort, uint16_t, stoull)
IMPL_GET(getInt,    int32_t,  stoll)
IMPL_GET(getUInt,   uint32_t, stoull)
IMPL_GET(getLong64, int64_t,  stoll)
IMPL_GET(getULong64,uint64_t, stoull)
IMPL_GET(getFloat,  float,    stod)
IMPL_GET(getDouble, double,   stod)

bool Preferences::putString(const char* key, const char* value) {
    store_[qualifiedKey(key)] = value ? value : "";
    save();
    return true;
}

String Preferences::getString(const char* key, const char* defaultValue) const {
    const auto it = store_.find(qualifiedKey(key));
    if (it == store_.end()) return String(defaultValue ? defaultValue : "");
    return String(it->second.c_str());
}
