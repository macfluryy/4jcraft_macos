#include "Settings.h"

#include "util/StringHelpers.h"

// 4J - TODO - serialise/deserialise from file
Settings::Settings(File* file) {}

void Settings::generateNewProperties() {}

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