#pragma once
#include "../chunk/ChunkSource.h"
#include "synth/PerlinNoise.h"
#include "LargeFeature.h"
#include "LargeHellCaveFeature.h"
#include "../chunk/LevelChunk.h"
#include "feature/HellFireFeature.h"
#include "feature/LightGemFeature.h"
#include "feature/HellPortalFeature.h"
#include "feature/FlowerFeature.h"
#include "feature/HellSpringFeature.h"
#include "structure/NetherBridgeFeature.h"

class ProgressListener;

class HellRandomLevelSource : public ChunkSource {
public:
    static const int CHUNK_HEIGHT = 8;
    static const int CHUNK_WIDTH = 4;

private:
    Random* random;
    Random* pprandom;  // 4J added

    PerlinNoise* lperlinNoise1;
    PerlinNoise* lperlinNoise2;
    PerlinNoise* perlinNoise1;
    PerlinNoise* perlinNoise2;
    PerlinNoise* perlinNoise3;

public:
    PerlinNoise* scaleNoise;
    PerlinNoise* depthNoise;

private:
    Level* level;

public:
    HellRandomLevelSource(Level* level, int64_t seed);
    ~HellRandomLevelSource();

    NetherBridgeFeature* netherBridgeFeature;

private:
    void prepareHeights(int xOffs, int zOffs, byteArray blocks);

public:
    void buildSurfaces(int xOffs, int zOffs, byteArray blocks);

private:
    LargeFeature* caveFeature;

public:
    LevelChunk* create(int x, int z);
    LevelChunk* getChunk(int xOffs, int zOffs);
    virtual void lightChunk(LevelChunk* lc);  // 4J added

private:
    doubleArray getHeights(doubleArray buffer, int x, int y, int z, int xSize,
                           int ySize, int zSize);

public:
    bool hasChunk(int x, int y);
    void postProcess(ChunkSource* parent, int xt, int zt);
    bool save(bool force, ProgressListener* progressListener);
    bool tick();
    bool shouldSave();
    std::wstring gatherStats();

    virtual std::vector<Biome::MobSpawnerData*>* getMobsAt(
        MobCategory* mobCategory, int x, int y, int z);
    virtual TilePos* findNearestMapFeature(Level* level,
                                           const std::wstring& featureName,
                                           int x, int y, int z);
    virtual void recreateLogicStructuresForChunk(int chunkX, int chunkZ);
};
