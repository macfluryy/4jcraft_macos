#include "DLCLocalisationFile.h"

#include "DLCManager.h"
#include "Minecraft.Client/Common/src/Localisation/StringTable.h"
#include "Minecraft.Client/Common/src/DLC/DLCFile.h"

DLCLocalisationFile::DLCLocalisationFile(const std::wstring& path)
    : DLCFile(DLCManager::e_DLCType_LocalisationData, path) {
    m_strings = nullptr;
}

void DLCLocalisationFile::addData(std::uint8_t* pbData,
                                  std::uint32_t dataBytes) {
    m_strings = new StringTable(pbData, dataBytes);
}
