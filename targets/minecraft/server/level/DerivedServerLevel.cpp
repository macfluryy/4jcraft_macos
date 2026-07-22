#include "DerivedServerLevel.h"

#include "minecraft/server/level/ServerLevel.h"
#include "minecraft/world/level/storage/DerivedLevelData.h"
#include "minecraft/world/level/storage/SavedDataStorage.h"

class LevelSettings;
class LevelStorage;
class MinecraftServer;

DerivedServerLevel::DerivedServerLevel(
    MinecraftServer* server, std::shared_ptr<LevelStorage> levelStorage,
    const std::wstring& levelName, int dimension, LevelSettings* levelSettings,
    ServerLevel* wrapped)
    : ServerLevel(server, levelStorage, levelName, dimension, levelSettings) {
    
    
    if (this->savedDataStorage) {
        delete this->savedDataStorage;
        this->savedDataStorage = nullptr;
    }
    this->savedDataStorage = wrapped->savedDataStorage;
    levelData = new DerivedLevelData(wrapped->getLevelData());
}

DerivedServerLevel::~DerivedServerLevel() {
    
    
    this->savedDataStorage = nullptr;
}

void DerivedServerLevel::saveLevelData() {
    
    
}