#pragma once

#include "LevelStorage.h"
#include "PlayerIO.h"




#include "minecraft/world/level/storage/ConsoleSaveFileIO/ConsoleSaveFile.h"
#include "nbt/com.mojang.nbt.h"

class MemoryLevelStorage : public LevelStorage, public PlayerIO {
public:
    MemoryLevelStorage();
    virtual LevelData* prepareLevel();
    virtual void checkSession();
    virtual ChunkStorage* createChunkStorage(Dimension* dimension);
    virtual void saveLevelData(LevelData* levelData,
                               std::vector<std::shared_ptr<Player> >* players);
    virtual void saveLevelData(LevelData* levelData);
    virtual PlayerIO* getPlayerIO();
    virtual void closeAll();
    virtual void save(std::shared_ptr<Player> player);
    virtual bool load(std::shared_ptr<Player> player);
    virtual CompoundTag* loadPlayerDataTag(const std::wstring& playerName);
    virtual ConsoleSavePath getDataFile(const std::wstring& id);
};