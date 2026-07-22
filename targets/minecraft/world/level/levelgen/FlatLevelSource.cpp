#include "FlatLevelSource.h"

#include <stdlib.h>
#include <string.h>

#include <vector>

#include "app/mac/MacGame.h"
#include "java/Random.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/biome/Biome.h"
#include "minecraft/world/level/chunk/ChunkSource.h"
#include "minecraft/world/level/chunk/LevelChunk.h"
#include "minecraft/world/level/levelgen/structure/VillageFeature.h"
#include "minecraft/world/level/storage/LevelData.h"
#include "minecraft/world/level/tile/Tile.h"



FlatLevelSource::FlatLevelSource(Level* level, int64_t seed,
                                 bool generateStructures) {
    m_XZSize = level->getLevelData()->getXZSize();

    this->level = level;
    this->generateStructures = generateStructures;
    this->random = new Random(seed);
    this->pprandom = new Random(
        seed);  
                

    villageFeature = new VillageFeature(m_XZSize);
}

FlatLevelSource::~FlatLevelSource() {
    delete random;
    delete pprandom;
    delete villageFeature;
}

void FlatLevelSource::prepareHeights(std::vector<uint8_t>& blocks) {
    int height = blocks.size() / (16 * 16);

    for (int xc = 0; xc < 16; xc++) {
        for (int zc = 0; zc < 16; zc++) {
            for (int yc = 0; yc < height; yc++) {
                int block = 0;
                if (yc == 0) {
                    block = Tile::unbreakable_Id;
                } else if (yc <= 2) {
                    block = Tile::dirt_Id;
                } else if (yc == 3) {
                    block = Tile::grass_Id;
                }
                blocks[xc << 11 | zc << 7 | yc] = (uint8_t)block;
            }
        }
    }
}

LevelChunk* FlatLevelSource::create(int x, int z) { return getChunk(x, z); }

LevelChunk* FlatLevelSource::getChunk(int xOffs, int zOffs) {
    
    
    int chunksSize = Level::genDepth * 16 * 16;
    uint8_t* tileData = (uint8_t*)malloc(chunksSize);
    memset(tileData, 0, chunksSize);
    std::vector<uint8_t> blocks =
        std::vector<uint8_t>(tileData, tileData + chunksSize);
    
    
    prepareHeights(blocks);

    
    
    

    if (generateStructures) {
        villageFeature->apply(this, level, xOffs, zOffs, blocks);
    }

    
    
    
    
    LevelChunk* levelChunk = new LevelChunk(level, blocks, xOffs, zOffs);
    free(tileData);

    levelChunk->recalcHeightmap();

    return levelChunk;
}

bool FlatLevelSource::hasChunk(int x, int y) { return true; }

void FlatLevelSource::postProcess(ChunkSource* parent, int xt, int zt) {
    
    
    pprandom->setSeed(level->getSeed());
    int64_t xScale = pprandom->nextLong() / 2 * 2 + 1;
    int64_t zScale = pprandom->nextLong() / 2 * 2 + 1;
    pprandom->setSeed(((xt * xScale) + (zt * zScale)) ^ level->getSeed());

    if (generateStructures) {
        villageFeature->postProcess(level, pprandom, xt, zt);
    }

    app.processSchematics(parent->getChunk(xt, zt));
}

bool FlatLevelSource::save(bool force, ProgressListener* progressListener) {
    return true;
}

bool FlatLevelSource::tick() { return false; }

bool FlatLevelSource::shouldSave() { return true; }

std::wstring FlatLevelSource::gatherStats() { return L"FlatLevelSource"; }

std::vector<Biome::MobSpawnerData*>* FlatLevelSource::getMobsAt(
    MobCategory* mobCategory, int x, int y, int z) {
    Biome* biome = level->getBiome(x, z);
    if (biome == nullptr) {
        return nullptr;
    }
    return biome->getMobs(mobCategory);
}

TilePos* FlatLevelSource::findNearestMapFeature(Level* level,
                                                const std::wstring& featureName,
                                                int x, int y, int z) {
    return nullptr;
}

void FlatLevelSource::recreateLogicStructuresForChunk(int chunkX, int chunkZ) {
    
}