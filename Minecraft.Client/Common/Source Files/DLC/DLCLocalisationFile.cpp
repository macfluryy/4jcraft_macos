#include "../../../../Minecraft.World/Header Files/stdafx.h"
#include "DLCManager.h"
#include "DLCLocalisationFile.h"
#include "../Localisation/StringTable.h"

DLCLocalisationFile::DLCLocalisationFile(const std::wstring& path)
    : DLCFile(DLCManager::e_DLCType_LocalisationData, path) {
    m_strings = nullptr;
}

void DLCLocalisationFile::addData(std::uint8_t* pbData,
                                  std::uint32_t dataBytes) {
    m_strings = new StringTable(pbData, dataBytes);
}
