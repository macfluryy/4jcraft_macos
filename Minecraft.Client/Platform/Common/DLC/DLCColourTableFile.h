#pragma once
#include "DLCFile.h"

class ColourTable;

class DLCColourTableFile : public DLCFile {
private:
    ColourTable* m_colourTable;

public:
    DLCColourTableFile(const std::wstring& path);
    ~DLCColourTableFile();

    virtual void addData(std::uint8_t* pbData, std::uint32_t dataBytes);

    ColourTable* getColourTable() { return m_colourTable; }
};
