#pragma once
#include <cstdint>
#include <string>

#include "AbstractTexturePack.h"


class BufferedImage;
class File;
class Textures;
class TexturePack;

class FileTexturePack : public AbstractTexturePack {
private:
    

public:
    FileTexturePack(std::uint32_t id, File* file, TexturePack* fallback);

    
    void unload(Textures* textures);

protected:
    InputStream* getResourceImplementation(
        const std::wstring& name);  

public:
    
    bool hasFile(const std::wstring& name);

private:
    void loadZipFile();  

public:
    bool isTerrainUpdateCompatible();
};
