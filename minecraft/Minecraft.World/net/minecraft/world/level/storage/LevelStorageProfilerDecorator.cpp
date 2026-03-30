#include "../../../../../Header Files/stdafx.h"
#include "../../entity/player/net.minecraft.world.entity.player.h"
#include "../net.minecraft.world.level.h"
#include "../chunk/storage/net.minecraft.world.level.chunk.storage.h"
#include "../dimension/net.minecraft.world.level.dimension.h"
#include "LevelStorageProfilerDecorator.h"

#include "../../../../../ConsoleHelpers/ConsoleSaveFileIO/ConsoleSaveFileIO.h"

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