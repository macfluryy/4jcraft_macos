#include "DLCCapeFile.h"

#include "DLCManager.h"
#include "Minecraft.Client/Common/src/DLC/DLCFile.h"
#include "Minecraft.Client/Linux/Linux_App.h"

DLCCapeFile::DLCCapeFile(const std::wstring& path)
    : DLCFile(DLCManager::e_DLCType_Cape, path) {}

void DLCCapeFile::addData(std::uint8_t* pbData, std::uint32_t dataBytes) {
    app.AddMemoryTextureFile(m_path, pbData, dataBytes);
}
