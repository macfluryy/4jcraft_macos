#pragma once
#include <cstdint>
#include "DLCManager.h"

class DLCFile {
protected:
    DLCManager::EDLCType m_type;
    std::wstring m_path;
    std::uint32_t m_dwSkinId;

public:
    DLCFile(DLCManager::EDLCType type, const std::wstring& path);
    virtual ~DLCFile() {}

    DLCManager::EDLCType getType() { return m_type; }
    std::wstring getPath() { return m_path; }
    std::uint32_t getSkinID() { return m_dwSkinId; }

    virtual void addData(std::uint8_t* pbData, std::uint32_t dataBytes) {}
    virtual std::uint8_t* getData(std::uint32_t& dataBytes) {
        dataBytes = 0;
        return nullptr;
    }
    virtual void addParameter(DLCManager::EDLCParameterType type,
                              const std::wstring& value) {}

    virtual std::wstring getParameterAsString(
        DLCManager::EDLCParameterType type) {
        return L"";
    }
    virtual bool getParameterAsBool(DLCManager::EDLCParameterType type) {
        return false;
    }
};
