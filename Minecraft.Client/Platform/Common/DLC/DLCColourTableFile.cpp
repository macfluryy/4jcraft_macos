#include "../../Minecraft.World/Platform/stdafx.h"
#include "DLCManager.h"
#include "DLCColourTableFile.h"
#include "../../Minecraft.Client/Minecraft.h"
#include "../../Minecraft.Client/Textures/Packs/TexturePackRepository.h"
#include "../../Minecraft.Client/Textures/Packs/TexturePack.h"

DLCColourTableFile::DLCColourTableFile(const std::wstring& path)
    : DLCFile(DLCManager::e_DLCType_ColourTable, path) {
    m_colourTable = NULL;
}

DLCColourTableFile::~DLCColourTableFile() {
    if (m_colourTable != NULL) {
        app.DebugPrintf("Deleting DLCColourTableFile data\n");
        delete m_colourTable;
    }
}

void DLCColourTableFile::addData(std::uint8_t* pbData,
                                 std::uint32_t dataBytes) {
    ColourTable* defaultColourTable =
        Minecraft::GetInstance()->skins->getDefault()->getColourTable();
    m_colourTable = new ColourTable(defaultColourTable, pbData, dataBytes);
}
