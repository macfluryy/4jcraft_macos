#include "../../Platform/stdafx.h"
#include "../../Headers/net.minecraft.world.entity.player.h"
#include "../../Headers/net.minecraft.world.level.h"
#include "../../Headers/net.minecraft.world.level.chunk.storage.h"
#include "../../Headers/net.minecraft.world.level.dimension.h"
#include "MockedLevelStorage.h"

#include "../../IO/Files/ConsoleSaveFileIO.h"

LevelData* MockedLevelStorage::prepareLevel() { return nullptr; }

void MockedLevelStorage::checkSession() {}

ChunkStorage* MockedLevelStorage::createChunkStorage(Dimension* dimension) {
    return nullptr;
}

void MockedLevelStorage::saveLevelData(
    LevelData* levelData, std::vector<std::shared_ptr<Player> >* players) {}

void MockedLevelStorage::saveLevelData(LevelData* levelData) {}

PlayerIO* MockedLevelStorage::getPlayerIO() { return nullptr; }

void MockedLevelStorage::closeAll() {}

ConsoleSavePath MockedLevelStorage::getDataFile(const std::wstring& id) {
    return ConsoleSavePath(std::wstring(L""));
}

std::wstring MockedLevelStorage::getLevelId() { return L"none"; }