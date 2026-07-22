#pragma once

#include "minecraft/world/level/biome/Biome.h"
class ProgressListener;
class TilePos;
class LevelChunk;


#ifdef _LARGE_WORLDS

#define LEVEL_MAX_WIDTH (5 * 64)  

#define LEVEL_WIDTH_CLASSIC 54
#define LEVEL_WIDTH_SMALL 64
#define LEVEL_WIDTH_MEDIUM (3 * 64)
#define LEVEL_WIDTH_LARGE (5 * 64)

#else
#define LEVEL_MAX_WIDTH 54
#endif
#define LEVEL_MIN_WIDTH 54
#define LEVEL_LEGACY_WIDTH 54




#ifdef _LARGE_WORLDS
#define HELL_LEVEL_MAX_SCALE 8

#define HELL_LEVEL_SCALE_CLASSIC 3
#define HELL_LEVEL_SCALE_SMALL 3
#define HELL_LEVEL_SCALE_MEDIUM 6
#define HELL_LEVEL_SCALE_LARGE 8

#else
#define HELL_LEVEL_MAX_SCALE 3
#endif
#define HELL_LEVEL_MIN_SCALE 3
#define HELL_LEVEL_LEGACY_SCALE 3

#define HELL_LEVEL_MAX_WIDTH (LEVEL_MAX_WIDTH / HELL_LEVEL_MAX_SCALE)
#define HELL_LEVEL_MIN_WIDTH 18

#define END_LEVEL_SCALE 3


#define END_LEVEL_MAX_WIDTH 18
#define END_LEVEL_MIN_WIDTH 18


class ChunkSource {
public:
    
    int m_XZSize;
#ifdef _LARGE_WORLDS
    bool m_classicEdgeMoat;
    bool m_smallEdgeMoat;
    bool m_mediumEdgeMoat;
#endif

public:
    virtual ~ChunkSource() {}

    virtual bool hasChunk(int x, int y) = 0;
    virtual bool reallyHasChunk(int x, int y) {
        return hasChunk(x, y);
    }  
    virtual LevelChunk* getChunk(int x, int z) = 0;
    virtual void lightChunk(LevelChunk* lc) {}  
    virtual LevelChunk* create(int x, int z) = 0;
    virtual void postProcess(ChunkSource* parent, int x, int z) = 0;
    virtual bool saveAllEntities() { return false; }  
    virtual bool save(bool force, ProgressListener* progressListener) = 0;
    virtual bool tick() = 0;
    virtual bool shouldSave() = 0;

    virtual LevelChunk** getCache() { return nullptr; }  
    virtual void dataReceived(int x, int z) {}           

    


    virtual std::wstring gatherStats() = 0;

    virtual std::vector<Biome::MobSpawnerData*>* getMobsAt(
        MobCategory* mobCategory, int x, int y, int z) = 0;
    virtual TilePos* findNearestMapFeature(Level* level,
                                           const std::wstring& featureName,
                                           int x, int y, int z) = 0;

    



    virtual void recreateLogicStructuresForChunk(int chunkX, int chunkZ) = 0;

    
};
