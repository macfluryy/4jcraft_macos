#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#define LOCALE_COUNT 11

class StringTable {
private:
    bool isStatic;

    std::unordered_map<std::wstring, std::wstring> m_stringsMap;
    std::vector<std::wstring> m_stringsVec;

    std::vector<uint8_t> src;

public:
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    

    StringTable(void);
    StringTable(std::uint8_t* pbData, unsigned int dataSize);
    ~StringTable(void);
    void ReloadStringTable();

    void getData(std::uint8_t** ppData, unsigned int* pSize);

    const wchar_t* getString(const std::wstring& id);
    const wchar_t* getString(int id);

    

private:
    
    void ProcessStringTableData(void);
};
