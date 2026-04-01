#pragma once
#include <string>
#include <unordered_map>

class File;

class Settings {
    //    public static Logger logger = Logger.getLogger("Minecraft");
    //    private Properties properties = new Properties();
private:
    std::unordered_map<std::wstring, std::wstring>
        properties;  // 4J - TODO was Properties type, will need to implement
                     // something we can serialise/deserialise too
                     // File *file;

public:
    Settings(File* file);
    void generateNewProperties();
    void saveProperties();
    std::wstring getString(const std::wstring& key,
                           const std::wstring& defaultValue);
    int getInt(const std::wstring& key, int defaultValue);
    bool getBoolean(const std::wstring& key, bool defaultValue);
    void setBooleanAndSave(const std::wstring& key, bool value);
};
