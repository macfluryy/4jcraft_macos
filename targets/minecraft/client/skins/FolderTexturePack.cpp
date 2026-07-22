#include "FolderTexturePack.h"

#include "java/File.h"
#include "java/InputOutputStream/InputStream.h"
#include "minecraft/client/skins/AbstractTexturePack.h"

class TexturePack;

FolderTexturePack::FolderTexturePack(std::uint32_t id, const std::wstring& name,
                                     File* folder, TexturePack* fallback)
    : AbstractTexturePack(id, folder, name, fallback) {
    
    loadIcon();
    loadName();
    loadDescription();

    bUILoaded = false;
}

InputStream* FolderTexturePack::getResourceImplementation(
    const std::wstring& name)  
{
    std::wstring wDrive = L"";
    
    wDrive = L"Common\\DummyTexturePack\\res";
    InputStream* resource = InputStream::getResourceAsStream(wDrive + name);
    
    
    
    
    
    

    
    return resource;
}

bool FolderTexturePack::hasFile(const std::wstring& name) {
    File file = File(getPath() + name);
    return file.exists() && file.isFile();
    
}

bool FolderTexturePack::isTerrainUpdateCompatible() { return true; }

std::wstring FolderTexturePack::getPath(bool bTitleUpdateTexture ,
                                        const char* pchBDPatchFilename) {
    std::wstring wDrive;
    wDrive = L"Common\\" + file->getPath() + L"\\";
    return wDrive;
}

void FolderTexturePack::loadUI() {}

void FolderTexturePack::unloadUI() {}