#include "../../Minecraft.World/Platform/stdafx.h"
#include "DLCManager.h"
#include "DLCLocalisationFile.h"
#include "../../Minecraft.Client/Utils/StringTable.h"

DLCLocalisationFile::DLCLocalisationFile(const std::wstring& path)
    : DLCFile(DLCManager::e_DLCType_LocalisationData, path) {
    m_strings = NULL;
}

void DLCLocalisationFile::addData(std::uint8_t* pbData,
                                  std::uint32_t dataBytes) {
    m_strings = new StringTable(pbData, dataBytes);
}
