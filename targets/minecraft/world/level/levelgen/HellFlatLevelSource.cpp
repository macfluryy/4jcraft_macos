#include "HellFlatLevelSource.h"

#include <stdlib.h>
#include <string.h>

#include <cmath>
#include <vector>

#include "app/mac/MacGame.h"
#include "java/Random.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/biome/Biome.h"
#include "minecraft/world/level/chunk/ChunkSource.h"
#include "minecraft/world/level/chunk/LevelChunk.h"
#include "minecraft/world/level/levelgen/feature/HellFireFeature.h"
#include "minecraft/world/level/levelgen/feature/LightGemFeature.h"
#include "minecraft/world/level/storage/LevelData.h"
#include "minecraft/world/level/tile/HeavyTile.h"
#include "minecraft/world/level/tile/Tile.h"

HellFlatLevelSource::HellFlatLevelSource(Level* level, int64_t seed) {
    int xzSize = level->getLevelData()->getXZSize();
    int hellScale = level->getLevelData()->getHellScale();
    m_XZSize = ceil((float)xzSize / hellScale);

    this->level = level;

    random = new Random(seed);
    pprandom = new Random(seed);
}

HellFlatLevelSource::~HellFlatLevelSource() {
    delete random;
    delete pprandom;
}

void HellFlatLevelSource::prepareHeights(int xOffs, int zOffs,
                                         std::vector<uint8_t>& blocks) {
    int height = blocks.size() / (16 * 16);

    for (int xc = 0; xc < 16; xc++) {
        for (int zc = 0; zc < 16; zc++) {
            for (int yc = 0; yc < height; yc++) {
                int block = 0;
                if ((yc <= 6) || (yc >= 121)) {
                    block = Tile::netherRack_Id;
                }

                blocks[xc << 11 | zc << 7 | yc] = (uint8_t)block;
            }
        }
    }
}

void HellFlatLevelSource::buildSurfaces(int xOffs, int zOffs,
                                        std::vector<uint8_t>& blocks) {
    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            for (int y = Level::genDepthMinusOne; y >= 0; y--) {
                int offs = (z * 16 + x) * Level::genDepth + y;

                
                bool blockSet = false;
                if (xOffs <= -(m_XZSize / 2)) {
                    if (z - random->nextInt(4) <= 0 ||
                        xOffs < -(m_XZSize / 2)) {
                        blocks[offs] = (uint8_t)Tile::unbreakable_Id;
                        blockSet = true;
                    }
                }
                if (zOffs <= -(m_XZSize / 2)) {
                    if (x - random->nextInt(4) <= 0 ||
                        zOffs < -(m_XZSize / 2)) {
                        blocks[offs] = (uint8_t)Tile::unbreakable_Id;
                        blockSet = true;
                    }
                }
                if (xOffs >= (m_XZSize / 2) - 1) {
                    if (z + random->nextInt(4) >= 15 ||
                        xOffs > (m_XZSize / 2)) {
                        blocks[offs] = (uint8_t)Tile::unbreakable_Id;
                        blockSet = true;
                    }
                }
                if (zOffs >= (m_XZSize / 2) - 1) {
                    if (x + random->nextInt(4) >= 15 ||
                        zOffs > (m_XZSize / 2)) {
                        blocks[offs] = (uint8_t)Tile::unbreakable_Id;
                        blockSet = true;
                    }
                }
                if (blockSet) continue;
                

                if (y >= Level::genDepthMinusOne - random->nextInt(5)) {
                    blocks[offs] = (uint8_t)Tile::unbreakable_Id;
                } else if (y <= 0 + random->nextInt(5)) {
                    blocks[offs] = (uint8_t)Tile::unbreakable_Id;
                }
            }
        }
    }
}

LevelChunk* HellFlatLevelSource::create(int x, int z) { return getChunk(x, z); }

LevelChunk* HellFlatLevelSource::getChunk(int xOffs, int zOffs) {
    random->setSeed(xOffs * 341873128712l + zOffs * 132897987541l);

    
    
    int chunksSize = Level::genDepth * 16 * 16;
    uint8_t* tileData = (uint8_t*)malloc(chunksSize);
    memset(tileData, 0, chunksSize);
    std::vector<uint8_t> blocks =
        std::vector<uint8_t>(tileData, tileData + chunksSize);
    
    

    prepareHeights(xOffs, zOffs, blocks);
    buildSurfaces(xOffs, zOffs, blocks);

    
    
    
    

    
    
    
    
    LevelChunk* levelChunk = new LevelChunk(level, blocks, xOffs, zOffs);
    free(tileData);
    return levelChunk;
}









void HellFlatLevelSource::lightChunk(LevelChunk* lc) { lc->recalcHeightmap(); }

bool HellFlatLevelSource::hasChunk(int x, int y) { return true; }

void HellFlatLevelSource::postProcess(ChunkSource* parent, int xt, int zt) {
    HeavyTile::instaFall = true;
    int xo = xt * 16;
    int zo = zt * 16;

    
    
    
    
    
    
    pprandom->setSeed(level->getSeed());
    int64_t xScale = pprandom->nextLong() / 2 * 2 + 1;
    int64_t zScale = pprandom->nextLong() / 2 * 2 + 1;
    pprandom->setSeed(((xt * xScale) + (zt * zScale)) ^ level->getSeed());

    int count = pprandom->nextInt(pprandom->nextInt(10) + 1) + 1;

    for (int i = 0; i < count; i++) {
        int x = xo + pprandom->nextInt(16) + 8;
        int y = pprandom->nextInt(Level::genDepth - 8) + 4;
        int z = zo + pprandom->nextInt(16) + 8;
        HellFireFeature().place(level, pprandom, x, y, z);
    }

    count = pprandom->nextInt(pprandom->nextInt(10) + 1);
    for (int i = 0; i < count; i++) {
        int x = xo + pprandom->nextInt(16) + 8;
        int y = pprandom->nextInt(Level::genDepth - 8) + 4;
        int z = zo + pprandom->nextInt(16) + 8;
        LightGemFeature().place(level, pprandom, x, y, z);
    }

    HeavyTile::instaFall = false;

    app.processSchematics(parent->getChunk(xt, zt));
}

bool HellFlatLevelSource::save(bool force, ProgressListener* progressListener) {
    return true;
}

bool HellFlatLevelSource::tick() { return false; }

bool HellFlatLevelSource::shouldSave() { return true; }

std::wstring HellFlatLevelSource::gatherStats() {
    return L"HellFlatLevelSource";
}

std::vector<Biome::MobSpawnerData*>* HellFlatLevelSource::getMobsAt(
    MobCategory* mobCategory, int x, int y, int z) {
    Biome* biome = level->getBiome(x, z);
    if (biome == nullptr) {
        return nullptr;
    }
    return biome->getMobs(mobCategory);
}

TilePos* HellFlatLevelSource::findNearestMapFeature(
    Level* level, const std::wstring& featureName, int x, int y, int z) {
    return nullptr;
}

void HellFlatLevelSource::recreateLogicStructuresForChunk(int chunkX,
                                                          int chunkZ) {}