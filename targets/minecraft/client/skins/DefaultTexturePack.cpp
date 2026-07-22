
#include "DefaultTexturePack.h"

#include <cstdint>
#include <vector>

#include "java/InputOutputStream/InputStream.h"
#include "minecraft/client/skins/AbstractTexturePack.h"

DefaultTexturePack::DefaultTexturePack()
    : AbstractTexturePack(0, nullptr, L"Minecraft", nullptr) {
    
    loadIcon();
    loadName();  
    loadDescription();
    loadColourTable();
}

void DefaultTexturePack::loadIcon() {
    if (app.hasArchiveFile(L"Graphics\\TexturePackIcon.png")) {
        std::vector<uint8_t> ba =
            app.getArchiveFile(L"Graphics\\TexturePackIcon.png");
        m_iconData = ba.data();
        m_iconSize = static_cast<std::uint32_t>(ba.size());
    }
}

void DefaultTexturePack::loadDescription() {
    desc1 = L"LOCALISE ME: The default look of Minecraft";
}
void DefaultTexturePack::loadName() { texname = L"Minecraft"; }

bool DefaultTexturePack::hasFile(const std::wstring& name) {
    
    return true;
}

bool DefaultTexturePack::isTerrainUpdateCompatible() { return true; }

InputStream* DefaultTexturePack::getResourceImplementation(
    const std::wstring& name)  
{
    std::wstring wDrive = L"";
    
    wDrive = L"Common\\res\\TitleUpdate\\res";

    InputStream* resource = InputStream::getResourceAsStream(wDrive + name);
    
    
    
    
    
    

    
    return resource;
}

void DefaultTexturePack::loadUI() {
    loadDefaultUI();

    AbstractTexturePack::loadUI();
}

void DefaultTexturePack::unloadUI() { AbstractTexturePack::unloadUI(); }
