#pragma once
#include <cstdint>
#include <string>

#include "DLCFile.h"

class StringTable;

class DLCLocalisationFile : public DLCFile {
private:
    StringTable* m_strings;

public:
    DLCLocalisationFile(const std::wstring& path);
    DLCLocalisationFile(
        std::uint8_t* pbData,
        std::uint32_t dataBytes);  
                                   

    virtual void addData(std::uint8_t* pbData, std::uint32_t dataBytes);

    StringTable* getStringTable() { return m_strings; }
};
