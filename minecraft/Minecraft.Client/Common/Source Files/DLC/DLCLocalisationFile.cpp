#include "DLCManager.h"
#include "DLCLocalisationFile.h"
#include "Minecraft.Client/Common/Source Files/Localisation/StringTable.h"
#include "Minecraft.Client/Common/Source Files/DLC/DLCFile.h"

DLCLocalisationFile::DLCLocalisationFile(const std::wstring& path)
    : DLCFile(DLCManager::e_DLCType_LocalisationData, path) {
    m_strings = nullptr;
}

void DLCLocalisationFile::addData(std::uint8_t* pbData,
                                  std::uint32_t dataBytes) {
    m_strings = new StringTable(pbData, dataBytes);
}
