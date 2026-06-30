#include "Settings.h"

#include "util/StringHelpers.h"

// 4J - TODO - serialise/deserialise from file
Settings::Settings(File* file) {}

void Settings::generateNewProperties() {}

// 4J macOS - Settings has no on-disk backing yet (stub). The in-memory
// `properties` map IS the persisted store for this build: getInt/getString/
// getBoolean lazily insert the default for a missing key and call
// saveProperties(), and setIntAndSave/setBooleanAndSave overwrite a key in
// place. Real server.properties (de)serialisation from `file` remains a TODO;
// until then saveProperties() has nothing to flush, so it is a no-op. Anything
// written into `properties` (e.g. the "view-distance" key, Req 2.4) is
// immediately readable back via getInt within the process lifetime.
void Settings::saveProperties() {}

std::wstring Settings::getString(const std::wstring& key,
                                 const std::wstring& defaultValue) {
    if (properties.find(key) == properties.end()) {
        properties[key] = defaultValue;
        saveProperties();
    }
    return properties[key];
}

int Settings::getInt(const std::wstring& key, int defaultValue) {
    if (properties.find(key) == properties.end()) {
        properties[key] = toWString<int>(defaultValue);
        saveProperties();
    }
    return fromWString<int>(properties[key]);
}

bool Settings::getBoolean(const std::wstring& key, bool defaultValue) {
    if (properties.find(key) == properties.end()) {
        properties[key] = toWString<bool>(defaultValue);
        saveProperties();
    }
    bool retval = fromWString<bool>(properties[key]);
    return retval;
}

void Settings::setBooleanAndSave(const std::wstring& key, bool value) {
    properties[key] = toWString<bool>(value);
    saveProperties();
}

// 4J macOS - persist an int under key (Req 2.4 for the "view-distance" key).
// Overwrites any existing/lazily-defaulted entry so the stored value is the
// authoritative one. Because the in-memory `properties` map is the store, a
// subsequent getInt(key, anyDefault) returns this value rather than its
// default - i.e. the value round-trips. Uses the same toWString<int> encoding
// that getInt's fromWString<int> reads back.
void Settings::setIntAndSave(const std::wstring& key, int value) {
    properties[key] = toWString<int>(value);
    saveProperties();
}