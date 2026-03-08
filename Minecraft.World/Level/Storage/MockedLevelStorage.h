#pragma once


#include "LevelStorage.h"

#include "../../IO/Files/ConsoleSavePath.h"

class MockedLevelStorage : public LevelStorage 
{
public:
	virtual LevelData *prepareLevel();
	virtual void checkSession();
	virtual ChunkStorage *createChunkStorage(Dimension *dimension);
	virtual void saveLevelData(LevelData *levelData, std::vector<std::shared_ptr<Player> > *players);
	virtual void saveLevelData(LevelData *levelData);
	virtual PlayerIO *getPlayerIO();
	virtual void closeAll();
	virtual ConsoleSavePath getDataFile(const std::wstring& id);
	virtual std::wstring getLevelId();
public:
	virtual ConsoleSaveFile *getSaveFile() { return NULL; }
};