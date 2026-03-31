#pragma once

#include <cstdint>
#include <string>

#include "AbstractTexturePack.h"

class File;
class TexturePack;

class FolderTexturePack : public AbstractTexturePack {
private:
    bool bUILoaded;

public:
    FolderTexturePack(std::uint32_t id, const std::wstring& name, File* folder,
                      TexturePack* fallback);

protected:
    //@Override
    InputStream* getResourceImplementation(
        const std::wstring& name);  // throws IOException

public:
    //@Override
    bool hasFile(const std::wstring& name);
    bool isTerrainUpdateCompatible();

    // 4J Added
    virtual std::wstring getPath(bool bTitleUpdateTexture = false,
                                 const char* pchBDPatchFilename = nullptr);
    virtual void loadUI();
    virtual void unloadUI();
};