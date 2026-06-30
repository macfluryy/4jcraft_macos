#pragma once

#include <stdint.h>

#include <format>
#include <string>
#include <vector>

#include "minecraft/world/level/biome/Biome.h"
#include "minecraft/world/level/chunk/ChunkSource.h"

class ProgressListener;
class LargeFeature;
class StrongholdFeature;
class VillageFeature;
class MineShaftFeature;
class PerlinNoise;
class RandomScatteredLargeFeature;
class Level;
class Random;

class RandomLevelSource : public ChunkSource {
public:
    static const double SNOW_CUTOFF;
    static const double SNOW_SCALE;
    static const bool FLOATING_ISLANDS = false;
    static const int CHUNK_HEIGHT = 8;
    static const int CHUNK_WIDTH = 4;

private:
    Random* random;
    Random* pprandom;  // 4J - added

    PerlinNoise* lperlinNoise1;
    PerlinNoise* lperlinNoise2;
    PerlinNoise* perlinNoise1;
    PerlinNoise* perlinNoise3;

public:
    PerlinNoise* scaleNoise;
    PerlinNoise* depthNoise;

private:
    PerlinNoise* floatingIslandScale;
    PerlinNoise* floatingIslandNoise;

    // 4J macOS - Phase 2 continental noise. Very low frequency (~660
    // block period) Perlin sample used in amplified mode to bias the
    // density field toward connected land masses. Without it the noise
    // sampler produces lots of small isolated pillars; with it, terrain
    // reads as "connected mountain range" instead of "scattered cliffs".
    PerlinNoise* continentNoise;

    // 4J macOS - Phase 3 mountain mask. Mid-frequency Perlin used as a
    // ridge-fold mask to push neighbouring chunks of biomeScale up
    // along the same line, producing TerraForged-style mountain chains
    // instead of isolated peaks. Only sampled in amplified mode and
    // only used where biomeDepth > 0 (so oceans aren't affected).
    PerlinNoise* mountainMaskNoise;

public:
    PerlinNoise* forestNoise;

private:
    Level* level;
    const bool generateStructures;
    bool amplified;

    std::vector<float> pows;

public:
    RandomLevelSource(Level* level, int64_t seed, bool generateStructures);
    ~RandomLevelSource();

public:
#ifdef _LARGE_WORLDS
    int getMinDistanceToEdge(int xxx, int zzz, int worldSize,
                             float falloffStart);

#endif
    float getHeightFalloff(int xxx, int zzz, int* pEMin);
    void prepareHeights(int xOffs, int zOffs, std::vector<uint8_t>& blocks);

public:
    void buildSurfaces(int xOffs, int zOffs, std::vector<uint8_t>& blocks,
                       std::vector<Biome*>& biomes);

private:
    void addAmplifiedFloatingIslands(int xOffs, int zOffs,
                                     std::vector<uint8_t>& blocks,
                                     std::vector<Biome*>& biomes);
    void addAmplifiedMountainResources(int xt, int zt);
    void addAmplifiedHighWaterfalls(int xt, int zt);
    void addAmplifiedCliffCaves(int xt, int zt);

private:
    LargeFeature* caveFeature;
    StrongholdFeature* strongholdFeature;
    VillageFeature* villageFeature;
    MineShaftFeature* mineShaftFeature;
    RandomScatteredLargeFeature* scatteredFeature;
    LargeFeature* canyonFeature;

private:
    virtual LevelChunk* create(int x, int z);

public:
    virtual LevelChunk* getChunk(int xOffs, int zOffs);
    virtual void lightChunk(LevelChunk* lc);  // 4J added

private:
    std::vector<double> getHeights(std::vector<double>& buffer, int x, int y,
                                   int z, int xSize, int ySize, int zSize,
                                   std::vector<Biome*>& biomes);

public:
    virtual bool hasChunk(int x, int y);

private:
    void calcWaterDepths(ChunkSource* parent, int xt, int zt);

public:
    virtual void postProcess(ChunkSource* parent, int xt, int zt);
    virtual bool save(bool force, ProgressListener* progressListener);
    virtual bool tick();
    virtual bool shouldSave();
    virtual std::wstring gatherStats();

public:
    virtual std::vector<Biome::MobSpawnerData*>* getMobsAt(
        MobCategory* mobCategory, int x, int y, int z);
    virtual TilePos* findNearestMapFeature(Level* level,
                                           const std::wstring& featureName,
                                           int x, int y, int z);
    virtual void recreateLogicStructuresForChunk(int chunkX, int chunkZ);
};
