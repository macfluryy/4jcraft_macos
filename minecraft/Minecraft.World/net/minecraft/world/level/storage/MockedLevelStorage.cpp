#include "MockedLevelStorage.h"
#include "Minecraft.World/ConsoleHelpers/ConsoleSaveFileIO/ConsoleSavePath.h"

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