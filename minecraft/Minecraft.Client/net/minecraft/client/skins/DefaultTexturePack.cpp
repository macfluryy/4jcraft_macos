#include "Minecraft.World/Header Files/stdafx.h"
#include "DefaultTexturePack.h"
#include "../renderer/Textures.h"
#include "Minecraft.World/ConsoleHelpers/StringHelpers.h"

DefaultTexturePack::DefaultTexturePack()
    : AbstractTexturePack(0, nullptr, L"Minecraft", nullptr) {
    // 4J Stu - These calls need to be in the most derived version of the class
    loadIcon();
    loadName();  // 4J-PB - added so the PS3 can have localised texture names'
    loadDescription();
    loadColourTable();
}

void DefaultTexturePack::loadIcon() {
    if (app.hasArchiveFile(L"Graphics\\TexturePackIcon.png")) {
        byteArray ba = app.getArchiveFile(L"Graphics\\TexturePackIcon.png");
        m_iconData = ba.data;
        m_iconSize = static_cast<std::uint32_t>(ba.length);
    }
}

void DefaultTexturePack::loadDescription() {
    desc1 = L"LOCALISE ME: The default look of Minecraft";
}
void DefaultTexturePack::loadName() { texname = L"Minecraft"; }

bool DefaultTexturePack::hasFile(const std::wstring& name) {
    //	return DefaultTexturePack::class->getResourceAsStream(name) != null;
    return true;
}

bool DefaultTexturePack::isTerrainUpdateCompatible() { return true; }

InputStream* DefaultTexturePack::getResourceImplementation(
    const std::wstring& name)  // throws FileNotFoundException
{
    std::wstring wDrive = L"";
    // Make the content package point to to the UPDATE: drive is needed
    wDrive = L"Common\\res\\TitleUpdate\\res";

    InputStream* resource = InputStream::getResourceAsStream(wDrive + name);
    // InputStream *stream =
    // DefaultTexturePack::class->getResourceAsStream(name); if (stream ==
    // nullptr)
    //{
    //	throw new FileNotFoundException(name);
    // }

    // return stream;
    return resource;
}

void DefaultTexturePack::loadUI() {
    loadDefaultUI();

    AbstractTexturePack::loadUI();
}

void DefaultTexturePack::unloadUI() { AbstractTexturePack::unloadUI(); }
