#include "../../Build/stdafx.h"
#include "../../Headers/net.minecraft.world.entity.player.h"
#include "../../Headers/net.minecraft.world.level.h"
#include "../../Headers/net.minecraft.world.level.chunk.storage.h"
#include "../../Headers/net.minecraft.world.level.dimension.h"
#include "LevelStorageProfilerDecorator.h"

#include "../../IO/Files/ConsoleSaveFileIO.h"

LevelStorageProfilerDecorator::LevelStorageProfilerDecorator(LevelStorage *capsulated) : capsulated ( capsulated )
{
}

LevelData *LevelStorageProfilerDecorator::prepareLevel()
{
	return capsulated->prepareLevel();
}

void LevelStorageProfilerDecorator::checkSession() // throws LevelConflictException
{
	capsulated->checkSession();
}

ChunkStorage *LevelStorageProfilerDecorator::createChunkStorage(Dimension *dimension)
{
	return new ChunkStorageProfilerDecorator(capsulated->createChunkStorage(dimension));
}

void LevelStorageProfilerDecorator::saveLevelData(LevelData *levelData, vector<std::shared_ptr<Player> > *players)
{
	capsulated->saveLevelData(levelData, players);
}

void LevelStorageProfilerDecorator::saveLevelData(LevelData *levelData)
{
	capsulated->saveLevelData(levelData);
}

PlayerIO *LevelStorageProfilerDecorator::getPlayerIO()
{
	return capsulated->getPlayerIO();
}

void LevelStorageProfilerDecorator::closeAll()
{
	capsulated->closeAll();
}

ConsoleSavePath LevelStorageProfilerDecorator::getDataFile(const wstring& id)
{
	return capsulated->getDataFile(id);
}

wstring LevelStorageProfilerDecorator::getLevelId()
{
	return capsulated->getLevelId();
}