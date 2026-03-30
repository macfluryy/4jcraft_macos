#include "Minecraft.World/Header Files/stdafx.h"
#include "DLCManager.h"
#include "DLCTextureFile.h"

DLCTextureFile::DLCTextureFile(const std::wstring& path)
    : DLCFile(DLCManager::e_DLCType_Texture, path) {
    m_bIsAnim = false;
    m_animString = L"";

    m_pbData = nullptr;
    m_dataBytes = 0;
}

void DLCTextureFile::addData(std::uint8_t* pbData, std::uint32_t dataBytes) {
    // app.AddMemoryTextureFile(m_path,pbData,dwBytes);
    m_pbData = pbData;
    m_dataBytes = dataBytes;
}

std::uint8_t* DLCTextureFile::getData(std::uint32_t& dataBytes) {
    dataBytes = m_dataBytes;
    return m_pbData;
}

void DLCTextureFile::addParameter(DLCManager::EDLCParameterType type,
                                  const std::wstring& value) {
    switch (type) {
        case DLCManager::e_DLCParamType_Anim:
            m_animString = value;
            m_bIsAnim = true;

            break;
        default:
            break;
    }
}

std::wstring DLCTextureFile::getParameterAsString(
    DLCManager::EDLCParameterType type) {
    switch (type) {
        case DLCManager::e_DLCParamType_Anim:
            return m_animString;
        default:
            return L"";
    }
}

bool DLCTextureFile::getParameterAsBool(DLCManager::EDLCParameterType type) {
    switch (type) {
        case DLCManager::e_DLCParamType_Anim:
            return m_bIsAnim;
        default:
            return false;
    }
}
