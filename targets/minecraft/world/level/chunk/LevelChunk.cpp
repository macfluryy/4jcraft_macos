#include "LevelChunk.h"

#include <string.h>

#include <algorithm>
#include <limits>
#include <mutex>
#include <string>
#include <utility>

#include "app/common/src/Network/GameNetworkManager.h"
#include "app/mac/MacGame.h"
#include "SparseLightStorage.h"
#include "java/Class.h"
#include "java/Random.h"
#include "java/System.h"
#include "minecraft/client/renderer/GameRenderer.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/server/level/ServerChunkCache.h"
#include "minecraft/server/level/ServerLevel.h"
#include "minecraft/util/Mth.h"
#include "minecraft/world/entity/Entity.h"
#include "minecraft/world/entity/EntityIO.h"
#include "minecraft/world/entity/EntitySelector.h"
#include "minecraft/world/entity/LivingEntity.h"
#include "minecraft/world/entity/Mob.h"
#include "minecraft/world/entity/item/ItemEntity.h"
#include "minecraft/world/entity/item/Minecart.h"
#include "minecraft/world/entity/monster/Monster.h"
#include "minecraft/world/entity/monster/Zombie.h"
#include "minecraft/world/entity/player/Player.h"
#include "minecraft/world/level/ChunkPos.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/LightLayer.h"
#include "minecraft/world/level/TilePos.h"
#include "minecraft/world/level/biome/Biome.h"
#include "minecraft/world/level/biome/BiomeSource.h"
#include "minecraft/world/level/chunk/ChunkSource.h"
#include "minecraft/world/level/chunk/CompressedTileStorage.h"
#include "minecraft/world/level/chunk/SparseDataStorage.h"
#include "minecraft/world/level/dimension/Dimension.h"
#include "minecraft/world/level/material/Material.h"
#include "minecraft/world/level/tile/ChestTile.h"
#include "minecraft/world/level/tile/EntityTile.h"
#include "minecraft/world/level/tile/Tile.h"
#include "minecraft/world/level/tile/entity/TileEntity.h"
#include "minecraft/world/phys/AABB.h"
#include "nbt/CompoundTag.h"
#include "nbt/ListTag.h"

class DataInputStream;
class DataOutputStream;

#if defined(SHARING_ENABLED)
std::recursive_mutex LevelChunk::m_csSharing;
#endif
std::recursive_mutex LevelChunk::m_csEntities;
std::recursive_mutex LevelChunk::m_csTileEntities;
bool LevelChunk::touchedSky = false;

void LevelChunk::staticCtor() {}

void LevelChunk::init(Level* level, int x, int z) {
    biomes = std::vector<uint8_t>(16 * 16);
    for (int i = 0; i < 16 * 16; i++) {
        biomes[i] = 0xff;
    }
    {
        std::lock_guard<std::recursive_mutex> lock(m_csEntities);
        entityBlocks =
            new std::vector<std::shared_ptr<Entity> >*[ENTITY_BLOCKS_LENGTH];
    }

    terrainPopulated = 0;
    m_unsaved = false;
    lastSaveHadEntities = false;
    lastSaveTime = 0;
    dontSave = false;
    loaded = false;
    minHeight = 0;
    hasGapsToCheck = false;
    seenByPlayer = true;  

    
    checkLightPosition = 0;  

    this->level = level;
    this->x = x;
    this->z = z;
    heightmap = std::vector<uint8_t>(16 * 16);
    {
        std::lock_guard<std::recursive_mutex> lock(m_csEntities);
        for (int i = 0; i < ENTITY_BLOCKS_LENGTH; i++) {
            entityBlocks[i] = new std::vector<std::shared_ptr<Entity> >();
        }
    }

    lowestHeightmap = 256;
    inhabitedTime = 0;

    
    
    for (int i = 0; i < 16 * 16; i++) {
        rainHeights[i] = 255;
    }
    
    
    
    for (int i = 0; i < 8 * 16; i++) {
        columnFlags[i] = 0;
    }

    
    
    
    
    emissiveAdded = true;

#if defined(_LARGE_WORLDS)
    m_bUnloaded = false;  
    m_unloadedEntitiesTag = nullptr;
#endif
}


LevelChunk::LevelChunk(Level* level, int x, int z)
    : ENTITY_BLOCKS_LENGTH(Level::maxBuildHeight / 16) {
    init(level, x, z);
    lowerBlocks = new CompressedTileStorage();
    lowerData = nullptr;
    lowerSkyLight = nullptr;
    lowerBlockLight = nullptr;
    serverTerrainPopulated = nullptr;

    if (Level::maxBuildHeight > Level::COMPRESSED_CHUNK_SECTION_HEIGHT) {
        
        
        upperBlocks = new CompressedTileStorage(true);
        upperData = new SparseDataStorage(true);
        upperSkyLight = new SparseLightStorage(true, true);
        upperBlockLight = new SparseLightStorage(false, true);
    } else {
        upperBlocks = nullptr;
        upperData = nullptr;
        upperSkyLight = nullptr;
        upperBlockLight = nullptr;
    }

#if defined(SHARING_ENABLED)
    sharingTilesAndData = false;
#endif
}




LevelChunk::LevelChunk(Level* level, std::vector<uint8_t>& blocks, int x, int z)
    : ENTITY_BLOCKS_LENGTH(Level::maxBuildHeight / 16) {
    init(level, x, z);

    
    
    
    
    
    
    bool createEmpty = blocks.empty();

    if (createEmpty) {
        lowerBlocks = new CompressedTileStorage(true);
        lowerData = new SparseDataStorage(true);

        lowerSkyLight = new SparseLightStorage(true, true);
        lowerBlockLight = new SparseLightStorage(false, true);
    } else {
        lowerBlocks = new CompressedTileStorage(blocks, 0);
        lowerData = new SparseDataStorage();

        
        lowerSkyLight = new SparseLightStorage(true);
        lowerBlockLight = new SparseLightStorage(false);
    }
    
    

    if (Level::maxBuildHeight > Level::COMPRESSED_CHUNK_SECTION_HEIGHT) {
        if (blocks.size() > Level::COMPRESSED_CHUNK_SECTION_TILES)
            upperBlocks = new CompressedTileStorage(
                blocks, Level::COMPRESSED_CHUNK_SECTION_TILES);
        else
            upperBlocks = new CompressedTileStorage(true);
        upperData = new SparseDataStorage(true);
        upperSkyLight = new SparseLightStorage(true, true);
        upperBlockLight = new SparseLightStorage(false, true);
    } else {
        upperBlocks = nullptr;
        upperData = nullptr;
        upperSkyLight = nullptr;
        upperBlockLight = nullptr;
    }

    serverTerrainPopulated = nullptr;
#if defined(SHARING_ENABLED)
    sharingTilesAndData = false;
#endif
}





LevelChunk::LevelChunk(Level* level, int x, int z, LevelChunk* lc)
    : ENTITY_BLOCKS_LENGTH(Level::maxBuildHeight / 16) {
    init(level, x, z);

    
    memcpy(biomes.data(), lc->biomes.data(), biomes.size());

#if defined(SHARING_ENABLED)
    lowerBlocks = lc->lowerBlocks;
    lowerData = lc->lowerData;
    lowerSkyLight = new SparseLightStorage(lc->lowerSkyLight);
    lowerBlockLight = new SparseLightStorage(lc->lowerBlockLight);
    upperBlocks = lc->upperBlocks;
    upperData = lc->upperData;
    upperSkyLight = new SparseLightStorage(lc->upperSkyLight);
    upperBlockLight = new SparseLightStorage(lc->upperBlockLight);

    sharingTilesAndData = true;
    serverTerrainPopulated = &lc->terrainPopulated;
#else
    this->blocks = new CompressedTileStorage(lc->blocks);
    this->data = new SparseDataStorage(lc->data);
    this->skyLight = new SparseLightStorage(lc->skyLight);
    this->blockLight = new SparseLightStorage(lc->blockLight);
    serverTerrainPopulated = nullptr;
#endif
}


void LevelChunk::setUnsaved(bool unsaved) {
#if defined(_LARGE_WORLDS)
    if (m_unsaved != unsaved) {
        if (unsaved)
            level->incrementUnsavedChunkCount();
        else
            level->decrementUnsavedChunkCount();
    }
#endif
    m_unsaved = unsaved;
}

void LevelChunk::stopSharingTilesAndData() {
#if defined(SHARING_ENABLED)
    {
        std::lock_guard<std::recursive_mutex> lock(m_csSharing);
        lastUnsharedTime = System::currentTimeMillis();
        if (!sharingTilesAndData) {
            return;
        }

        
        
        
        
        
        
        if ((serverTerrainPopulated) &&
            (((*serverTerrainPopulated) & sTerrainPopulatedAllAffecting) !=
             sTerrainPopulatedAllAffecting)) {
            return;
        }

        
        
        
        if (isEmpty()) {
            return;
        }

        
        
        lowerBlocks = new CompressedTileStorage(lowerBlocks);

        
        
        lowerData = new SparseDataStorage(lowerData);

        if (Level::maxBuildHeight > Level::COMPRESSED_CHUNK_SECTION_HEIGHT) {
            upperBlocks = new CompressedTileStorage(upperBlocks);
            upperData = new SparseDataStorage(upperData);
        } else {
            upperBlocks = nullptr;
            upperData = nullptr;
        }

        










        sharingTilesAndData = false;
    }
#endif
}






