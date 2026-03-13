#include "../../Platform/stdafx.h"
#include "../../Headers/net.minecraft.world.entity.player.h"
#include "../../Headers/net.minecraft.world.level.h"
#include "../../Headers/net.minecraft.world.level.chunk.storage.h"
#include "../../Headers/net.minecraft.world.level.dimension.h"
#include "../../Headers/com.mojang.nbt.h"
#include "MemoryLevelStorage.h"

#include "../../IO/Files/ConsoleSaveFileIO.h"

MemoryLevelStorage::MemoryLevelStorage() {}

LevelData* MemoryLevelStorage::prepareLevel() { return NULL; }

void MemoryLevelStorage::checkSession() {}

ChunkStorage* MemoryLevelStorage::createChunkStorage(Dimension* dimension) {
    return new MemoryChunkStorage();
}

void MemoryLevelStorage::saveLevelData(
    LevelData* levelData, std::vector<std::shared_ptr<Player> >* players) {}

void MemoryLevelStorage::saveLevelData(LevelData* levelData) {}

PlayerIO* MemoryLevelStorage::getPlayerIO() { return this; }

void MemoryLevelStorage::closeAll() {}

void MemoryLevelStorage::save(std::shared_ptr<Player> player) {}

bool MemoryLevelStorage::load(std::shared_ptr<Player> player) { return false; }

CompoundTag* MemoryLevelStorage::loadPlayerDataTag(
    const std::wstring& playerName) {
    return NULL;
}

ConsoleSavePath MemoryLevelStorage::getDataFile(const std::wstring& id) {
    return ConsoleSaveFile(std::wstring(L""));
}