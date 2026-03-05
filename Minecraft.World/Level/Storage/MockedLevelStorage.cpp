#include "../../Build/stdafx.h"
#include "../../Headers/net.minecraft.world.entity.player.h"
#include "../../Headers/net.minecraft.world.level.h"
#include "../../Headers/net.minecraft.world.level.chunk.storage.h"
#include "../../Headers/net.minecraft.world.level.dimension.h"
#include "MockedLevelStorage.h"

#include "../../IO/Files/ConsoleSaveFileIO.h"

LevelData *MockedLevelStorage::prepareLevel() 
{
    return NULL;
}

void MockedLevelStorage::checkSession()
{
}

ChunkStorage *MockedLevelStorage::createChunkStorage(Dimension *dimension)
{
    return NULL;
}

void MockedLevelStorage::saveLevelData(LevelData *levelData, vector<shared_ptr<Player> > *players)
{
}

void MockedLevelStorage::saveLevelData(LevelData *levelData)
{
}

PlayerIO *MockedLevelStorage::getPlayerIO()
{
    return NULL;
}

void MockedLevelStorage::closeAll() 
{
}

ConsoleSavePath MockedLevelStorage::getDataFile(const wstring& id)
{
    return ConsoleSavePath(wstring(L""));
}

wstring MockedLevelStorage::getLevelId()
{
	return L"none";
}