void LevelChunk::reSyncLighting() {
#if defined(SHARING_ENABLED)
    {
        std::lock_guard<std::recursive_mutex> lock(m_csSharing);

        if (isEmpty()) {
            return;
        }

#if defined(_LARGE_WORLDS)
        LevelChunk* lc = MinecraftServer::getInstance()
                             ->getLevel(level->dimension->id)
                             ->cache->getChunkLoadedOrUnloaded(x, z);
#else
        LevelChunk* lc = MinecraftServer::getInstance()
                             ->getLevel(level->dimension->id)
                             ->cache->getChunk(x, z);
#endif

        GameRenderer::AddForDelete(lowerSkyLight);
        lowerSkyLight = new SparseLightStorage(lc->lowerSkyLight);
        GameRenderer::FinishedReassigning();
        GameRenderer::AddForDelete(lowerBlockLight);
        lowerBlockLight = new SparseLightStorage(lc->lowerBlockLight);
        GameRenderer::FinishedReassigning();

        if (Level::maxBuildHeight > Level::COMPRESSED_CHUNK_SECTION_HEIGHT) {
            GameRenderer::AddForDelete(upperSkyLight);
            upperSkyLight = new SparseLightStorage(lc->upperSkyLight);
            GameRenderer::FinishedReassigning();
            GameRenderer::AddForDelete(upperBlockLight);
            upperBlockLight = new SparseLightStorage(lc->upperBlockLight);
            GameRenderer::FinishedReassigning();
        }
    }
#endif
}

void LevelChunk::startSharingTilesAndData(int forceMs) {
#if defined(SHARING_ENABLED)
    {
        std::lock_guard<std::recursive_mutex> lock(m_csSharing);
        if (sharingTilesAndData) {
            return;
        }

        
        
        
        
        if (isEmpty()) {
            return;
        }

#if defined(_LARGE_WORLDS)
        LevelChunk* lc = MinecraftServer::getInstance()
                             ->getLevel(level->dimension->id)
                             ->cache->getChunkLoadedOrUnloaded(x, z);
#else
        LevelChunk* lc = MinecraftServer::getInstance()
                             ->getLevel(level->dimension->id)
                             ->cache->getChunk(x, z);
#endif

        
        
        
        if (forceMs == 0) {
            
            
            if (!lowerBlocks->isSameAs(lc->lowerBlocks) ||
                (upperBlocks && lc->upperBlocks &&
                 !upperBlocks->isSameAs(lc->upperBlocks))) {
                return;
            }
        } else {
            
            
            int64_t timenow = System::currentTimeMillis();
            if ((timenow - lastUnsharedTime) < forceMs) {
                return;
            }
        }

        
        
        
        GameRenderer::AddForDelete(lowerBlocks);
        lowerBlocks = lc->lowerBlocks;
        GameRenderer::FinishedReassigning();

        GameRenderer::AddForDelete(lowerData);
        lowerData = lc->lowerData;
        GameRenderer::FinishedReassigning();

        if (Level::maxBuildHeight > Level::COMPRESSED_CHUNK_SECTION_HEIGHT) {
            GameRenderer::AddForDelete(upperBlocks);
            upperBlocks = lc->upperBlocks;
            GameRenderer::FinishedReassigning();

            GameRenderer::AddForDelete(upperData);
            upperData = lc->upperData;
            GameRenderer::FinishedReassigning();
        }

        sharingTilesAndData = true;
    }
#endif
}

LevelChunk::~LevelChunk() {
#if defined(SHARING_ENABLED)
    if (!sharingTilesAndData)
#endif
    {
        delete lowerData;
        delete lowerBlocks;
        if (upperData) delete upperData;
        if (upperBlocks) delete upperBlocks;
    }

    delete lowerSkyLight;
    delete lowerBlockLight;
    if (upperSkyLight) delete upperSkyLight;
    if (upperBlockLight) delete upperBlockLight;

    for (int i = 0; i < ENTITY_BLOCKS_LENGTH; ++i) delete entityBlocks[i];
    delete[] entityBlocks;

#if defined(_LARGE_WORLDS)
    delete m_unloadedEntitiesTag;
#endif
}

bool LevelChunk::isAt(int x, int z) { return x == this->x && z == this->z; }

int LevelChunk::getHeightmap(int x, int z) {
    return heightmap[z << 4 | x] & 0xff;
}

int LevelChunk::getHighestSectionPosition() {
    return Level::maxBuildHeight - 16;
    
    
    
    
    
    
    
}

void LevelChunk::recalcBlockLights() {}

void LevelChunk::recalcHeightmapOnly() {
    int min = Level::maxBuildHeight - 1;
    for (int x = 0; x < 16; x++)
        for (int z = 0; z < 16; z++) {
            rainHeights[x + ((unsigned)z << 4)] =
                255;  
                      

            int y = Level::maxBuildHeight - 1;
            
            
            CompressedTileStorage* blocks =
                (y - 1) >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT ? upperBlocks
                                                                  : lowerBlocks;
            while (
                y > 0 &&
                Tile::lightBlock[blocks->get(
                                     x,
                                     (y - 1) %
                                         Level::COMPRESSED_CHUNK_SECTION_HEIGHT,
                                     z) &
                                 0xff] ==
                    0)  
            {
                y--;
                blocks = (y - 1) >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT
                             ? upperBlocks
                             : lowerBlocks;
            }
            heightmap[(unsigned)z << 4 | x] = (uint8_t)y;
            if (y < min) min = y;
        }

    this->minHeight = min;
    this->setUnsaved(true);
}

void LevelChunk::recalcHeightmap() {
    lowestHeightmap = std::numeric_limits<int>::max();

    int min = Level::maxBuildHeight - 1;
    for (int x = 0; x < 16; x++)
        for (int z = 0; z < 16; z++) {
            int y = Level::maxBuildHeight - 1;
            
            

            CompressedTileStorage* blocks =
                (y - 1) >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT ? upperBlocks
                                                                  : lowerBlocks;
            while (
                y > 0 &&
                Tile::lightBlock[blocks->get(
                                     x,
                                     (y - 1) %
                                         Level::COMPRESSED_CHUNK_SECTION_HEIGHT,
                                     z) &
                                 0xff] ==
                    0)  
            {
                y--;
                blocks = (y - 1) >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT
                             ? upperBlocks
                             : lowerBlocks;
            }
            heightmap[(unsigned)z << 4 | x] = (uint8_t)y;
            if (y < min) min = y;
            if (y < lowestHeightmap) lowestHeightmap = y;

            if (!level->dimension->hasCeiling) {
                int br = Level::MAX_BRIGHTNESS;
                int yy = Level::maxBuildHeight - 1;
                CompressedTileStorage* blocks =
                    yy >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT ? upperBlocks
                                                                 : lowerBlocks;
                SparseLightStorage* skyLight =
                    yy >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT
                        ? upperSkyLight
                        : lowerSkyLight;
                do {
                    br -= Tile::lightBlock
                        [blocks->get(
                             x, (yy % Level::COMPRESSED_CHUNK_SECTION_HEIGHT),
                             z) &
                         0xff];  
                    if (br > 0) {
                        skyLight->set(
                            x, (yy % Level::COMPRESSED_CHUNK_SECTION_HEIGHT), z,
                            br);
                    }
                    yy--;
                    blocks = yy >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT
                                 ? upperBlocks
                                 : lowerBlocks;
                    skyLight = yy >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT
                                   ? upperSkyLight
                                   : lowerSkyLight;
                } while (yy > 0 && br > 0);
            }
        }

    this->minHeight = min;

    for (int x = 0; x < 16; x++)
        for (int z = 0; z < 16; z++) {
            lightGaps(x, z);
        }

    this->setUnsaved(true);
}





void LevelChunk::lightLava() {
    if (!emissiveAdded) return;

    for (int x = 0; x < 16; x++)
        for (int z = 0; z < 16; z++) {
            
            
            int ymax = getHeightmap(x, z);
            for (int y = 0; y < Level::COMPRESSED_CHUNK_SECTION_HEIGHT; y++) {
                CompressedTileStorage* blocks = lowerBlocks;
                int emit = Tile::lightEmission[blocks->get(
                    x, y, z)];  
                if (emit > 0) {
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    if (getBrightness(LightLayer::Block, x, y, z) < emit) {
                        level->checkLight(LightLayer::Block, this->x * 16 + x,
                                          y, this->z * 16 + z, true);
                    }
                }
            }
        }
    emissiveAdded = false;
}

