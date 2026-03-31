#include "LevelStorageProfilerDecorator.h"
#include "Minecraft.World/ConsoleHelpers/ConsoleSaveFileIO/ConsoleSavePath.h"
#include "Minecraft.World/net/minecraft/world/level/chunk/storage/ChunkStorageProfileDecorator.h"
#include "Minecraft.World/net/minecraft/world/level/storage/LevelStorage.h"

LevelStorageProfilerDecorator::LevelStorageProfilerDecorator(
    LevelStorage* capsulated)
    : capsulated(capsulated) {}

LevelData* LevelStorageProfilerDecorator::prepareLevel() {
    return capsulated->prepareLevel();
}

void LevelStorageProfilerDecorator::checkSession()  // throws
                                                    // LevelConflictException
{
    capsulated->checkSession();
}

ChunkStorage* LevelStorageProfilerDecorator::createChunkStorage(
    Dimension* dimension) {
    return new ChunkStorageProfilerDecorator(
        capsulated->createChunkStorage(dimension));
}

void LevelStorageProfilerDecorator::saveLevelData(
    LevelData* levelData, std::vector<std::shared_ptr<Player> >* players) {
    capsulated->saveLevelData(levelData, players);
}

void LevelStorageProfilerDecorator::saveLevelData(LevelData* levelData) {
    capsulated->saveLevelData(levelData);
}

PlayerIO* LevelStorageProfilerDecorator::getPlayerIO() {
    return capsulated->getPlayerIO();
}

void LevelStorageProfilerDecorator::closeAll() { capsulated->closeAll(); }

ConsoleSavePath LevelStorageProfilerDecorator::getDataFile(
    const std::wstring& id) {
    return capsulated->getDataFile(id);
}

std::wstring LevelStorageProfilerDecorator::getLevelId() {
    return capsulated->getLevelId();
}