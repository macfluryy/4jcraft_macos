#pragma once
#include <string>
#include <unordered_map>

class File;

class Settings {
    
    
private:
    std::unordered_map<std::wstring, std::wstring>
        properties;  
                     
                     

public:
    Settings(File* file);
    void generateNewProperties();
    void saveProperties();
    std::wstring getString(const std::wstring& key,
                           const std::wstring& defaultValue);
    int getInt(const std::wstring& key, int defaultValue);
    bool getBoolean(const std::wstring& key, bool defaultValue);
    void setBooleanAndSave(const std::wstring& key, bool value);
    
    
    
    void setIntAndSave(const std::wstring& key, int value);
};