void LevelChunk::lightGaps(int x, int z) {
    
    
    
    int slot = (x >> 1) | (z * 8);
    int shift = (x & 1) * 4;
    columnFlags[slot] |= (eColumnFlag_recheck << shift);
    hasGapsToCheck = true;
}
void LevelChunk::recheckGaps(bool bForce) {
    
    
    
    
    
    if (isEmpty()) return;

    
    int minXZ = -(level->dimension->getXZSize() * 16) / 2;
    int maxXZ = (level->dimension->getXZSize() * 16) / 2 - 1;

    
    
    
    if (level->hasChunksAt(x * 16 + 8, Level::maxBuildHeight / 2, z * 16 + 8,
                           16)) {
        for (int x = 0; x < 16; x++)
            for (int z = 0; z < 16; z++) {
                int slot = (x >> 1) | (z * 8);
                int shift = (x & 1) * 4;
                if (bForce ||
                    (columnFlags[slot] & (eColumnFlag_recheck << shift))) {
                    columnFlags[slot] &= ~(eColumnFlag_recheck << shift);
                    int height = getHeightmap(x, z);
                    int xOffs = (this->x * 16) + x;
                    int zOffs = (this->z * 16) + z;

                    
                    
                    
                    
                    
                    int nmin = level->getHeightmap(xOffs, zOffs);
                    if (xOffs - 1 >= minXZ) {
                        int n = level->getHeightmap(xOffs - 1, zOffs);
                        if (n < nmin) nmin = n;
                    }
                    if (xOffs + 1 <= maxXZ) {
                        int n = level->getHeightmap(xOffs + 1, zOffs);
                        if (n < nmin) nmin = n;
                    }
                    if (zOffs - 1 >= minXZ) {
                        int n = level->getHeightmap(xOffs, zOffs - 1);
                        if (n < nmin) nmin = n;
                    }
                    if (zOffs + 1 <= maxXZ) {
                        int n = level->getHeightmap(xOffs, zOffs + 1);
                        if (n < nmin) nmin = n;
                    }
                    lightGap(xOffs, zOffs, nmin);

                    if (!bForce)  
                                  
                                  
                    {
                        if (xOffs - 1 >= minXZ)
                            lightGap(xOffs - 1, zOffs, height);
                        if (xOffs + 1 <= maxXZ)
                            lightGap(xOffs + 1, zOffs, height);
                        if (zOffs - 1 >= minXZ)
                            lightGap(xOffs, zOffs - 1, height);
                        if (zOffs + 1 <= maxXZ)
                            lightGap(xOffs, zOffs + 1, height);
                    }
                    hasGapsToCheck = false;
                }
            }
    }
}

void LevelChunk::lightGap(int x, int z, int source) {
    int height = level->getHeightmap(x, z);

    if (height > source) {
        lightGap(x, z, source, height + 1);
    } else if (height < source) {
        lightGap(x, z, height, source + 1);
    }
}

void LevelChunk::lightGap(int x, int z, int y1, int y2) {
    if (y2 > y1) {
        if (level->hasChunksAt(x, Level::maxBuildHeight / 2, z, 16)) {
            for (int y = y1; y < y2; y++) {
                level->checkLight(LightLayer::Sky, x, y, z);
            }
            this->setUnsaved(true);
        }
    }
}

void LevelChunk::recalcHeight(int x, int yStart, int z) {
    int yOld = heightmap[(unsigned)z << 4 | x] & 0xff;
    int y = yOld;
    if (yStart > yOld) y = yStart;

    
    

    CompressedTileStorage* blocks =
        (y - 1) >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT ? upperBlocks
                                                          : lowerBlocks;
    while (
        y > 0 &&
        Tile::lightBlock
                [blocks->get(
                     x, (y - 1) % Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z) &
                 0xff] == 0)  
    {
        y--;
        blocks = (y - 1) >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT
                     ? upperBlocks
                     : lowerBlocks;
    }
    if (y == yOld) return;

    
    
    heightmap[(unsigned)z << 4 | x] = (uint8_t)y;

    if (y < minHeight) {
        minHeight = y;
    } else {
        int min = Level::maxBuildHeight - 1;
        for (int _x = 0; _x < 16; _x++)
            for (int _z = 0; _z < 16; _z++) {
                if ((heightmap[(unsigned)_z << 4 | _x] & 0xff) < min)
                    min = (heightmap[(unsigned)_z << 4 | _x] & 0xff);
            }
        this->minHeight = min;
    }

    int xOffs = (this->x * 16) + x;
    int zOffs = (this->z * 16) + z;
    if (!level->dimension->hasCeiling) {
        if (y < yOld) {
            SparseLightStorage* skyLight =
                y >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT ? upperSkyLight
                                                            : lowerSkyLight;
            for (int yy = y; yy < yOld; yy++) {
                skyLight = yy >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT
                               ? upperSkyLight
                               : lowerSkyLight;
                skyLight->set(x, (yy % Level::COMPRESSED_CHUNK_SECTION_HEIGHT),
                              z, 15);
            }
        } else {
            
            
            
            SparseLightStorage* skyLight =
                y >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT ? upperSkyLight
                                                            : lowerSkyLight;
            for (int yy = yOld; yy < y; yy++) {
                skyLight = yy >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT
                               ? upperSkyLight
                               : lowerSkyLight;
                skyLight->set(x, (yy % Level::COMPRESSED_CHUNK_SECTION_HEIGHT),
                              z, 0);
            }
        }

        int br = 15;

        SparseLightStorage* skyLight =
            y >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT ? upperSkyLight
                                                        : lowerSkyLight;
        while (y > 0 && br > 0) {
            y--;
            skyLight = y >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT
                           ? upperSkyLight
                           : lowerSkyLight;
            int block = Tile::lightBlock[getTile(x, y, z)];
            if (block == 0) block = 1;
            br -= block;
            if (br < 0) br = 0;
            skyLight->set(x, (y % Level::COMPRESSED_CHUNK_SECTION_HEIGHT), z,
                          br);
            
            
        }
    }
    
    
    
    
    
    level->lightColumnChanged(xOffs, zOffs, y, yOld);

    
    int height = heightmap[(unsigned)z << 4 | x];
    int y1 = yOld;
    int y2 = height;
    if (y2 < y1) {
        int tmp = y1;
        y1 = y2;
        y2 = tmp;
    }
    if (height < lowestHeightmap) lowestHeightmap = height;
    if (!level->dimension->hasCeiling) {
        lightGap(xOffs - 1, zOffs, y1, y2);
        lightGap(xOffs + 1, zOffs, y1, y2);
        lightGap(xOffs, zOffs - 1, y1, y2);
        lightGap(xOffs, zOffs + 1, y1, y2);
        lightGap(xOffs, zOffs, y1, y2);
    }

    this->setUnsaved(true);
}










int LevelChunk::getTileLightBlock(int x, int y, int z) {
    return Tile::lightBlock[getTile(x, y, z)];
}

int LevelChunk::getTile(int x, int y, int z) {
    CompressedTileStorage* blocks =
        y >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT ? upperBlocks : lowerBlocks;
    return blocks->get(x, y % Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z);
}

bool LevelChunk::setTileAndData(int x, int y, int z, int _tile, int _data) {
    uint8_t tile = (uint8_t)_tile;

    
    
    int slot = (unsigned)z << 4 | x;

    if (y >= ((int)rainHeights[slot]) - 1) {
        rainHeights[slot] = 255;
    }

    int oldHeight = heightmap[slot] & 0xff;

    CompressedTileStorage* blocks =
        y >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT ? upperBlocks : lowerBlocks;
    SparseDataStorage* data =
        y >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT ? upperData : lowerData;
    int old = blocks->get(x, y % Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z);
    int oldData = data->get(x, y % Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z);
    if (old == _tile && oldData == _data) {
        
        
        std::shared_ptr<TileEntity> te = getTileEntity(x, y, z);
        if (te != nullptr) {
            te->clearCache();
        }

        return false;
    }
    int xOffs = this->x * 16 + x;
    int zOffs = this->z * 16 + z;
    if (old != 0 && !level->isClientSide) {
        Tile::tiles[old]->onRemoving(level, xOffs, y, zOffs, oldData);
    }
    blocks->set(x, y % Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z, tile);

    if (old != 0) {
        if (!level->isClientSide) {
            Tile::tiles[old]->onRemove(level, xOffs, y, zOffs, old, oldData);
        } else if (Tile::tiles[old]->isEntityTile() && old != _tile) {
            level->removeTileEntity(xOffs, y, zOffs);
        }
    }
    data->set(x, y % Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z, _data);

    
    
    if (Tile::lightEmission[tile & 0xff] > 0) {
        emissiveAdded = true;
    }

    
    
    
    if (Tile::lightBlock[tile & 0xff] != Tile::lightBlock[old & 0xff]) {
        if (!level->dimension->hasCeiling) {
            if (Tile::lightBlock[tile & 0xff] != 0) {
                if (y >= oldHeight) {
                    recalcHeight(x, y + 1, z);
                }
            } else {
                if (y == oldHeight - 1) {
                    recalcHeight(x, y, z);
                }
            }
        }

        
        
        lightGaps(x, z);
    }

    data->set(x, y % Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z, _data);
    if (_tile != 0) {
        if (!level->isClientSide) {
            Tile::tiles[_tile]->onPlace(level, xOffs, y, zOffs);
        } else {
            
            
            
            
            
            
            
            if (_tile == Tile::fire_Id) {
                if (!Tile::tiles[_tile]->mayPlace(level, xOffs, y, zOffs)) {
                    blocks->set(x, y % Level::COMPRESSED_CHUNK_SECTION_HEIGHT,
                                z, 0);
                    
                    
                    
                }
            }
        }
        
        
        
        
        if (_tile > 0 && Tile::tiles[_tile] != nullptr &&
            Tile::tiles[_tile]->isEntityTile()) {
            std::shared_ptr<TileEntity> te = getTileEntity(x, y, z);
            if (te == nullptr) {
                te = dynamic_cast<EntityTile*>(Tile::tiles[_tile])
                         ->newTileEntity(level);
                
                
                
                level->setTileEntity(xOffs, y, zOffs, te);
            }
            if (te != nullptr) {
                
                
                
                te->clearCache();
            }
        }
    }
    
    
    
    
    else if (old > 0 && Tile::tiles[_tile] != nullptr &&
             Tile::tiles[_tile]->isEntityTile()) {
        std::shared_ptr<TileEntity> te = getTileEntity(x, y, z);
        if (te != nullptr) {
            te->clearCache();
        }
    }

    this->setUnsaved(true);
    return true;
}

