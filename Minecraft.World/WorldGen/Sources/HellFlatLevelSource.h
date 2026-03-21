#pragma once
#include "../../Level/ChunkSource.h"
#include "../Noise/PerlinNoise.h"
#include "../Features/LargeFeature.h"
#include "../Features/LargeHellCaveFeature.h"
#include "../../Level/LevelChunk.h"
#include "../Features/HellFireFeature.h"
#include "../Features/LightGemFeature.h"
#include "../Features/HellPortalFeature.h"
#include "../Features/FlowerFeature.h"
#include "../Features/HellSpringFeature.h"

class ProgressListener;

class HellFlatLevelSource : public ChunkSource {
public:
    static const int CHUNK_HEIGHT = 8;
    static const int CHUNK_WIDTH = 4;

private:
    Random* random;
    Random* pprandom;

private:
    Level* level;

public:
    HellFlatLevelSource(Level* level, int64_t seed);
    ~HellFlatLevelSource();

private:
    void prepareHeights(int xOffs, int zOffs, byteArray blocks);

public:
    void buildSurfaces(int xOffs, int zOffs, byteArray blocks);

    LevelChunk* create(int x, int z);
    LevelChunk* getChunk(int xOffs, int zOffs);
    virtual void lightChunk(LevelChunk* lc);  // 4J added

public:
    virtual bool hasChunk(int x, int y);
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
};
