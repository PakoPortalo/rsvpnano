#pragma once
#include "Arduino.h"
#include <map>
#include <string>

class Preferences {
public:
    bool begin(const char* name, bool readOnly = false, const char* partitionLabel = nullptr);
    void end();
    void clear();

    bool   isKey(const char* key) const;

    bool   putBool(const char* key, bool value);
    bool   getBool(const char* key, bool defaultValue = false) const;

    bool   putChar(const char* key, int8_t value);
    int8_t getChar(const char* key, int8_t defaultValue = 0) const;

    bool    putUChar(const char* key, uint8_t value);
    uint8_t getUChar(const char* key, uint8_t defaultValue = 0) const;

    bool     putShort(const char* key, int16_t value);
    int16_t  getShort(const char* key, int16_t defaultValue = 0) const;

    bool     putUShort(const char* key, uint16_t value);
    uint16_t getUShort(const char* key, uint16_t defaultValue = 0) const;

    bool    putInt(const char* key, int32_t value);
    int32_t getInt(const char* key, int32_t defaultValue = 0) const;

    bool     putUInt(const char* key, uint32_t value);
    uint32_t getUInt(const char* key, uint32_t defaultValue = 0) const;

    bool    putLong(const char* key, int32_t value)  { return putInt(key, value); }
    int32_t getLong(const char* key, int32_t dv = 0) { return getInt(key, dv); }

    bool     putULong(const char* key, uint32_t value)  { return putUInt(key, value); }
    uint32_t getULong(const char* key, uint32_t dv = 0) { return getUInt(key, dv); }

    bool    putLong64(const char* key, int64_t value);
    int64_t getLong64(const char* key, int64_t defaultValue = 0) const;

    bool     putULong64(const char* key, uint64_t value);
    uint64_t getULong64(const char* key, uint64_t defaultValue = 0) const;

    bool   putFloat(const char* key, float value);
    float  getFloat(const char* key, float defaultValue = 0.0f) const;

    bool   putDouble(const char* key, double value);
    double getDouble(const char* key, double defaultValue = 0.0) const;

    bool   putString(const char* key, const char* value);
    bool   putString(const char* key, const String& value) { return putString(key, value.c_str()); }
    String getString(const char* key, const char* defaultValue = "") const;
    String getString(const char* key, const String& defaultValue) const {
        return getString(key, defaultValue.c_str());
    }

    size_t putBytes(const char* key, const void* buf, size_t len) { return 0; }
    size_t getBytes(const char* key, void* buf, size_t maxLen) const { return 0; }

    bool remove(const char* key);

private:
    std::string ns_;
    bool readOnly_ = false;
    std::map<std::string, std::string> store_;

    std::string qualifiedKey(const char* key) const;
    std::string prefsFilePath() const;
    void load();
    void save() const;
};
