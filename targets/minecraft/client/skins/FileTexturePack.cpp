#include "FileTexturePack.h"

#include "java/File.h"
#include "minecraft/client/skins/AbstractTexturePack.h"

class TexturePack;

FileTexturePack::FileTexturePack(std::uint32_t id, File* file,
                                 TexturePack* fallback)
    : AbstractTexturePack(id, file, file->getName(), fallback) {
    
    loadIcon();
    loadName();
    loadDescription();
}

void FileTexturePack::unload(Textures* textures) {}

InputStream* FileTexturePack::getResourceImplementation(
    const std::wstring& name)  
{
    return nullptr;
}

bool FileTexturePack::hasFile(const std::wstring& name) { return false; }

void FileTexturePack::loadZipFile()  
{}

bool FileTexturePack::isTerrainUpdateCompatible() { return false; }
