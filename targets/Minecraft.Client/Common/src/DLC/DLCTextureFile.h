#pragma once
#include <cstdint>
#include <string>

#include "DLCFile.h"
#include "Minecraft.Client/Common/src/DLC/DLCManager.h"

class DLCTextureFile : public DLCFile {
private:
    bool m_bIsAnim;
    std::wstring m_animString;

    std::uint8_t* m_pbData;
    std::uint32_t m_dataBytes;

public:
    DLCTextureFile(const std::wstring& path);

    virtual void addData(std::uint8_t* pbData, std::uint32_t dataBytes);
    virtual std::uint8_t* getData(std::uint32_t& dataBytes);

    virtual void addParameter(DLCManager::EDLCParameterType type,
                              const std::wstring& value);

    virtual std::wstring getParameterAsString(
        DLCManager::EDLCParameterType type);
    virtual bool getParameterAsBool(DLCManager::EDLCParameterType type);
};
