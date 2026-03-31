#include "Minecraft.World/Header Files/stdafx.h"
#include "Minecraft.World/net/minecraft/world/level/chunk/storage/net.minecraft.world.level.chunk.storage.h"
#include "MemoryLevelStorage.h"
#include "LevelSummary.h"
#include "MemoryLevelStorageSource.h"

MemoryLevelStorageSource::MemoryLevelStorageSource() {}

std::wstring MemoryLevelStorageSource::getName() { return L"Memory Storage"; }

std::shared_ptr<LevelStorage> MemoryLevelStorageSource::selectLevel(
    const std::wstring& levelId, bool createPlayerDir) {
        return std::shared_ptr<LevelStorage> () new MemoryLevelStorage());
}

std::vector<LevelSummary*>* MemoryLevelStorageSource::getLevelList() {
    return new std::vector<LevelSummary*>;
}

void MemoryLevelStorageSource::clearAll() {}

LevelData* MemoryLevelStorageSource::getDataTagFor(
    const std::wstring& levelId) {
    return nullptr;
}

bool MemoryLevelStorageSource::isNewLevelIdAcceptable(
    const std::wstring& levelId) {
    return true;
}

void MemoryLevelStorageSource::deleteLevel(const std::wstring& levelId) {}

void MemoryLevelStorageSource::renameLevel(const std::wstring& levelId,
                                           const std::wstring& newLevelName) {}

bool MemoryLevelStorageSource::isConvertible(const std::wstring& levelId) {
    return false;
}

bool MemoryLevelStorageSource::requiresConversion(const std::wstring& levelId) {
    return false;
}

bool MemoryLevelStorageSource::convertLevel(const std::wstring& levelId,
                                            ProgressListener* progress) {
    return false;
}