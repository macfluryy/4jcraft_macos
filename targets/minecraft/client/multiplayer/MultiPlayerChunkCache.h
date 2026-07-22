#pragma once
#include <mutex>
#include <string>
#include <vector>

#include "minecraft/world/level/biome/Biome.h"
#include "minecraft/world/level/chunk/ChunkSource.h"

#include "minecraft/world/level/levelgen/RandomLevelSource.h"


class ServerChunkCache;
class Level;
class LevelChunk;



class MultiPlayerChunkCache : public ChunkSource {
    friend class LevelRenderer;

private:
    LevelChunk* emptyChunk;
    LevelChunk* waterChunk;

    std::vector<LevelChunk*> loadedChunkList;

    LevelChunk** cache;
    
    std::mutex m_csLoadCreate;
    
    int XZSIZE;
    int XZOFFSET;
    bool* hasData;

    Level* level;

public:
    MultiPlayerChunkCache(Level* level);
    ~MultiPlayerChunkCache();
    virtual bool hasChunk(int x, int z);
    virtual bool reallyHasChunk(int x, int z);
    virtual void drop(int x, int z);
    virtual LevelChunk* create(int x, int z);
    virtual LevelChunk* getChunk(int x, int z);
    virtual bool save(bool force, ProgressListener* progressListener);
    virtual bool tick();
    virtual bool shouldSave();
    virtual void postProcess(ChunkSource* parent, int x, int z);
    virtual std::wstring gatherStats();
    virtual std::vector<Biome::MobSpawnerData*>* getMobsAt(
        MobCategory* mobCategory, int x, int y, int z);
    virtual TilePos* findNearestMapFeature(Level* level,
                                           const std::wstring& featureName,
                                           int x, int y, int z);
    virtual void recreateLogicStructuresForChunk(int chunkX, int chunkZ);
    virtual void dataReceived(int x, int z);  

    virtual LevelChunk** getCache() { return cache; }  
};