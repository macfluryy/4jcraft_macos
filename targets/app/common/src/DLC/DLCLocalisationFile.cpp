#include "DLCLocalisationFile.h"

#include "DLCManager.h"
#include "app/common/src/DLC/DLCFile.h"
#include "app/common/src/Localisation/StringTable.h"

DLCLocalisationFile::DLCLocalisationFile(const std::wstring& path)
    : DLCFile(DLCManager::e_DLCType_LocalisationData, path) {
    m_strings = nullptr;
}

void DLCLocalisationFile::addData(std::uint8_t* pbData,
                                  std::uint32_t dataBytes) {
    m_strings = new StringTable(pbData, dataBytes);
}
