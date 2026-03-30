#include "../../../../../Header Files/stdafx.h"
#include "../../entity/player/net.minecraft.world.entity.player.h"
#include "../net.minecraft.world.level.h"
#include "../chunk/storage/net.minecraft.world.level.chunk.storage.h"
#include "../dimension/net.minecraft.world.level.dimension.h"
#include "MockedLevelStorage.h"

#include "../../../../../ConsoleHelpers/ConsoleSaveFileIO/ConsoleSaveFileIO.h"

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