bool LevelChunk::setTile(int x, int y, int z, int _tile) {
    
    
    return setTileAndData(x, y, z, _tile, 0);
}

int LevelChunk::getData(int x, int y, int z) {
    SparseDataStorage* data =
        y >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT ? upperData : lowerData;
    return data->get(x, y % Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z);
}

bool LevelChunk::setData(int x, int y, int z, int val, int mask,
                         bool* maskedBitsChanged) {
    SparseDataStorage* data =
        y >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT ? upperData : lowerData;
    this->setUnsaved(true);
    int old = data->get(x, y % Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z);

    *maskedBitsChanged = ((old & mask) != (val & mask));

    if (old == val) {
        return false;
    }

    data->set(x, y % Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z, val);
    int _tile = getTile(x, y, z);
    if (_tile > 0 && dynamic_cast<EntityTile*>(Tile::tiles[_tile]) != nullptr) {
        std::shared_ptr<TileEntity> te = getTileEntity(x, y, z);
        if (te != nullptr) {
            te->clearCache();
            te->data = val;
        }
    }
    return true;
}

int LevelChunk::getBrightness(LightLayer::variety layer, int x, int y, int z) {
    if (layer == LightLayer::Sky) {
        if (level->dimension->hasCeiling) {
            return 0;
        }
        SparseLightStorage* skyLight =
            y >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT ? upperSkyLight
                                                        : lowerSkyLight;
        if (!skyLight) return 0;
        return skyLight->get(x, y % Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z);
    } else if (layer == LightLayer::Block) {
        SparseLightStorage* blockLight =
            y >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT ? upperBlockLight
                                                        : lowerBlockLight;
        if (!blockLight) return 0;
        return blockLight->get(x, y % Level::COMPRESSED_CHUNK_SECTION_HEIGHT,
                               z);
    } else
        return 0;
}


void LevelChunk::getNeighbourBrightnesses(int* brightnesses,
                                          LightLayer::variety layer, int x,
                                          int y, int z) {
    SparseLightStorage* light;
    if (layer == LightLayer::Sky)
        light = y >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT ? upperSkyLight
                                                            : lowerSkyLight;
    else
        light = y >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT ? upperBlockLight
                                                            : lowerBlockLight;

    if (light) {
        brightnesses[0] =
            light->get(x - 1, y % Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z);
        brightnesses[1] =
            light->get(x + 1, y % Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z);
        brightnesses[4] =
            light->get(x, y % Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z - 1);
        brightnesses[5] =
            light->get(x, y % Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z + 1);
    }

    if (layer == LightLayer::Sky)
        light = (y - 1) >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT
                    ? upperSkyLight
                    : lowerSkyLight;
    else
        light = (y - 1) >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT
                    ? upperBlockLight
                    : lowerBlockLight;
    if (light)
        brightnesses[2] =
            light->get(x, (y - 1) % Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z);

    if (layer == LightLayer::Sky)
        light = (y + 1) >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT
                    ? upperSkyLight
                    : lowerSkyLight;
    else
        light = (y + 1) >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT
                    ? upperBlockLight
                    : lowerBlockLight;
    if (light)
        brightnesses[3] =
            light->get(x, (y + 1) % Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z);
}

void LevelChunk::setBrightness(LightLayer::variety layer, int x, int y, int z,
                               int brightness) {
    this->setUnsaved(true);
    if (layer == LightLayer::Sky) {
        if (!level->dimension->hasCeiling) {
            SparseLightStorage* skyLight =
                y >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT ? upperSkyLight
                                                            : lowerSkyLight;
            skyLight->set(x, y % Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z,
                          brightness);
        }
    } else if (layer == LightLayer::Block) {
        SparseLightStorage* blockLight =
            y >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT ? upperBlockLight
                                                        : lowerBlockLight;
        blockLight->set(x, y % Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z,
                        brightness);
    }
}

int LevelChunk::getRawBrightness(int x, int y, int z, int skyDampen) {
    SparseLightStorage* skyLight = y >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT
                                       ? upperSkyLight
                                       : lowerSkyLight;
    int light =
        level->dimension->hasCeiling
            ? 0
            : skyLight->get(x, y % Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z);
    if (light > 0) touchedSky = true;
    light -= skyDampen;
    SparseLightStorage* blockLight = y >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT
                                         ? upperBlockLight
                                         : lowerBlockLight;
    int block =
        blockLight->get(x, y % Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z);
    if (block > light) light = block;

    







    return light;
}

void LevelChunk::addEntity(std::shared_ptr<Entity> e) {
    lastSaveHadEntities = true;

    int xc = Mth::floor(e->x / 16);
    int zc = Mth::floor(e->z / 16);
    if (xc != this->x || zc != this->z) {
        app.DebugPrintf("Wrong location!");
        
        
    }
    int yc = Mth::floor(e->y / 16);
    if (yc < 0) yc = 0;
    if (yc >= ENTITY_BLOCKS_LENGTH) yc = ENTITY_BLOCKS_LENGTH - 1;
    e->inChunk = true;
    e->xChunk = x;
    e->yChunk = yc;
    e->zChunk = z;

    {
        std::lock_guard<std::recursive_mutex> lock(m_csEntities);
        entityBlocks[yc]->push_back(e);
    }
}

void LevelChunk::removeEntity(std::shared_ptr<Entity> e) {
    removeEntity(e, e->yChunk);
}

void LevelChunk::removeEntity(std::shared_ptr<Entity> e, int yc) {
    if (yc < 0) yc = 0;
    if (yc >= ENTITY_BLOCKS_LENGTH) yc = ENTITY_BLOCKS_LENGTH - 1;

    {
        std::lock_guard<std::recursive_mutex> lock(m_csEntities);

        
        auto it = find(entityBlocks[yc]->begin(), entityBlocks[yc]->end(), e);
        if (it != entityBlocks[yc]->end()) {
            entityBlocks[yc]->erase(it);
            
            
            entityBlocks[yc]->shrink_to_fit();
        }
    }
}

bool LevelChunk::isSkyLit(int x, int y, int z) {
    return y >= (heightmap[(unsigned)z << 4 | x] & 0xff);
}

void LevelChunk::skyBrightnessChanged() {
    int x0 = this->x * 16;
    int y0 = this->minHeight - 16;
    int z0 = this->z * 16;
    int x1 = this->x * 16 + 16;
    int y1 = Level::maxBuildHeight - 1;
    int z1 = this->z * 16 + 16;

    level->setTilesDirty(x0, y0, z0, x1, y1, z1);
}

std::shared_ptr<TileEntity> LevelChunk::getTileEntity(int x, int y, int z) {
    TilePos pos(x, y, z);

    
    
    
    std::shared_ptr<TileEntity> tileEntity = nullptr;
    {
        std::unique_lock<std::recursive_mutex> lock(m_csTileEntities);
        auto it = tileEntities.find(pos);

        if (it == tileEntities.end()) {
            lock.unlock();  
                            

            
            
            
            
            
            if (level->m_bDisableAddNewTileEntities) return nullptr;

            int t = getTile(x, y, z);
            if (t <= 0 || !Tile::tiles[t]->isEntityTile()) return nullptr;

            
            
            

            
            
            tileEntity =
                dynamic_cast<EntityTile*>(Tile::tiles[t])->newTileEntity(level);
            level->setTileEntity(this->x * 16 + x, y, this->z * 16 + z,
                                 tileEntity);
            

            
            

            
            
            {
                std::lock_guard<std::recursive_mutex> lock2(m_csTileEntities);
                auto newIt = tileEntities.find(pos);
                if (newIt != tileEntities.end()) {
                    tileEntity = newIt->second;
                }
            }
        } else {
            tileEntity = it->second;
        }
    }
    if (tileEntity != nullptr && tileEntity->isRemoved()) {
        {
            std::lock_guard<std::recursive_mutex> lock(m_csTileEntities);
            tileEntities.erase(pos);
        }
        return nullptr;
    }

    return tileEntity;
}

void LevelChunk::addTileEntity(std::shared_ptr<TileEntity> te) {
    int xx = (int)(te->x - this->x * 16);
    int yy = (int)te->y;
    int zz = (int)(te->z - this->z * 16);
    setTileEntity(xx, yy, zz, te);
    if (loaded) {
        {
            std::lock_guard<std::recursive_mutex> lock(
                level->m_tileEntityListCS);
            level->tileEntityList.push_back(te);
        }
    }
}

