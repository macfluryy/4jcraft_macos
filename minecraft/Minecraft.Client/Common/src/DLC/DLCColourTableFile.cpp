#include "DLCManager.h"
#include "DLCColourTableFile.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/skins/TexturePackRepository.h"
#include "minecraft/client/skins/TexturePack.h"
#include "Minecraft.Client/Common/src/Colours/ColourTable.h"
#include "Minecraft.Client/Common/src/DLC/DLCFile.h"
#include "Minecraft.Client/Linux/Linux_App.h"

DLCColourTableFile::DLCColourTableFile(const std::wstring& path)
    : DLCFile(DLCManager::e_DLCType_ColourTable, path) {
    m_colourTable = nullptr;
}

DLCColourTableFile::~DLCColourTableFile() {
    if (m_colourTable != nullptr) {
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
