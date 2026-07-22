#pragma once

#include <string>

#include "DirectoryLevelStorage.h"
#include "java/File.h"

class ConsoleSaveFile;

class McRegionLevelStorage : public DirectoryLevelStorage {
    

    friend class McRegionLevelStorageSource;  
                                              

protected:
    static const int MCREGION_VERSION_ID = 0x4abc;

public:
    McRegionLevelStorage(ConsoleSaveFile* saveFile, File dir,
                         const std::wstring& levelName, bool createPlayerDir);
    ~McRegionLevelStorage();

    virtual ChunkStorage* createChunkStorage(Dimension* dimension);
    virtual void saveLevelData(LevelData* levelData,
                               std::vector<std::shared_ptr<Player> >* players);
    virtual void closeAll();
};