void LevelChunk::setTileEntity(int x, int y, int z,
                               std::shared_ptr<TileEntity> tileEntity) {
    TilePos pos(x, y, z);

    tileEntity->setLevel(level);
    tileEntity->x = this->x * 16 + x;
    tileEntity->y = y;
    tileEntity->z = this->z * 16 + z;

    if (getTile(x, y, z) == 0 ||
        !Tile::tiles[getTile(x, y, z)]
             ->isEntityTile())  
                                
    {
        app.DebugPrintf(
            "Attempted to place a tile entity where there was no entity "
            "tile!\n");
        return;
    }
    auto it = tileEntities.find(pos);
    if (it != tileEntities.end()) it->second->setRemoved();

    tileEntity->clearRemoved();

    {
        std::lock_guard<std::recursive_mutex> lock(m_csTileEntities);
        tileEntities[pos] = tileEntity;
    }
}

void LevelChunk::removeTileEntity(int x, int y, int z) {
    TilePos pos(x, y, z);

    if (loaded) {
        
        
        
        
        
        {
            std::lock_guard<std::recursive_mutex> lock(m_csTileEntities);
            auto it = tileEntities.find(pos);
            if (it != tileEntities.end()) {
                std::shared_ptr<TileEntity> te = tileEntities[pos];
                tileEntities.erase(pos);
                if (te != nullptr) {
                    if (level->isClientSide) {
                        app.DebugPrintf("Removing tile entity of type %d\n",
                                        te->GetType());
                    }
                    te->setRemoved();
                }
            }
        }
    }
}

void LevelChunk::load() {
    loaded = true;

    if (!level->isClientSide) {
#if defined(_LARGE_WORLDS)
        if (m_bUnloaded && m_unloadedEntitiesTag) {
            ListTag<CompoundTag>* entityTags =
                (ListTag<CompoundTag>*)m_unloadedEntitiesTag->getList(
                    L"Entities");
            if (entityTags != nullptr) {
                for (int i = 0; i < entityTags->size(); i++) {
                    CompoundTag* teTag = entityTags->get(i);
                    std::shared_ptr<Entity> ent =
                        EntityIO::loadStatic(teTag, level);
                    if (ent != nullptr) {
                        ent->onLoadedFromSave();
                        addEntity(ent);
                    }
                }
            }

            ListTag<CompoundTag>* tileEntityTags =
                (ListTag<CompoundTag>*)m_unloadedEntitiesTag->getList(
                    L"TileEntities");
            if (tileEntityTags != nullptr) {
                for (int i = 0; i < tileEntityTags->size(); i++) {
                    CompoundTag* teTag = tileEntityTags->get(i);
                    std::shared_ptr<TileEntity> te =
                        TileEntity::loadStatic(teTag);
                    if (te != nullptr) {
                        addTileEntity(te);
                    }
                }
            }
            delete m_unloadedEntitiesTag;
            m_unloadedEntitiesTag = nullptr;
            m_bUnloaded = false;
        }
#endif

        std::vector<std::shared_ptr<TileEntity> > values;
        {
            std::lock_guard<std::recursive_mutex> lock(m_csTileEntities);
            for (auto it = tileEntities.begin(); it != tileEntities.end();
                 it++) {
                values.push_back(it->second);
            }
        }
        level->addAllPendingTileEntities(values);

        {
            std::lock_guard<std::recursive_mutex> lock(m_csEntities);
            for (int i = 0; i < ENTITY_BLOCKS_LENGTH; i++) {
                level->addEntities(entityBlocks[i]);
            }
        }
    } else {
#if defined(_LARGE_WORLDS)
        m_bUnloaded = false;
#endif
    }
}

void LevelChunk::unload(bool unloadTileEntities)  
{
    loaded = false;
    if (unloadTileEntities) {
        std::vector<std::shared_ptr<TileEntity> > tileEntitiesToRemove;
        {
            std::lock_guard<std::recursive_mutex> lock(m_csTileEntities);
            for (auto it = tileEntities.begin(); it != tileEntities.end();
                 it++) {
                tileEntitiesToRemove.push_back(it->second);
            }
        }

        auto itEnd = tileEntitiesToRemove.end();
        for (auto it = tileEntitiesToRemove.begin(); it != itEnd; it++) {
            
            level->markForRemoval(*it);
        }
    }

    {
        std::lock_guard<std::recursive_mutex> lock(m_csEntities);
        for (int i = 0; i < ENTITY_BLOCKS_LENGTH; i++) {
            level->removeEntities(entityBlocks[i]);
        }
    }
    

#if defined(_LARGE_WORLDS)
    if (!m_bUnloaded)  
                       
    {
        m_bUnloaded = true;
        if (!level->isClientSide) {
            delete m_unloadedEntitiesTag;
            
            
            m_unloadedEntitiesTag = new CompoundTag();
            ListTag<CompoundTag>* entityTags = new ListTag<CompoundTag>();

            {
                std::lock_guard<std::recursive_mutex> lock(m_csEntities);
                for (int i = 0; i < ENTITY_BLOCKS_LENGTH; i++) {
                    auto itEnd = entityBlocks[i]->end();
                    for (std::vector<std::shared_ptr<Entity> >::iterator it =
                             entityBlocks[i]->begin();
                         it != itEnd; it++) {
                        std::shared_ptr<Entity> e = *it;
                        CompoundTag* teTag = new CompoundTag();
                        if (e->save(teTag)) {
                            entityTags->add(teTag);
                        }
                    }

                    
                    entityBlocks[i]->clear();
                }
            }

            m_unloadedEntitiesTag->put(L"Entities", entityTags);

            ListTag<CompoundTag>* tileEntityTags = new ListTag<CompoundTag>();

            auto itEnd = tileEntities.end();
            for (std::unordered_map<TilePos, std::shared_ptr<TileEntity>,
                                    TilePosKeyHash, TilePosKeyEq>::iterator it =
                     tileEntities.begin();
                 it != itEnd; it++) {
                std::shared_ptr<TileEntity> te = it->second;
                CompoundTag* teTag = new CompoundTag();
                te->save(teTag);
                tileEntityTags->add(teTag);
            }
            
            tileEntities.clear();

            m_unloadedEntitiesTag->put(L"TileEntities", tileEntityTags);
        }
    }
#endif
}

bool LevelChunk::containsPlayer() {
    {
        std::lock_guard<std::recursive_mutex> lock(m_csEntities);
        for (int i = 0; i < ENTITY_BLOCKS_LENGTH; i++) {
            std::vector<std::shared_ptr<Entity> >* vecEntity = entityBlocks[i];
            for (int j = 0; j < vecEntity->size(); j++) {
                if (vecEntity->at(j)->GetType() == eTYPE_SERVERPLAYER) {
                    return true;
                }
            }
        }
    }
    return false;
}

#if defined(_LARGE_WORLDS)
bool LevelChunk::isUnloaded() { return m_bUnloaded; }
#endif

void LevelChunk::markUnsaved() { this->setUnsaved(true); }

void LevelChunk::getEntities(std::shared_ptr<Entity> except, AABB* bb,
                             std::vector<std::shared_ptr<Entity> >& es,
                             const EntitySelector* selector) {
    int yc0 = Mth::floor((bb->y0 - 2) / 16);
    int yc1 = Mth::floor((bb->y1 + 2) / 16);
    if (yc0 < 0) yc0 = 0;
    if (yc1 >= ENTITY_BLOCKS_LENGTH) yc1 = ENTITY_BLOCKS_LENGTH - 1;

    
    
    {
        std::lock_guard<std::recursive_mutex> lock(m_csEntities);
        for (int yc = yc0; yc <= yc1; yc++) {
            std::vector<std::shared_ptr<Entity> >* entities = entityBlocks[yc];

            auto itEnd = entities->end();
            for (auto it = entities->begin(); it != itEnd; it++) {
                std::shared_ptr<Entity> e = *it;  
                if (e != except && e->bb.intersects(*bb) &&
                    (selector == nullptr || selector->matches(e))) {
                    es.push_back(e);
                    std::vector<std::shared_ptr<Entity> >* subs =
                        e->getSubEntities();
                    if (subs != nullptr) {
                        for (int j = 0; j < subs->size(); j++) {
                            e = subs->at(j);
                            if (e != except && e->bb.intersects(*bb) &&
                                (selector == nullptr || selector->matches(e))) {
                                es.push_back(e);
                            }
                        }
                    }
                }
            }
        }
    }
}

void LevelChunk::getEntitiesOfClass(const std::type_info& ec, AABB* bb,
                                    std::vector<std::shared_ptr<Entity> >& es,
                                    const EntitySelector* selector) {
    int yc0 = Mth::floor((bb->y0 - 2) / 16);
    int yc1 = Mth::floor((bb->y1 + 2) / 16);

    if (yc0 < 0) {
        yc0 = 0;
    } else if (yc0 >= ENTITY_BLOCKS_LENGTH) {
        yc0 = ENTITY_BLOCKS_LENGTH - 1;
    }
    if (yc1 >= ENTITY_BLOCKS_LENGTH) {
        yc1 = ENTITY_BLOCKS_LENGTH - 1;
    } else if (yc1 < 0) {
        yc1 = 0;
    }

    
    
    {
        std::lock_guard<std::recursive_mutex> lock(m_csEntities);
        for (int yc = yc0; yc <= yc1; yc++) {
            std::vector<std::shared_ptr<Entity> >* entities = entityBlocks[yc];

            auto itEnd = entities->end();
            for (auto it = entities->begin(); it != itEnd; it++) {
                std::shared_ptr<Entity> e = *it;  

                bool isAssignableFrom = false;
                
                
                
                if (ec == typeid(Player))
                    isAssignableFrom = e->instanceof(eTYPE_PLAYER);
                else if (ec == typeid(Entity))
                    isAssignableFrom = e->instanceof(eTYPE_ENTITY);
                else if (ec == typeid(Mob))
                    isAssignableFrom = e->instanceof(eTYPE_MOB);
                else if (ec == typeid(LivingEntity))
                    isAssignableFrom = e->instanceof(eTYPE_LIVINGENTITY);
                else if (ec == typeid(ItemEntity))
                    isAssignableFrom = e->instanceof(eTYPE_ITEMENTITY);
                else if (ec == typeid(Minecart))
                    isAssignableFrom = e->instanceof(eTYPE_MINECART);
                else if (ec == typeid(Monster))
                    isAssignableFrom = e->instanceof(eTYPE_MONSTER);
                else if (ec == typeid(Zombie))
                    isAssignableFrom = e->instanceof(eTYPE_ZOMBIE);
                else if (Entity* entity = e.get();
                         entity != nullptr && ec == typeid(*entity))
                    isAssignableFrom = true;
                if (isAssignableFrom && e->bb.intersects(*bb)) {
                    if (selector == nullptr || selector->matches(e)) {
                        es.push_back(e);
                    }
                }
                
                
            }
        }
    }
}

int LevelChunk::countEntities() {
    int entityCount = 0;
    {
        std::lock_guard<std::recursive_mutex> lock(m_csEntities);
        for (int yc = 0; yc < ENTITY_BLOCKS_LENGTH; yc++) {
            entityCount += (int)entityBlocks[yc]->size();
        }
    }
    return entityCount;
}

bool LevelChunk::shouldSave(bool force) {
    if (dontSave) return false;
    if (force) {
        if ((lastSaveHadEntities && level->getGameTime() != lastSaveTime) ||
            m_unsaved) {
            return true;
        }
    } else {
        if (lastSaveHadEntities &&
            level->getGameTime() >= lastSaveTime + 20 * 30)
            return true;
    }

    return m_unsaved;
}

int LevelChunk::getBlocksAndData(std::vector<uint8_t>* data, int x0, int y0,
                                 int z0, int x1, int y1, int z1, int p,
                                 bool includeLighting ) {
    int xs = x1 - x0;
    int ys = y1 - y0;
    int zs = z1 - z0;

    
    
    int compressedHeight = Level::COMPRESSED_CHUNK_SECTION_HEIGHT;

    
    if (y0 < Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
        p += lowerBlocks->getDataRegion(*data, x0, y0, z0, x1,
                                        std::min(compressedHeight, y1), z1, p);
    if (y1 > Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
        p += upperBlocks->getDataRegion(
            *data, x0, std::max(y0 - compressedHeight, 0), z0, x1,
            y1 - Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z1, p);

    
    if (y0 < Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
        p += lowerData->getDataRegion(*data, x0, y0, z0, x1,
                                      std::min(compressedHeight, y1), z1, p);
    if (y1 > Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
        p += upperData->getDataRegion(
            *data, x0, std::max(y0 - compressedHeight, 0), z0, x1,
            y1 - Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z1, p);

    if (includeLighting) {
        
        
        if (y0 < Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
            p += lowerBlockLight->getDataRegion(
                *data, x0, y0, z0, x1, std::min(compressedHeight, y1), z1, p);
        if (y1 > Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
            p += upperBlockLight->getDataRegion(
                *data, x0, std::max(y0 - compressedHeight, 0), z0, x1,
                y1 - Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z1, p);

        if (y0 < Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
            p += lowerSkyLight->getDataRegion(
                *data, x0, y0, z0, x1, std::min(compressedHeight, y1), z1, p);
        if (y1 > Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
            p += upperSkyLight->getDataRegion(
                *data, x0, std::max(y0 - compressedHeight, 0), z0, x1,
                y1 - Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z1, p);
    }

    

















    return p;
}


bool LevelChunk::testSetBlocksAndData(std::vector<uint8_t>& data, int x0,
                                      int y0, int z0, int x1, int y1, int z1,
                                      int p) {
    bool changed = false;

    
    
    int compressedHeight = Level::COMPRESSED_CHUNK_SECTION_HEIGHT;

    if (y0 < Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
        changed = lowerBlocks->testSetDataRegion(
            data, x0, y0, z0, x1, std::min(compressedHeight, y1), z1, p);
    if (y1 > Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
        changed =
            changed || upperBlocks->testSetDataRegion(
                           data, x0, std::max(y0 - compressedHeight, 0), z0, x1,
                           y1 - Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z1, p);

    return changed;
}

void LevelChunk::tileUpdatedCallback(int x, int y, int z, void* param,
                                     int yparam) {
    LevelChunk* lc = (LevelChunk*)param;
    int xx = lc->x * 16 + x;
    int yy = y + yparam;
    int zz = lc->z * 16 + z;
    lc->level->checkLight(xx, yy, zz);
}

int LevelChunk::setBlocksAndData(std::vector<uint8_t>& data, int x0, int y0,
                                 int z0, int x1, int y1, int z1, int p,
                                 bool includeLighting ) {
    
    
    
    
    
    
    if (includeLighting) {
        GameRenderer::AddForDelete(lowerBlocks);
        std::vector<uint8_t> emptyByteArray;
        lowerBlocks = new CompressedTileStorage(emptyByteArray, 0);
        GameRenderer::FinishedReassigning();

        GameRenderer::AddForDelete(lowerSkyLight);
        lowerSkyLight = new SparseLightStorage(true, false);
        GameRenderer::FinishedReassigning();

        GameRenderer::AddForDelete(lowerBlockLight);
        lowerBlockLight = new SparseLightStorage(false, false);
        GameRenderer::FinishedReassigning();

        GameRenderer::AddForDelete(lowerData);
        lowerData = new SparseDataStorage(false);
        GameRenderer::FinishedReassigning();
    }

    
    
    int compressedHeight = Level::COMPRESSED_CHUNK_SECTION_HEIGHT;

    
    if (y0 < Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
        p += lowerBlocks->setDataRegion(
            data, x0, y0, z0, x1, std::min(compressedHeight, y1), z1, p,
            includeLighting ? nullptr : tileUpdatedCallback, this, 0);
    if (y1 > Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
        p += upperBlocks->setDataRegion(
            data, x0, std::max(y0 - compressedHeight, 0), z0, x1,
            y1 - Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z1, p,
            includeLighting ? nullptr : tileUpdatedCallback, this,
            Level::COMPRESSED_CHUNK_SECTION_HEIGHT);
    









    recalcHeightmapOnly();

    
    if (y0 < Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
        p += lowerData->setDataRegion(
            data, x0, y0, z0, x1, std::min(compressedHeight, y1), z1, p,
            includeLighting ? nullptr : tileUpdatedCallback, this, 0);
    if (y1 > Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
        p += upperData->setDataRegion(
            data, x0, std::max(y0 - compressedHeight, 0), z0, x1,
            y1 - Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z1, p,
            includeLighting ? nullptr : tileUpdatedCallback, this,
            Level::COMPRESSED_CHUNK_SECTION_HEIGHT);

    if (includeLighting) {
        
        
        if (y0 < Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
            p += lowerBlockLight->setDataRegion(
                data, x0, y0, z0, x1, std::min(compressedHeight, y1), z1, p);
        if (y1 > Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
            p += upperBlockLight->setDataRegion(
                data, x0, std::max(y0 - compressedHeight, 0), z0, x1,
                y1 - Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z1, p);

        if (y0 < Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
            p += lowerSkyLight->setDataRegion(
                data, x0, y0, z0, x1, std::min(compressedHeight, y1), z1, p);
        if (y1 > Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
            p += upperSkyLight->setDataRegion(
                data, x0, std::max(y0 - compressedHeight, 0), z0, x1,
                y1 - Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z1, p);

        memcpy(biomes.data(), &data.data()[p], biomes.size());
        p += biomes.size();
    } else {
        
        
        
        
        
        
        
        if (level->isClientSide && g_NetworkManager.IsHost()) {
            reSyncLighting();
            level->getChunk(x - 1, z - 1)->reSyncLighting();
            level->getChunk(x - 0, z - 1)->reSyncLighting();
            level->getChunk(x + 1, z - 1)->reSyncLighting();
            level->getChunk(x - 1, z + 0)->reSyncLighting();
            level->getChunk(x + 1, z + 0)->reSyncLighting();
            level->getChunk(x - 1, z + 1)->reSyncLighting();
            level->getChunk(x + 0, z + 1)->reSyncLighting();
            level->getChunk(x + 1, z + 1)->reSyncLighting();
        }
    }

    

















    for (auto it = tileEntities.begin(); it != tileEntities.end(); ++it) {
        it->second->clearCache();
    }
    

    
    
    if (includeLighting) {
        compressLighting();
        compressBlocks();
        compressData();
    }

    return p;
}

void LevelChunk::setCheckAllLight() { checkLightPosition = 0; }

Random* LevelChunk::getRandom(int64_t l) {
    return new Random((level->getSeed() + x * x * 4987142 + x * 5947611 +
                       z * z * 4392871l + z * 389711) ^
                      l);
}

bool LevelChunk::isEmpty() { return false; }
void LevelChunk::attemptCompression() {
    
}

void LevelChunk::checkPostProcess(ChunkSource* source, ChunkSource* parent,
                                  int x, int z) {
    if (((terrainPopulated & sTerrainPopulatedFromHere) == 0) &&
        source->hasChunk(x + 1, z + 1) && source->hasChunk(x, z + 1) &&
        source->hasChunk(x + 1, z)) {
        source->postProcess(parent, x, z);
    }
    if (source->hasChunk(x - 1, z) &&
        ((source->getChunk(x - 1, z)->terrainPopulated &
          sTerrainPopulatedFromHere) == 0) &&
        source->hasChunk(x - 1, z + 1) && source->hasChunk(x, z + 1) &&
        source->hasChunk(x - 1, z + 1)) {
        source->postProcess(parent, x - 1, z);
    }
    if (source->hasChunk(x, z - 1) &&
        ((source->getChunk(x, z - 1)->terrainPopulated &
          sTerrainPopulatedFromHere) == 0) &&
        source->hasChunk(x + 1, z - 1) && source->hasChunk(x + 1, z)) {
        source->postProcess(parent, x, z - 1);
    }
    if (source->hasChunk(x - 1, z - 1) &&
        ((source->getChunk(x - 1, z - 1)->terrainPopulated &
          sTerrainPopulatedFromHere) == 0) &&
        source->hasChunk(x, z - 1) && source->hasChunk(x - 1, z)) {
        source->postProcess(parent, x - 1, z - 1);
    }
}




void LevelChunk::checkChests(ChunkSource* source, int x, int z) {
    LevelChunk* lc = source->getChunk(x, z);

    for (int xx = 0; xx < 16; xx++)
        for (int zz = 0; zz < 16; zz++)
            for (int yy = 0; yy < 128; yy++) {
                if (lc->getTile(xx, yy, zz) == Tile::chest_Id) {
                    if (lc->getData(xx, yy, zz) == 0) {
                        int xOffs = x * 16 + xx;
                        int zOffs = z * 16 + zz;
                        ChestTile* tile =
                            (ChestTile*)Tile::tiles[Tile::chest_Id];
                        tile->recalcLockDir(level, xOffs, yy, zOffs);
                        level->checkLight(xOffs, yy, zOffs, true);
                    }
                }
            }
}


void LevelChunk::tick() {
    if (hasGapsToCheck && !level->dimension->hasCeiling) recheckGaps();
}

ChunkPos* LevelChunk::getPos() { return new ChunkPos(x, z); }

bool LevelChunk::isYSpaceEmpty(int y1, int y2) {
    return false;
    
    












}


void LevelChunk::reloadBiomes() {
    BiomeSource* biomeSource = level->dimension->biomeSource;
    for (unsigned int x = 0; x < 16; ++x) {
        for (unsigned int z = 0; z < 16; ++z) {
            Biome* biome =
                biomeSource->getBiome((this->x << 4) + x, (this->z << 4) + z);
            biomes[(z << 4) | x] = (uint8_t)((biome->id) & 0xff);
        }
    }
}

Biome* LevelChunk::getBiome(int x, int z, BiomeSource* biomeSource) {
    int value = biomes[((unsigned)z << 4) | x] & 0xff;
    if (value == 0xff) {
        
        Biome* biome = biomeSource->getBiome(((unsigned)this->x << 4) + x,
                                             ((unsigned)this->z << 4) + z);
        value = biome->id;
        biomes[((unsigned)z << 4) | x] = (uint8_t)(value & 0xff);
    }
    if (Biome::biomes[value] == nullptr) {
        return Biome::plains;
    }
    return Biome::biomes[value];
}

std::vector<uint8_t> LevelChunk::getBiomes() { return biomes; }

void LevelChunk::setBiomes(std::vector<uint8_t>& biomes) {
    this->biomes = biomes;
}


int LevelChunk::getTopRainBlock(int x, int z) {
    int slot = x | ((unsigned)z << 4);
    int h = rainHeights[slot];

    if (h == 255) {
        int y = Level::maxBuildHeight - 1;
        h = -1;
        while (y > 0 && h == -1) {
            int t = getTile(x, y, z);
            Material* m = t == 0 ? Material::air : Tile::tiles[t]->material;
            if (!m->blocksMotion() && !m->isLiquid()) {
                y--;
            } else {
                h = y + 1;
            }
        }
        
        
        
        
        if (h == 255) h = 254;
        rainHeights[slot] = h;
    }

    return h;
}



bool LevelChunk::biomeHasRain(int x, int z) {
    updateBiomeFlags(x, z);
    int slot = (x >> 1) | (z * 8);
    int shift = (x & 1) * 4;
    return ((columnFlags[slot] & (eColumnFlag_biomeHasRain << shift)) != 0);
}



bool LevelChunk::biomeHasSnow(int x, int z) {
    updateBiomeFlags(x, z);
    int slot = (x >> 1) | (z * 8);
    int shift = (x & 1) * 4;
    return ((columnFlags[slot] & (eColumnFlag_biomeHasSnow << shift)) != 0);
}

void LevelChunk::updateBiomeFlags(int x, int z) {
    int slot = (x >> 1) | (z * 8);
    int shift = (x & 1) * 4;
    if ((columnFlags[slot] & (eColumnFlag_biomeOk << shift)) == 0) {
        int xOffs = (this->x * 16) + x;
        int zOffs = (this->z * 16) + z;
        std::vector<Biome*> biomes;
        level->getBiomeSource()->getBiomeBlock(biomes, xOffs, zOffs, 1, 1,
                                               true);
        if (biomes[0]->hasRain())
            columnFlags[slot] |= (eColumnFlag_biomeHasRain << shift);
        if (biomes[0]->hasSnow())
            columnFlags[slot] |= (eColumnFlag_biomeHasSnow << shift);
        columnFlags[slot] |= (eColumnFlag_biomeOk << shift);
    }
}



void LevelChunk::getDataData(std::vector<uint8_t>& data) {
    lowerData->getData(data, 0);
    if (data.size() > Level::COMPRESSED_CHUNK_SECTION_TILES / 2)
        upperData->getData(data, Level::COMPRESSED_CHUNK_SECTION_TILES / 2);
}



void LevelChunk::setDataData(std::vector<uint8_t>& data) {
    if (lowerData == nullptr) lowerData = new SparseDataStorage();
    if (upperData == nullptr) upperData = new SparseDataStorage(true);
    lowerData->setData(data, 0);
    if (data.size() > Level::COMPRESSED_CHUNK_SECTION_TILES / 2)
        upperData->setData(data, Level::COMPRESSED_CHUNK_SECTION_TILES / 2);
}



void LevelChunk::getSkyLightData(std::vector<uint8_t>& data) {
    lowerSkyLight->getData(data, 0);
    if (data.size() > Level::COMPRESSED_CHUNK_SECTION_TILES / 2)
        upperSkyLight->getData(data, Level::COMPRESSED_CHUNK_SECTION_TILES / 2);
}



void LevelChunk::getBlockLightData(std::vector<uint8_t>& data) {
    lowerBlockLight->getData(data, 0);
    if (data.size() > Level::COMPRESSED_CHUNK_SECTION_TILES / 2)
        upperBlockLight->getData(data,
                                 Level::COMPRESSED_CHUNK_SECTION_TILES / 2);
}



void LevelChunk::setSkyLightData(std::vector<uint8_t>& data) {
    if (lowerSkyLight == nullptr) lowerSkyLight = new SparseLightStorage(true);
    if (upperSkyLight == nullptr)
        upperSkyLight = new SparseLightStorage(true, true);
    lowerSkyLight->setData(data, 0);
    if (data.size() > Level::COMPRESSED_CHUNK_SECTION_TILES / 2)
        upperSkyLight->setData(data, Level::COMPRESSED_CHUNK_SECTION_TILES / 2);
}



void LevelChunk::setBlockLightData(std::vector<uint8_t>& data) {
    if (lowerBlockLight == nullptr)
        lowerBlockLight = new SparseLightStorage(false);
    if (upperBlockLight == nullptr)
        upperBlockLight = new SparseLightStorage(false, true);
    lowerBlockLight->setData(data, 0);
    if (data.size() > Level::COMPRESSED_CHUNK_SECTION_TILES / 2)
        upperBlockLight->setData(data,
                                 Level::COMPRESSED_CHUNK_SECTION_TILES / 2);
}


void LevelChunk::setSkyLightDataAllBright() {
    lowerSkyLight->setAllBright();
    upperSkyLight->setAllBright();
}




void LevelChunk::compressLighting() {
    
    
    
    
    
    lowerSkyLight->compress();
    upperSkyLight->compress();
    lowerBlockLight->compress();
    upperBlockLight->compress();
}

void LevelChunk::compressBlocks() {
#if defined(SHARING_ENABLED)
    CompressedTileStorage* blocksToCompressLower = nullptr;
    CompressedTileStorage* blocksToCompressUpper = nullptr;

    
    
    
    
    
    
    if (level->isClientSide && g_NetworkManager.IsHost()) {
        
        
        
        {
            std::lock_guard<std::recursive_mutex> lock(m_csSharing);
            if (sharingTilesAndData) {
                blocksToCompressLower = lowerBlocks;
                blocksToCompressUpper = upperBlocks;
            }
        }
    } else {
        
        blocksToCompressLower = lowerBlocks;
        blocksToCompressUpper = upperBlocks;
    }

    
    if (blocksToCompressLower) blocksToCompressLower->compress();
    if (blocksToCompressUpper) blocksToCompressUpper->compress();
#else
    blocks->compress();
#endif
}

bool LevelChunk::isLowerBlockStorageCompressed() {
    return lowerBlocks->isCompressed();
}

int LevelChunk::isLowerBlockLightStorageCompressed() {
    return lowerBlockLight->isCompressed();
}

int LevelChunk::isLowerDataStorageCompressed() {
    return lowerData->isCompressed();
}

void LevelChunk::writeCompressedBlockData(DataOutputStream* dos) {
    lowerBlocks->write(dos);
    upperBlocks->write(dos);
}

void LevelChunk::writeCompressedDataData(DataOutputStream* dos) {
    lowerData->write(dos);
    upperData->write(dos);
}

void LevelChunk::writeCompressedSkyLightData(DataOutputStream* dos) {
    lowerSkyLight->write(dos);
    upperSkyLight->write(dos);
}

void LevelChunk::writeCompressedBlockLightData(DataOutputStream* dos) {
    lowerBlockLight->write(dos);
    upperBlockLight->write(dos);
}

void LevelChunk::readCompressedBlockData(DataInputStream* dis) {
    lowerBlocks->read(dis);
    upperBlocks->read(dis);
}

void LevelChunk::readCompressedDataData(DataInputStream* dis) {
    if (lowerData == nullptr) lowerData = new SparseDataStorage();
    if (upperData == nullptr) upperData = new SparseDataStorage(true);
    lowerData->read(dis);
    upperData->read(dis);
}

void LevelChunk::readCompressedSkyLightData(DataInputStream* dis) {
    if (lowerSkyLight == nullptr) lowerSkyLight = new SparseLightStorage(true);
    if (upperSkyLight == nullptr)
        upperSkyLight = new SparseLightStorage(true, true);
    lowerSkyLight->read(dis);
    upperSkyLight->read(dis);
}

void LevelChunk::readCompressedBlockLightData(DataInputStream* dis) {
    if (lowerBlockLight == nullptr)
        lowerBlockLight = new SparseLightStorage(false);
    if (upperBlockLight == nullptr)
        upperBlockLight = new SparseLightStorage(false, true);
    lowerBlockLight->read(dis);
    upperBlockLight->read(dis);
}




void LevelChunk::compressData() {
#if defined(SHARING_ENABLED)
    SparseDataStorage* dataToCompressLower = nullptr;
    SparseDataStorage* dataToCompressUpper = nullptr;

    
    
    
    
    
    
    if (level->isClientSide && g_NetworkManager.IsHost()) {
        
        
        
        {
            std::lock_guard<std::recursive_mutex> lock(m_csSharing);
            if (sharingTilesAndData) {
                dataToCompressLower = lowerData;
                dataToCompressUpper = upperData;
            }
        }
    } else {
        
        dataToCompressLower = lowerData;
        dataToCompressUpper = upperData;
    }

    
    if (dataToCompressLower) dataToCompressLower->compress();
    if (dataToCompressUpper) dataToCompressUpper->compress();
#else
    data->compress();
#endif
}

bool LevelChunk::isRenderChunkEmpty(int y) {
    if (isEmpty()) {
        return true;
    }
    if (y >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT) {
        return upperBlocks->isRenderChunkEmpty(
            y - Level::COMPRESSED_CHUNK_SECTION_HEIGHT);
    } else {
        return lowerBlocks->isRenderChunkEmpty(y);
    }
}


void LevelChunk::setBlockData(std::vector<uint8_t>& data) {
    lowerBlocks->setData(data, 0);
    if (data.size() > Level::COMPRESSED_CHUNK_SECTION_TILES)
        upperBlocks->setData(data, Level::COMPRESSED_CHUNK_SECTION_TILES);
}


void LevelChunk::getBlockData(std::vector<uint8_t>& data) {
    lowerBlocks->getData(data, 0);
    if (data.size() > Level::COMPRESSED_CHUNK_SECTION_TILES)
        upperBlocks->getData(data, Level::COMPRESSED_CHUNK_SECTION_TILES);
}

int LevelChunk::getBlocksAllocatedSize(int* count0, int* count1, int* count2,
                                       int* count4, int* count8) {
    return lowerBlocks->getAllocatedSize(count0, count1, count2, count4,
                                         count8);
}

int LevelChunk::getHighestNonEmptyY() {
    int highestNonEmptyY = -1;
    if (upperBlocks) {
        int upperNonEmpty = upperBlocks->getHighestNonEmptyY();
        if (upperNonEmpty >= 0) {
            highestNonEmptyY =
                upperNonEmpty + Level::COMPRESSED_CHUNK_SECTION_HEIGHT;
        }
    }
    if (highestNonEmptyY < 0)
        highestNonEmptyY = lowerBlocks->getHighestNonEmptyY();
    if (highestNonEmptyY < 0) highestNonEmptyY = 0;

    return highestNonEmptyY;
}

std::vector<uint8_t> LevelChunk::getReorderedBlocksAndData(int x0, int y0,
                                                           int z0, int xs,
                                                           int& ys, int zs) {
    ys = std::min(Level::maxBuildHeight - y0, ys);

    int x1 = x0 + xs;
    int y1 = y0 + ys;
    int z1 = z0 + zs;

    unsigned int tileCount = xs * ys * zs;
    unsigned int halfTileCount = tileCount / 2;

    std::vector<uint8_t> data =
        std::vector<uint8_t>(tileCount + (3 * halfTileCount) + biomes.size());
    for (int x = 0; x < xs; x++) {
        for (int z = 0; z < zs; z++) {
            for (int y = 0; y < ys; y++) {
                int slot = (y * xs * zs) + (z * xs) + x;

                data[slot] = getTile(x, y, z);
            }
        }
    }

    int p = tileCount;

    
    
    int compressedHeight = Level::COMPRESSED_CHUNK_SECTION_HEIGHT;

    
    if (y0 < Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
        p += lowerData->getDataRegion(data, x0, y0, z0, x1,
                                      std::min(compressedHeight, y1), z1, p);
    if (y1 > Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
        p += upperData->getDataRegion(
            data, x0, std::max(y0 - compressedHeight, 0), z0, x1,
            y1 - Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z1, p);

    
    
    if (y0 < Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
        p += lowerBlockLight->getDataRegion(
            data, x0, y0, z0, x1, std::min(compressedHeight, y1), z1, p);
    if (y1 > Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
        p += upperBlockLight->getDataRegion(
            data, x0, std::max(y0 - compressedHeight, 0), z0, x1,
            y1 - Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z1, p);

    if (y0 < Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
        p += lowerSkyLight->getDataRegion(
            data, x0, y0, z0, x1, std::min(compressedHeight, y1), z1, p);
    if (y1 > Level::COMPRESSED_CHUNK_SECTION_HEIGHT)
        p += upperSkyLight->getDataRegion(
            data, x0, std::max(y0 - compressedHeight, 0), z0, x1,
            y1 - Level::COMPRESSED_CHUNK_SECTION_HEIGHT, z1, p);

    memcpy(&data.data()[p], biomes.data(), biomes.size());

    return data;

    
    
    
    
    
    
    
    
    

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
}

void LevelChunk::reorderBlocksAndDataToXZY(int y0, int xs, int ys, int zs,
                                           std::vector<uint8_t>* data) {
    int y1 = y0 + ys;
    unsigned int tileCount = xs * ys * zs;
    unsigned int halfTileCount = tileCount / 2;

    int sectionHeight = Level::COMPRESSED_CHUNK_SECTION_HEIGHT;
    int lowerYSpan = std::min(y1, sectionHeight) - y0;
    int upperYSpan = ys - lowerYSpan;
    int upperSlotOffset = xs * zs * lowerYSpan;

    int biomesLength = 16 * 16;
    std::vector<uint8_t> newBuffer =
        std::vector<uint8_t>(tileCount + (3 * halfTileCount) + biomesLength);
    for (int x = 0; x < xs; x++) {
        for (int z = 0; z < zs; z++) {
            for (int y = 0; y < ys; y++) {
                int slotY = y;
                unsigned int targetSlotOffset = 0;
                int ySpan = lowerYSpan;
                if (y >= Level::COMPRESSED_CHUNK_SECTION_HEIGHT) {
                    slotY -= Level::COMPRESSED_CHUNK_SECTION_HEIGHT;
                    targetSlotOffset = upperSlotOffset;
                    ySpan = upperYSpan;
                }
                int slot = (x * zs * ySpan) + (z * ySpan) + slotY;
                int slot2 = (y * xs * zs) + (z * xs) + x;

                newBuffer[slot + targetSlotOffset] = (*data)[slot2];
            }
        }
    }
    
    memcpy(newBuffer.data() + tileCount, data->data() + tileCount,
           3 * halfTileCount + biomesLength);
    *data = std::move(newBuffer);

    
    

    
    
    
    
    
    
    
    
    
    
    
    
    
    

    
    
    
    
    
    
    
    
    
}
