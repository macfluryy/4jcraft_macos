#include "Level.h"

#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <mutex>
#include <numbers>
#include <optional>

#include "Explosion.h"
#include "IPlatformInput.h"
#include "LevelListener.h"
#include "app/common/App_enums.h"
#include "app/common/src/Colours/ColourTable.h"
#include "app/common/src/Console_Debug_enum.h"
#include "app/common/src/Network/GameNetworkManager.h"
#include "app/mac/MacGame.h"
#include "app/mac/Stubs/winapi_stubs.h"
#include "app/include/FrameProfiler.h"
#include "java/Random.h"
#include "minecraft/Direction.h"
#include "minecraft/Facing.h"
#include "minecraft/Pos.h"
#include "minecraft/SharedConstants.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/renderer/LevelRenderer.h"
#include "minecraft/core/particles/ParticleTypes.h"
#include "minecraft/sounds/SoundTypes.h"
#include "minecraft/stats/GenericStats.h"
#include "minecraft/util/Mth.h"
#include "minecraft/world/Difficulty.h"
#include "minecraft/world/entity/Entity.h"
#include "minecraft/world/entity/MobCategory.h"
#include "minecraft/world/entity/ai/village/VillageSiege.h"
#include "minecraft/world/entity/ai/village/Villages.h"
#include "minecraft/world/entity/player/Abilities.h"
#include "minecraft/world/entity/player/Player.h"
#include "minecraft/world/level/ChunkPos.h"
#include "minecraft/world/level/LevelType.h"
#include "minecraft/world/level/LightLayer.h"
#include "minecraft/world/level/Region.h"
#include "minecraft/world/level/biome/Biome.h"
#include "minecraft/world/level/biome/BiomeSource.h"
#include "minecraft/world/level/chunk/ChunkSource.h"
#include "minecraft/world/level/chunk/LevelChunk.h"
#include "minecraft/world/level/dimension/Dimension.h"
#include "minecraft/world/level/material/Material.h"
#include "minecraft/world/level/pathfinder/PathFinder.h"
#include "minecraft/world/level/redstone/Redstone.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/ConsoleSaveFile.h"
#include "minecraft/world/level/storage/LevelData.h"
#include "minecraft/world/level/storage/LevelStorage.h"
#include "minecraft/world/level/storage/SavedDataStorage.h"
#include "minecraft/world/level/tile/ComparatorTile.h"
#include "minecraft/world/level/tile/FireTile.h"
#include "minecraft/world/level/tile/HalfSlabTile.h"
#include "minecraft/world/level/tile/HopperTile.h"
#include "minecraft/world/level/tile/LevelEvent.h"
#include "minecraft/world/level/tile/LiquidTile.h"
#include "minecraft/world/level/tile/NotGateTile.h"
#include "minecraft/world/level/tile/StairTile.h"
#include "minecraft/world/level/tile/Tile.h"
#include "minecraft/world/level/tile/TopSnowTile.h"
#include "minecraft/world/level/tile/entity/TileEntity.h"
#include "minecraft/world/phys/AABB.h"
#include "minecraft/world/phys/HitResult.h"
#include "minecraft/world/phys/Vec3.h"
#include "minecraft/world/scores/Scoreboard.h"
#include "platform/PlatformServices.h"

class CompoundTag;
class ItemInstance;
class TickNextTickData;
class TilePos;













thread_local bool Level::m_tlsInstaTick = false;
thread_local Level::lightCache_t* Level::m_tlsLightCache = nullptr;

void Level::enableLightingCache() {
    
    
    
    
    m_tlsLightCache = (lightCache_t*)malloc(256 * 1024);
}

void Level::destroyLightingCache() { delete m_tlsLightCache; }

inline int GetIndex(int x, int y, int z) {
    return ((x & 15) << 8) | ((y & 15) << 4) | (z & 15);
}

void Level::initCachePartial(lightCache_t* cache, int xc, int yc, int zc) {
    cachewritten = false;
    if (cache == nullptr) return;

    int idx;
    if (!(yc & 0xffffff00)) {
        idx = GetIndex(xc, yc, zc);
        cache[idx] = 0;
        idx = GetIndex(xc - 1, yc, zc);
        cache[idx] = 0;
        idx = GetIndex(xc + 1, yc, zc);
        cache[idx] = 0;
        idx = GetIndex(xc, yc, zc - 1);
        cache[idx] = 0;
        idx = GetIndex(xc, yc, zc + 1);
        cache[idx] = 0;
    }
    if (!((yc - 1) & 0xffffff00)) {
        idx = GetIndex(xc, yc - 1, zc);
        cache[idx] = 0;
    }
    if (!((yc + 1) & 0xffffff00)) {
        idx = GetIndex(xc, yc + 1, zc);
        cache[idx] = 0;
    }
}

void Level::initCacheComplete(lightCache_t* cache, int xc, int yc, int zc) {
    lightCache_t old[7];
    if (!(yc & 0xffffff00)) {
        old[0] = cache[GetIndex(xc, yc, zc)];
        old[1] = cache[GetIndex(xc - 1, yc, zc)];
        old[2] = cache[GetIndex(xc + 1, yc, zc)];
        old[5] = cache[GetIndex(xc, yc, zc - 1)];
        old[6] = cache[GetIndex(xc, yc, zc + 1)];
    }
    if (!((yc - 1) & 0xffffff00)) {
        old[3] = cache[GetIndex(xc, yc - 1, zc)];
    }
    if (!((yc + 1) & 0xffffff00)) {
        old[4] = cache[GetIndex(xc, yc + 1, zc)];
    }

    memset(cache, 0, 16 * 16 * 16 * sizeof(lightCache_t));

    if (!(yc & 0xffffff00)) {
        cache[GetIndex(xc, yc, zc)] = old[0];
        cache[GetIndex(xc - 1, yc, zc)] = old[1];
        cache[GetIndex(xc + 1, yc, zc)] = old[2];
        cache[GetIndex(xc, yc, zc - 1)] = old[5];
        cache[GetIndex(xc, yc, zc + 1)] = old[6];
    }
    if (!((yc - 1) & 0xffffff00)) {
        cache[GetIndex(xc, yc - 1, zc)] = old[3];
    }
    if (!((yc + 1) & 0xffffff00)) {
        cache[GetIndex(xc, yc + 1, zc)] = old[4];
    }
}


void inline Level::setBrightnessCached(lightCache_t* cache, uint64_t* cacheUse,
                                       LightLayer::variety layer, int x, int y,
                                       int z, int brightness) {
    if (cache == nullptr) {
        setBrightness(layer, x, y, z, brightness, true);
        return;
    }
    if (y & 0xffffff00) return;  

    int idx = ((x & 15) << 8) | ((y & 15) << 4) | (z & 15);
    lightCache_t posbits =
        ((x & 0x3f0) << 6) | ((y & 0x0f0) << 2) | ((z & 0x3f0) >> 4);
#if defined(_LARGE_WORLDS)
    
    posbits |= ((((uint64_t)x) & 0x3FFFC00L) << 38) |
               ((((uint64_t)z) & 0x3FFFC00L) << 22);
#endif

    lightCache_t cacheValue = cache[idx];

    
    if ((cacheValue & POSITION_MASK) != posbits) {
        
        if (cacheValue & LIGHTING_WRITEBACK) {
            
            int val = (cacheValue >> LIGHTING_SHIFT) & 15;
            int xx = ((cacheValue >> 6) & 0x3f0) | (x & 15);
#if defined(_LARGE_WORLDS)
            xx |= ((cacheValue >> 38) & 0x3FFFC00);
            xx = (xx << 6) >> 6;  
#else
            xx = (xx << 22) >> 22;  
#endif
            int yy = ((cacheValue >> 2) & 0x0f0) | (y & 15);
            int zz = ((cacheValue << 4) & 0x3f0) | (z & 15);
#if defined(_LARGE_WORLDS)
            zz |= ((cacheValue >> 22) & 0x3FFFC00);
            zz = (zz << 6) >> 6;  
#else
            zz = (zz << 22) >> 22;  
#endif
            setBrightness(layer, xx, yy, zz, val, true);
        }
        cacheValue = posbits;
    }

    
    cacheValue &= ~(15 << LIGHTING_SHIFT);
    cacheValue |= brightness << LIGHTING_SHIFT;
    cacheValue |= (LIGHTING_WRITEBACK | LIGHTING_VALID);

    
    
    (*cacheUse) |= ((1LL << (x & 15)) | (0x10000LL << (y & 15)) |
                    (0x100000000LL << (z & 15)));

    cache[idx] = cacheValue;
}


inline int Level::getBrightnessCached(lightCache_t* cache,
                                      LightLayer::variety layer, int x, int y,
                                      int z) {
    if (cache == nullptr) return getBrightness(layer, x, y, z);
    if (y & 0xffffff00)
        return getBrightness(
            layer, x, y,
            z);  

    int idx = ((x & 15) << 8) | ((y & 15) << 4) | (z & 15);
    lightCache_t posbits =
        ((x & 0x3f0) << 6) | ((y & 0x0f0) << 2) | ((z & 0x3f0) >> 4);
#if defined(_LARGE_WORLDS)
    
    posbits |= ((((uint64_t)x) & 0x3FFFC00L) << 38) |
               ((((uint64_t)z) & 0x3FFFC00L) << 22);
#endif

    lightCache_t cacheValue = cache[idx];

    if ((cacheValue & POSITION_MASK) != posbits) {
        
        if (cacheValue & LIGHTING_WRITEBACK) {
            
            int val = (cacheValue >> LIGHTING_SHIFT) & 15;
            int xx = ((cacheValue >> 6) & 0x3f0) | (x & 15);
#if defined(_LARGE_WORLDS)
            xx |= ((cacheValue >> 38) & 0x3FFFC00);
            xx = (xx << 6) >> 6;  
#else
            xx = (xx << 22) >> 22;  
#endif
            int yy = ((cacheValue >> 2) & 0x0f0) | (y & 15);
            int zz = ((cacheValue << 4) & 0x3f0) | (z & 15);
#if defined(_LARGE_WORLDS)
            zz |= ((cacheValue >> 22) & 0x3FFFC00);
            zz = (zz << 6) >> 6;  
#else
            zz = (zz << 22) >> 22;  
#endif
            setBrightness(layer, xx, yy, zz, val, true);
        }
        cacheValue = posbits | LIGHTING_VALID;
        int val = getBrightness(layer, x, y, z);
        cacheValue |= val << LIGHTING_SHIFT;
    } else {
        
        
        if ((cacheValue & LIGHTING_VALID) == 0) {
            int val = getBrightness(layer, x, y, z);
            cacheValue |= val << LIGHTING_SHIFT;
            cacheValue |= LIGHTING_VALID;
        } else {
            
            return (cacheValue >> LIGHTING_SHIFT) & 15;
        }
    }

    cache[idx] = cacheValue;
    return (cacheValue >> LIGHTING_SHIFT) & 15;
}



inline int Level::getEmissionCached(lightCache_t* cache, int ct, int x, int y,
                                    int z) {
    if (cache == nullptr) return Tile::lightEmission[ct];

    int idx = ((x & 15) << 8) | ((y & 15) << 4) | (z & 15);
    lightCache_t posbits =
        ((x & 0x3f0) << 6) | ((y & 0x0f0) << 2) | ((z & 0x3f0) >> 4);
#if defined(_LARGE_WORLDS)
    
    posbits |= ((((uint64_t)x) & 0x3FFFC00) << 38) |
               ((((uint64_t)z) & 0x3FFFC00) << 22);
#endif

    lightCache_t cacheValue = cache[idx];

    if ((cacheValue & POSITION_MASK) != posbits) {
        
        if (cacheValue & LIGHTING_WRITEBACK) {
            
            int val = (cacheValue >> LIGHTING_SHIFT) & 15;
            int xx = ((cacheValue >> 6) & 0x3f0) | (x & 15);
#if defined(_LARGE_WORLDS)
            xx |= ((cacheValue >> 38) & 0x3FFFC00);
            xx = (xx << 6) >> 6;  
#else
            xx = (xx << 22) >> 22;  
#endif
            int yy = ((cacheValue >> 2) & 0x0f0) | (y & 15);
            int zz = ((cacheValue << 4) & 0x3f0) | (z & 15);
#if defined(_LARGE_WORLDS)
            zz |= ((cacheValue >> 22) & 0x3FFFC00);
            zz = (zz << 6) >> 6;  
#else
            zz = (zz << 22) >> 22;  
#endif
            setBrightness(LightLayer::Block, xx, yy, zz, val, true);
        }

        
        cacheValue = posbits | EMISSION_VALID | BLOCKING_VALID;
        int t = getTile(x, y, z);
        cacheValue |= (Tile::lightEmission[t] & 15) << EMISSION_SHIFT;
        cacheValue |= (Tile::lightBlock[t] & 15) << BLOCKING_SHIFT;
    } else {
        
        
        if ((cacheValue & EMISSION_VALID) == 0) {
            
            cacheValue |= EMISSION_VALID | BLOCKING_VALID;
            int t = getTile(x, y, z);
            cacheValue |= (Tile::lightEmission[t] & 15) << EMISSION_SHIFT;
            cacheValue |= (Tile::lightBlock[t] & 15) << BLOCKING_SHIFT;
        } else {
            
            return (cacheValue >> EMISSION_SHIFT) & 15;
        }
    }
    cache[idx] = cacheValue;
    return (cacheValue >> EMISSION_SHIFT) & 15;
}



inline int Level::getBlockingCached(lightCache_t* cache,
                                    LightLayer::variety layer, int* ct, int x,
                                    int y, int z) {
    if (cache == nullptr) {
        int t = getTile(x, y, z);
        if (ct) *ct = t;
        return Tile::lightBlock[t];
    }

    int idx = ((x & 15) << 8) | ((y & 15) << 4) | (z & 15);
    lightCache_t posbits =
        ((x & 0x3f0) << 6) | ((y & 0x0f0) << 2) | ((z & 0x3f0) >> 4);
#if defined(_LARGE_WORLDS)
    
    posbits |= ((((uint64_t)x) & 0x3FFFC00L) << 38) |
               ((((uint64_t)z) & 0x3FFFC00L) << 22);
#endif

    lightCache_t cacheValue = cache[idx];

    if ((cacheValue & POSITION_MASK) != posbits) {
        
        if (cacheValue & LIGHTING_WRITEBACK) {
            
            int val = (cacheValue >> LIGHTING_SHIFT) & 15;
            int xx = ((cacheValue >> 6) & 0x3f0) | (x & 15);
#if defined(_LARGE_WORLDS)
            xx |= ((cacheValue >> 38) & 0x3FFFC00);
            xx = (xx << 6) >> 6;  
#else
            xx = (xx << 22) >> 22;  
#endif
            int yy = ((cacheValue >> 2) & 0x0f0) | (y & 15);
            int zz = ((cacheValue << 4) & 0x3f0) | (z & 15);
#if defined(_LARGE_WORLDS)
            zz |= ((cacheValue >> 22) & 0x3FFFC00);
            zz = (zz << 6) >> 6;  
#else
            zz = (zz << 22) >> 22;  
#endif
            setBrightness(layer, xx, yy, zz, val, true);
        }

        
        cacheValue = posbits | EMISSION_VALID | BLOCKING_VALID;
        int t = getTile(x, y, z);
        cacheValue |= (Tile::lightEmission[t] & 15) << EMISSION_SHIFT;
        cacheValue |= (Tile::lightBlock[t] & 15) << BLOCKING_SHIFT;
    } else {
        
        
        if ((cacheValue & EMISSION_VALID) == 0) {
            
            cacheValue |= EMISSION_VALID | BLOCKING_VALID;
            int t = getTile(x, y, z);
            cacheValue |= (Tile::lightEmission[t] & 15) << EMISSION_SHIFT;
            cacheValue |= (Tile::lightBlock[t] & 15) << BLOCKING_SHIFT;
        } else {
            
            return (cacheValue >> BLOCKING_SHIFT) & 15;
        }
    }

    cache[idx] = cacheValue;
    return (cacheValue >> BLOCKING_SHIFT) & 15;
}










void Level::flushCache(lightCache_t* cache, uint64_t cacheUse,
                       LightLayer::variety layer) {
    
    
    if (cacheUse == 0) return;
    if (cache) {
        lightCache_t* pcache = cache;
        for (int x = 0; x < 16; x++) {
            if ((cacheUse & (1LL << x)) == 0) {
                pcache += 16 * 16;
                continue;
            }
            for (int y = 0; y < 16; y++) {
                if ((cacheUse & (0x10000LL << y)) == 0) {
                    pcache += 16;
                    continue;
                }
                for (int z = 0; z < 16; z++) {
                    if ((cacheUse & (0x100000000LL << z)) == 0) {
                        pcache++;
                        continue;
                    }
                    lightCache_t cacheValue = *pcache++;
                    if (cacheValue & LIGHTING_WRITEBACK) {
                        int val = (cacheValue >> LIGHTING_SHIFT) & 15;
                        int xx = ((cacheValue >> 6) & 0x3f0) | (x & 15);
#if defined(_LARGE_WORLDS)
                        xx |= ((cacheValue >> 38) & 0x3FFFC00);
                        xx = (xx << 6) >> 6;  
#else
                        xx = (xx << 22) >> 22;  
#endif
                        int yy = ((cacheValue >> 2) & 0x0f0) | (y & 15);
                        int zz = ((cacheValue << 4) & 0x3f0) | (z & 15);
#if defined(_LARGE_WORLDS)
                        zz |= ((cacheValue >> 22) & 0x3FFFC00);
                        zz = (zz << 6) >> 6;  
#else
                        zz = (zz << 22) >> 22;  
#endif
                        setBrightness(layer, xx, yy, zz, val, true);
                    }
                }
            }
        }
    }
    
    
    
    if (isClientSide && cachewritten) {
        setTilesDirty(cacheminx, cacheminy, cacheminz, cachemaxx, cachemaxy,
                      cachemaxz);
    }
}



bool Level::getInstaTick() { return m_tlsInstaTick; }

void Level::setInstaTick(bool enable) { m_tlsInstaTick = enable; }


bool Level::hasEntitiesToRemove() { return !entitiesToRemove.empty(); }

void Level::_init() {
    cloudColor = 0xffffff;

    skyDarken = 0;

    randValue = (new Random())->nextInt();

    addend = 1013904223;

    oRainLevel = rainLevel = 0.0f;

    oThunderLevel = thunderLevel = 0.0f;

    skyFlashTime = 0;

    difficulty = 0;

    random = new Random();
    isNew = false;

    dimension = nullptr;

    chunkSource = nullptr;

    levelStorage = nullptr;

    levelData = nullptr;

    isFindingSpawn = false;

    savedDataStorage = nullptr;

    spawnEnemies = true;

    spawnFriendlies = true;

    delayUntilNextMoodSound = random->nextInt(20 * 60 * 10);

    isClientSide = false;

    updatingTileEntities = false;

    villageSiege = new VillageSiege(this);
    scoreboard = new Scoreboard();

    toCheckLevel = new int[32 * 32 * 32];  

    
    m_bDisableAddNewTileEntities = false;
    m_iHighestY = -1000;
    m_unsavedChunkCount = 0;
}


Biome* Level::getBiome(int x, int z) {
    if (hasChunkAt(x, 0, z)) {
        LevelChunk* lc = getChunkAt(x, z);
        if (lc != nullptr) {
            
            
            
            Biome* biome =
                lc->getBiome(x & 0xf, z & 0xf, dimension->biomeSource);
            if (biome) return biome;
        }
    }
    return dimension->biomeSource->getBiome(x, z);
}

BiomeSource* Level::getBiomeSource() { return dimension->biomeSource; }

Level::Level(std::shared_ptr<LevelStorage> levelStorage,
             const std::wstring& name, Dimension* dimension,
             LevelSettings* levelSettings, bool doCreateChunkSource)
    : seaLevel(constSeaLevel) {
    _init();
    this->levelStorage =
        levelStorage;  
    this->dimension = dimension;
    levelData = new LevelData(levelSettings, name);
    if (!this->levelData->useNewSeaLevel())
        seaLevel = Level::genDepth /
                   2;  
                       
    savedDataStorage = new SavedDataStorage(levelStorage.get());

    std::shared_ptr<Villages> savedVillages =
        std::dynamic_pointer_cast<Villages>(
            savedDataStorage->get(typeid(Villages), Villages::VILLAGE_FILE_ID));
    if (savedVillages == nullptr) {
        villages = std::make_shared<Villages>(this);
        savedDataStorage->set(Villages::VILLAGE_FILE_ID, villages);
    } else {
        villages = savedVillages;
        villages->setLevel(this);
    }

    dimension->init(this);
    chunkSource = nullptr;  
                            

    updateSkyBrightness();
    prepareWeather();
}

Level::Level(std::shared_ptr<LevelStorage> levelStorage,
             const std::wstring& levelName, LevelSettings* levelSettings)
    : seaLevel(constSeaLevel) {
    _init(levelStorage, levelName, levelSettings, nullptr, true);
}

Level::Level(std::shared_ptr<LevelStorage> levelStorage,
             const std::wstring& levelName, LevelSettings* levelSettings,
             Dimension* fixedDimension, bool doCreateChunkSource)
    : seaLevel(constSeaLevel) {
    _init(levelStorage, levelName, levelSettings, fixedDimension,
          doCreateChunkSource);
}

void Level::_init(std::shared_ptr<LevelStorage> levelStorage,
                  const std::wstring& levelName, LevelSettings* levelSettings,
                  Dimension* fixedDimension, bool doCreateChunkSource) {
    _init();
    this->levelStorage =
        levelStorage;  
    savedDataStorage = new SavedDataStorage(levelStorage.get());

    std::shared_ptr<Villages> savedVillages =
        std::dynamic_pointer_cast<Villages>(
            savedDataStorage->get(typeid(Villages), Villages::VILLAGE_FILE_ID));
    if (savedVillages == nullptr) {
        villages = std::make_shared<Villages>(this);
        savedDataStorage->set(Villages::VILLAGE_FILE_ID, villages);
    } else {
        villages = savedVillages;
        villages->setLevel(this);
    }

    levelData = levelStorage->prepareLevel();
    isNew = levelData == nullptr;

    if (fixedDimension != nullptr) {
        dimension = fixedDimension;
    }
    
    
    
    
    
    
    else {
        dimension = Dimension::getNew(0);
    }

    if (levelData == nullptr) {
        levelData = new LevelData(levelSettings, levelName);
    }
    if (!this->levelData->useNewSeaLevel())
        seaLevel = Level::genDepth /
                   2;  
                       

    ((Dimension*)dimension)->init(this);

    chunkSource = doCreateChunkSource
                      ? createChunkSource()
                      : nullptr;  
                                  

    
    
    
    
    
    

    updateSkyBrightness();
    prepareWeather();
}

Level::~Level() {
    delete random;
    delete dimension;
    delete chunkSource;
    delete levelData;
    delete toCheckLevel;
    delete scoreboard;
    delete villageSiege;

    if (!isClientSide) {
        NotGateTile::removeLevelReferences(this);  
    }

    
    
    
    if (savedDataStorage != nullptr) delete savedDataStorage;

    
    
    



}

void Level::initializeLevel(LevelSettings* settings) {
    levelData->setInitialized(true);
}

void Level::validateSpawn() { setSpawnPos(8, 64, 8); }

int Level::getTopTile(int x, int z) {
    
    
    if (levelData->getGenerator() == LevelType::lvl_flat) {
        return Tile::grass_Id;
    }

    int y = seaLevel;
    while (!isEmptyTile(x, y + 1, z)) {
        y++;
    }
    return getTile(x, y, z);
}
int Level::getTile(int x, int y, int z) {
    if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE ||
        z >= MAX_LEVEL_SIZE) {
        return 0;
    }
    if (y < minBuildHeight) return 0;
    if (y >= maxBuildHeight) return 0;
    return getChunk(x >> 4, z >> 4)->getTile(x & 15, y, z & 15);
}

int Level::getTileLightBlock(int x, int y, int z) {
    if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE ||
        z >= MAX_LEVEL_SIZE) {
        return 0;
    }
    if (y < minBuildHeight) return 0;
    if (y >= maxBuildHeight) return 0;
    return getChunk(x >> 4, z >> 4)->getTileLightBlock(x & 15, y, z & 15);
}

bool Level::isEmptyTile(int x, int y, int z) { return getTile(x, y, z) == 0; }

bool Level::isEntityTile(int x, int y, int z) {
    int t = getTile(x, y, z);
    if (Tile::tiles[t] != nullptr && Tile::tiles[t]->isEntityTile()) {
        return true;
    }
    return false;
}

int Level::getTileRenderShape(int x, int y, int z) {
    int t = getTile(x, y, z);
    if (Tile::tiles[t] != nullptr) {
        return Tile::tiles[t]->getRenderShape();
    }
    return Tile::SHAPE_INVISIBLE;
}



int Level::getTileRenderShape(int t) {
    if (Tile::tiles[t] != nullptr) {
        return Tile::tiles[t]->getRenderShape();
    }
    return Tile::SHAPE_INVISIBLE;
}

bool Level::hasChunkAt(int x, int y, int z) {
    if (y < minBuildHeight || y >= maxBuildHeight) return false;
    return hasChunk(x >> 4, z >> 4);
}


bool Level::reallyHasChunkAt(int x, int y, int z) {
    if (y < minBuildHeight || y >= maxBuildHeight) return false;
    return reallyHasChunk(x >> 4, z >> 4);
}

bool Level::hasChunksAt(int x, int y, int z, int r) {
    return hasChunksAt(x - r, y - r, z - r, x + r, y + r, z + r);
}


bool Level::reallyHasChunksAt(int x, int y, int z, int r) {
    return reallyHasChunksAt(x - r, y - r, z - r, x + r, y + r, z + r);
}

bool Level::hasChunksAt(int x0, int y0, int z0, int x1, int y1, int z1) {
    if (y1 < minBuildHeight || y0 >= maxBuildHeight) return false;

    x0 >>= 4;
    z0 >>= 4;
    x1 >>= 4;
    z1 >>= 4;

    for (int x = x0; x <= x1; x++)
        for (int z = z0; z <= z1; z++)
            if (!hasChunk(x, z)) return false;

    return true;
}


bool Level::reallyHasChunksAt(int x0, int y0, int z0, int x1, int y1, int z1) {
    x0 >>= 4;
    z0 >>= 4;
    x1 >>= 4;
    z1 >>= 4;

    for (int x = x0; x <= x1; x++)
        for (int z = z0; z <= z1; z++)
            if (!reallyHasChunk(x, z)) return false;

    return true;
}

bool Level::hasChunk(int x, int z) { return this->chunkSource->hasChunk(x, z); }


bool Level::reallyHasChunk(int x, int z) {
    return this->chunkSource->reallyHasChunk(x, z);
}

LevelChunk* Level::getChunkAt(int x, int z) { return getChunk(x >> 4, z >> 4); }

LevelChunk* Level::getChunk(int x, int z) {
    return this->chunkSource->getChunk(x, z);
}

bool Level::setTileAndData(int x, int y, int z, int tile, int data,
                           int updateFlags) {
    if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE ||
        z >= MAX_LEVEL_SIZE) {
        return false;
    }
    if (y < 0) return false;
    if (y >= maxBuildHeight) return false;
    LevelChunk* c = getChunk(x >> 4, z >> 4);

    int oldTile = 0;
    if ((updateFlags & Tile::UPDATE_NEIGHBORS) != 0) {
        oldTile = c->getTile(x & 15, y, z & 15);
    }
    bool result;
#if !defined(_CONTENT_PACKAGE)
    int old = c->getTile(x & 15, y, z & 15);
    int olddata = c->getData(x & 15, y, z & 15);
#endif
    result = c->setTileAndData(x & 15, y, z & 15, tile, data);
    if (updateFlags != Tile::UPDATE_INVISIBLE_NO_LIGHT) {
        checkLight(x, y, z);
    }
    if (result) {
        if ((updateFlags & Tile::UPDATE_CLIENTS) != 0 &&
            !(isClientSide && (updateFlags & Tile::UPDATE_INVISIBLE) != 0)) {
            sendTileUpdated(x, y, z);
        }
        if (!isClientSide && (updateFlags & Tile::UPDATE_NEIGHBORS) != 0) {
            tileUpdated(x, y, z, oldTile);
            Tile* tobj = Tile::tiles[tile];
            if (tobj != nullptr && tobj->hasAnalogOutputSignal())
                updateNeighbourForOutputSignal(x, y, z, tile);
        }
    }
    return result;
}

Material* Level::getMaterial(int x, int y, int z) {
    int t = getTile(x, y, z);
    if (t == 0) return Material::air;
    return Tile::tiles[t]->material;
}

int Level::getData(int x, int y, int z) {
    if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE ||
        z >= MAX_LEVEL_SIZE) {
        return 0;
    }
    if (y < 0) return 0;
    if (y >= maxBuildHeight) return 0;
    LevelChunk* c = getChunk(x >> 4, z >> 4);
    x &= 15;
    z &= 15;
    return c->getData(x, y, z);
}

bool Level::setData(int x, int y, int z, int data, int updateFlags,
                    bool forceUpdate )  
{
    if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE ||
        z >= MAX_LEVEL_SIZE) {
        return false;
    }
    if (y < 0) return false;
    if (y >= maxBuildHeight) return false;
    LevelChunk* c = getChunk(x >> 4, z >> 4);
    int cx = x & 15;
    int cz = z & 15;
    
    
    
    
    
    
    
    
    unsigned char importantMask =
        Tile::_sendTileData[c->getTile(cx, y, cz) & Tile::TILE_NUM_MASK];
    bool sendTileData = importantMask != 0;

    bool maskedBitsChanged;
    bool result =
        c->setData(cx, y, cz, data, importantMask, &maskedBitsChanged);
    if (result || forceUpdate) {
        int tile = c->getTile(cx, y, cz);
        if (forceUpdate ||
            ((updateFlags & Tile::UPDATE_CLIENTS) != 0 &&
             !(isClientSide && (updateFlags & Tile::UPDATE_INVISIBLE) != 0))) {
            sendTileUpdated(x, y, z);
        }
        if (!isClientSide &&
            (forceUpdate || (updateFlags & Tile::UPDATE_NEIGHBORS) != 0)) {
            tileUpdated(x, y, z, tile);
            Tile* tobj = Tile::tiles[tile];
            if (tobj != nullptr && tobj->hasAnalogOutputSignal())
                updateNeighbourForOutputSignal(x, y, z, tile);
        }
    }
    return result;
}









bool Level::removeTile(int x, int y, int z) {
    return setTileAndData(x, y, z, 0, 0, Tile::UPDATE_ALL);
}











bool Level::destroyTile(int x, int y, int z, bool dropResources) {
    int tile = getTile(x, y, z);
    if (tile > 0) {
        int data = getData(x, y, z);
        levelEvent(LevelEvent::PARTICLES_DESTROY_BLOCK, x, y, z,
                   tile + (data << Tile::TILE_NUM_SHIFT));
        if (dropResources) {
            Tile::tiles[tile]->spawnResources(this, x, y, z, data, 0);
        }
        return setTileAndData(x, y, z, 0, 0, Tile::UPDATE_ALL);
    }
    return false;
}

bool Level::setTileAndUpdate(int x, int y, int z, int tile) {
    return setTileAndData(x, y, z, tile, 0, Tile::UPDATE_ALL);
}

void Level::sendTileUpdated(int x, int y, int z) {
    auto itEnd = listeners.end();
    for (auto it = listeners.begin(); it != itEnd; it++) {
        (*it)->tileChanged(x, y, z);
    }
}

void Level::tileUpdated(int x, int y, int z, int tile) {
    updateNeighborsAt(x, y, z, tile);
}

void Level::lightColumnChanged(int x, int z, int y0, int y1) {
    if (y0 > y1) {
        int tmp = y1;
        y1 = y0;
        y0 = tmp;
    }

    if (!dimension->hasCeiling) {
        for (int y = y0; y <= y1; y++) {
            checkLight(LightLayer::Sky, x, y, z);
        }
    }
    setTilesDirty(x, y0, z, x, y1, z);
}

void Level::setTileDirty(int x, int y, int z) {
    auto itEnd = listeners.end();
    for (auto it = listeners.begin(); it != itEnd; it++) {
        (*it)->setTilesDirty(x, y, z, x, y, z, this);
    }
}

void Level::setTilesDirty(int x0, int y0, int z0, int x1, int y1, int z1) {
    auto itEnd = listeners.end();
    for (auto it = listeners.begin(); it != itEnd; it++) {
        (*it)->setTilesDirty(x0, y0, z0, x1, y1, z1, this);
    }
}

void Level::updateNeighborsAt(int x, int y, int z, int tile) {
    neighborChanged(x - 1, y, z, tile);
    neighborChanged(x + 1, y, z, tile);
    neighborChanged(x, y - 1, z, tile);
    neighborChanged(x, y + 1, z, tile);
    neighborChanged(x, y, z - 1, tile);
    neighborChanged(x, y, z + 1, tile);
}

void Level::updateNeighborsAtExceptFromFacing(int x, int y, int z, int tile,
                                              int skipFacing) {
    if (skipFacing != Facing::WEST) neighborChanged(x - 1, y, z, tile);
    if (skipFacing != Facing::EAST) neighborChanged(x + 1, y, z, tile);
    if (skipFacing != Facing::DOWN) neighborChanged(x, y - 1, z, tile);
    if (skipFacing != Facing::UP) neighborChanged(x, y + 1, z, tile);
    if (skipFacing != Facing::NORTH) neighborChanged(x, y, z - 1, tile);
    if (skipFacing != Facing::SOUTH) neighborChanged(x, y, z + 1, tile);
}

void Level::neighborChanged(int x, int y, int z, int type) {
    if (isClientSide) return;
    int id = getTile(x, y, z);
    Tile* tile = Tile::tiles[id];

    if (tile != nullptr) {
        tile->neighborChanged(this, x, y, z, type);
    }
}

bool Level::isTileToBeTickedAt(int x, int y, int z, int tileId) {
    return false;
}

bool Level::canSeeSky(int x, int y, int z) {
    return getChunk(x >> 4, z >> 4)->isSkyLit(x & 15, y, z & 15);
}

int Level::getDaytimeRawBrightness(int x, int y, int z) {
    if (y < 0) return 0;
    if (y >= maxBuildHeight) y = maxBuildHeight - 1;
    return getChunk(x >> 4, z >> 4)->getRawBrightness(x & 15, y, z & 15, 0);
}

int Level::getRawBrightness(int x, int y, int z) {
    return getRawBrightness(x, y, z, true);
}

int Level::getRawBrightness(int x, int y, int z, bool propagate) {
    if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE ||
        z >= MAX_LEVEL_SIZE) {
        return MAX_BRIGHTNESS;
    }

    if (propagate) {
        int id = getTile(x, y, z);
        if (Tile::propagate[id]) {
            int br = getRawBrightness(x, y + 1, z, false);
            int br1 = getRawBrightness(x + 1, y, z, false);
            int br2 = getRawBrightness(x - 1, y, z, false);
            int br3 = getRawBrightness(x, y, z + 1, false);
            int br4 = getRawBrightness(x, y, z - 1, false);
            if (br1 > br) br = br1;
            if (br2 > br) br = br2;
            if (br3 > br) br = br3;
            if (br4 > br) br = br4;
            return br;
        }
    }

    if (y < 0) return 0;
    if (y >= maxBuildHeight) y = maxBuildHeight - 1;

    LevelChunk* c = getChunk(x >> 4, z >> 4);
    x &= 15;
    z &= 15;
    return c->getRawBrightness(x, y, z, skyDarken);
}

bool Level::isSkyLit(int x, int y, int z) {
    if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE ||
        z >= MAX_LEVEL_SIZE) {
        return false;
    }
    if (dimension->hasCeiling) return false;

    if (y < 0) return false;
    if (y >= maxBuildHeight) return true;
    if (!hasChunk(x >> 4, z >> 4)) return false;

    LevelChunk* c = getChunk(x >> 4, z >> 4);
    x &= 15;
    z &= 15;
    return c->isSkyLit(x, y, z);
}

int Level::getHeightmap(int x, int z) {
    if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE ||
        z >= MAX_LEVEL_SIZE) {
        return 0;
    }
    if (!hasChunk(x >> 4, z >> 4)) return 0;

    LevelChunk* c = getChunk(x >> 4, z >> 4);
    return c->getHeightmap(x & 15, z & 15);
}

int Level::getLowestHeightmap(int x, int z) {
    if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE ||
        z >= MAX_LEVEL_SIZE) {
        return 0;
    }
    if (!hasChunk(x >> 4, z >> 4)) return 0;

    LevelChunk* c = getChunk(x >> 4, z >> 4);
    return c->lowestHeightmap;
}

void Level::updateLightIfOtherThan(LightLayer::variety layer, int x, int y,
                                   int z, int expected) {
    if (dimension->hasCeiling && layer == LightLayer::Sky) return;

    if (!hasChunkAt(x, y, z)) return;

    if (layer == LightLayer::Sky) {
        if (isSkyLit(x, y, z)) expected = 15;
    } else if (layer == LightLayer::Block) {
        int t = getTile(x, y, z);
        if (Tile::lightEmission[t] > expected)
            expected = Tile::lightEmission[t];
    }

    if (getBrightness(layer, x, y, z) != expected) {
        setBrightness(layer, x, y, z, expected);
    }
}


int Level::getBrightnessPropagate(LightLayer::variety layer, int x, int y,
                                  int z, int tileId) {
    if (dimension->hasCeiling && layer == LightLayer::Sky) return 0;

    if (y < 0) y = 0;
    if (y >= maxBuildHeight && layer == LightLayer::Sky) {
        
        
        
        
        return (int)layer;
    }
    if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE ||
        z >= MAX_LEVEL_SIZE) {
        
        
        
        
        return (int)layer;
    }
    int xc = x >> 4;
    int zc = z >> 4;
    if (!hasChunk(xc, zc)) return (int)layer;

    {
        int id = tileId > -1 ? tileId : getTile(x, y, z);
        if (Tile::propagate[id]) {
            int br = getBrightness(layer, x, y + 1, z);
            int br1 = getBrightness(layer, x + 1, y, z);
            int br2 = getBrightness(layer, x - 1, y, z);
            int br3 = getBrightness(layer, x, y, z + 1);
            int br4 = getBrightness(layer, x, y, z - 1);
            if (br1 > br) br = br1;
            if (br2 > br) br = br2;
            if (br3 > br) br = br3;
            if (br4 > br) br = br4;
            return br;
        }
    }

    LevelChunk* c = getChunk(xc, zc);
    return c->getBrightness(layer, x & 15, y, z & 15);
}

int Level::getBrightness(LightLayer::variety layer, int x, int y, int z) {
    
    
    
    
    int xc = x >> 4;
    int zc = z >> 4;

    int ix = xc + (chunkSourceXZSize / 2);
    int iz = zc + (chunkSourceXZSize / 2);

    if ((ix < 0) || (ix >= chunkSourceXZSize)) return 0;
    if ((iz < 0) || (iz >= chunkSourceXZSize)) return 0;
    int idx = ix * chunkSourceXZSize + iz;
    LevelChunk* c = chunkSourceCache[idx];

    if (c == nullptr) return (int)layer;

    if (y < 0) y = 0;
    if (y >= maxBuildHeight) y = maxBuildHeight - 1;

    return c->getBrightness(layer, x & 15, y, z & 15);
}



void Level::getNeighbourBrightnesses(int* brightnesses,
                                     LightLayer::variety layer, int x, int y,
                                     int z) {
    if ((((x & 15) == 0) || ((x & 15) == 15)) ||
        (((z & 15) == 0) || ((z & 15) == 15)) || ((y <= 0) || (y >= 127))) {
        
        
        brightnesses[0] = getBrightness(layer, x - 1, y, z);
        brightnesses[1] = getBrightness(layer, x + 1, y, z);
        brightnesses[2] = getBrightness(layer, x, y - 1, z);
        brightnesses[3] = getBrightness(layer, x, y + 1, z);
        brightnesses[4] = getBrightness(layer, x, y, z - 1);
        brightnesses[5] = getBrightness(layer, x, y, z + 1);
    } else {
        
        
        int xc = x >> 4;
        int zc = z >> 4;

        int ix = xc + (chunkSourceXZSize / 2);
        int iz = zc + (chunkSourceXZSize / 2);

        
        
        
        
        if (((ix < 0) || (ix >= chunkSourceXZSize)) ||
            ((iz < 0) || (iz >= chunkSourceXZSize))) {
            for (int i = 0; i < 6; i++) {
                brightnesses[i] = (int)layer;
            }
            return;
        }

        int idx = ix * chunkSourceXZSize + iz;
        LevelChunk* c = chunkSourceCache[idx];

        
        
        
        
        if (c == nullptr) {
            for (int i = 0; i < 6; i++) {
                brightnesses[i] = (int)layer;
            }
            return;
        }

        
        
        c->getNeighbourBrightnesses(brightnesses, layer, x & 15, y, z & 15);
    }
}

void Level::setBrightness(
    LightLayer::variety layer, int x, int y, int z, int brightness,
    bool noUpdateOnClient )  
{
    if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE ||
        z >= MAX_LEVEL_SIZE) {
        return;
    }
    if (y < 0) return;
    if (y >= maxBuildHeight) return;
    if (!hasChunk(x >> 4, z >> 4)) return;
    LevelChunk* c = getChunk(x >> 4, z >> 4);

    c->setBrightness(layer, x & 15, y, z & 15, brightness);

    
    if (isClientSide && noUpdateOnClient) {
        if (cachewritten) {
            if (x < cacheminx) cacheminx = x;
            if (x > cachemaxx) cachemaxx = x;
            if (y < cacheminy) cacheminy = y;
            if (y > cachemaxy) cachemaxy = y;
            if (z < cacheminz) cacheminz = z;
            if (z > cachemaxz) cachemaxz = z;
        } else {
            cachewritten = true;
            cacheminx = x;
            cachemaxx = x;
            cacheminy = y;
            cachemaxy = y;
            cacheminz = z;
            cachemaxz = z;
        }
    } else {
        auto itEnd = listeners.end();
        for (auto it = listeners.begin(); it != itEnd; it++) {
            (*it)->tileLightChanged(x, y, z);
        }
    }
}

void Level::setTileBrightnessChanged(int x, int y, int z) {
    auto itEnd = listeners.end();
    for (auto it = listeners.begin(); it != itEnd; it++) {
        (*it)->tileLightChanged(x, y, z);
    }
}

int Level::getLightColor(int x, int y, int z, int emitt, int tileId ) {
    int s = getBrightnessPropagate(LightLayer::Sky, x, y, z, tileId);
    int b = getBrightnessPropagate(LightLayer::Block, x, y, z, tileId);
    if (b < emitt) b = emitt;
    return s << 20 | b << 4;
}

float Level::getBrightness(int x, int y, int z, int emitt) {
    int n = getRawBrightness(x, y, z);
    if (n < emitt) n = emitt;
    return dimension->brightnessRamp[n];
}

float Level::getBrightness(int x, int y, int z) {
    return dimension->brightnessRamp[getRawBrightness(x, y, z)];
}

bool Level::isDay() { return skyDarken < 4; }

HitResult* Level::clip(Vec3* a, Vec3* b) { return clip(a, b, false, false); }

HitResult* Level::clip(Vec3* a, Vec3* b, bool liquid) {
    return clip(a, b, liquid, false);
}

HitResult* Level::clip(Vec3* a, Vec3* b, bool liquid, bool solidOnly) {
    if (std::isnan(a->x) || std::isnan(a->y) || std::isnan(a->z))
        return nullptr;
    if (std::isnan(b->x) || std::isnan(b->y) || std::isnan(b->z))
        return nullptr;

    int xTile1 = Mth::floor(b->x);
    int yTile1 = Mth::floor(b->y);
    int zTile1 = Mth::floor(b->z);

    int xTile0 = Mth::floor(a->x);
    int yTile0 = Mth::floor(a->y);
    int zTile0 = Mth::floor(a->z);

    {
        int t = getTile(xTile0, yTile0, zTile0);
        int data = getData(xTile0, yTile0, zTile0);
        Tile* tile = Tile::tiles[t];
        if (solidOnly && tile != nullptr &&
            !tile->getAABB(this, xTile0, yTile0, zTile0).has_value()) {
            

        } else if (t > 0 && tile->mayPick(data, liquid)) {
            HitResult* r = tile->clip(this, xTile0, yTile0, zTile0, a, b);
            if (r != nullptr) return r;
        }
    }

    int maxIterations = 200;
    while (maxIterations-- >= 0) {
        if (std::isnan(a->x) || std::isnan(a->y) || std::isnan(a->z))
            return nullptr;
        if (xTile0 == xTile1 && yTile0 == yTile1 && zTile0 == zTile1)
            return nullptr;

        bool xClipped = true;
        bool yClipped = true;
        bool zClipped = true;

        double xClip = 999;
        double yClip = 999;
        double zClip = 999;

        if (xTile1 > xTile0)
            xClip = xTile0 + 1.000;
        else if (xTile1 < xTile0)
            xClip = xTile0 + 0.000;
        else
            xClipped = false;

        if (yTile1 > yTile0)
            yClip = yTile0 + 1.000;
        else if (yTile1 < yTile0)
            yClip = yTile0 + 0.000;
        else
            yClipped = false;

        if (zTile1 > zTile0)
            zClip = zTile0 + 1.000;
        else if (zTile1 < zTile0)
            zClip = zTile0 + 0.000;
        else
            zClipped = false;

        double xDist = 999;
        double yDist = 999;
        double zDist = 999;

        double xd = b->x - a->x;
        double yd = b->y - a->y;
        double zd = b->z - a->z;

        if (xClipped) xDist = (xClip - a->x) / xd;
        if (yClipped) yDist = (yClip - a->y) / yd;
        if (zClipped) zDist = (zClip - a->z) / zd;

        int face = 0;
        if (xDist < yDist && xDist < zDist) {
            if (xTile1 > xTile0)
                face = 4;
            else
                face = 5;

            a->x = xClip;
            a->y += yd * xDist;
            a->z += zd * xDist;
        } else if (yDist < zDist) {
            if (yTile1 > yTile0)
                face = 0;
            else
                face = 1;

            a->x += xd * yDist;
            a->y = yClip;
            a->z += zd * yDist;
        } else {
            if (zTile1 > zTile0)
                face = 2;
            else
                face = 3;

            a->x += xd * zDist;
            a->y += yd * zDist;
            a->z = zClip;
        }

        Vec3 tPos(a->x, a->y, a->z);
        xTile0 = (int)(tPos.x = floor(a->x));
        if (face == 5) {
            xTile0--;
            tPos.x++;
        }
        yTile0 = (int)(tPos.y = floor(a->y));
        if (face == 1) {
            yTile0--;
            tPos.y++;
        }
        zTile0 = (int)(tPos.z = floor(a->z));
        if (face == 3) {
            zTile0--;
            tPos.z++;
        }

        int t = getTile(xTile0, yTile0, zTile0);
        int data = getData(xTile0, yTile0, zTile0);
        Tile* tile = Tile::tiles[t];
        if (solidOnly && tile != nullptr &&
            !tile->getAABB(this, xTile0, yTile0, zTile0).has_value()) {
            

        } else if (t > 0 && tile->mayPick(data, liquid)) {
            HitResult* r = tile->clip(this, xTile0, yTile0, zTile0, a, b);
            if (r != nullptr) return r;
        }
    }
    return nullptr;
}

void Level::playEntitySound(std::shared_ptr<Entity> entity, int iSound,
                            float volume, float pitch) {
    if (entity == nullptr) return;
    auto itEnd = listeners.end();
    for (auto it = listeners.begin(); it != itEnd; it++) {
        
        if (entity->GetType() == eTYPE_SERVERPLAYER) {
            

            (*it)->playSound(iSound, entity->x,
                             entity->y - entity->heightOffset, entity->z,
                             volume, pitch);
        } else {
            (*it)->playSound(iSound, entity->x,
                             entity->y - entity->heightOffset, entity->z,
                             volume, pitch);
        }
    }
}

void Level::playPlayerSound(std::shared_ptr<Player> entity, int iSound,
                            float volume, float pitch) {
    if (entity == nullptr) return;
    auto itEnd = listeners.end();
    for (auto it = listeners.begin(); it != itEnd; it++) {
        (*it)->playSoundExceptPlayer(entity, iSound, entity->x,
                                     entity->y - entity->heightOffset,
                                     entity->z, volume, pitch);
    }
}



void Level::playSound(double x, double y, double z, int iSound, float volume,
                      float pitch, float fClipSoundDist) {
    auto itEnd = listeners.end();
    for (auto it = listeners.begin(); it != itEnd; it++) {
        (*it)->playSound(iSound, x, y, z, volume, pitch, fClipSoundDist);
    }
}

void Level::playLocalSound(double x, double y, double z, int iSound,
                           float volume, float pitch, bool distanceDelay,
                           float fClipSoundDist) {}

void Level::playStreamingMusic(const std::wstring& name, int x, int y, int z) {
    auto itEnd = listeners.end();
    for (auto it = listeners.begin(); it != itEnd; it++) {
        (*it)->playStreamingMusic(name, x, y, z);
    }
}

void Level::playMusic(double x, double y, double z, const std::wstring& string,
                      float volume) {}













void Level::addParticle(ePARTICLE_TYPE id, double x, double y, double z,
                        double xd, double yd, double zd) {
    auto itEnd = listeners.end();
    for (auto it = listeners.begin(); it != itEnd; it++)
        (*it)->addParticle(id, x, y, z, xd, yd, zd);
}

bool Level::addGlobalEntity(std::shared_ptr<Entity> e) {
    globalEntities.push_back(e);
    return true;
}

#pragma optimize("", off)

bool Level::addEntity(std::shared_ptr<Entity> e) {
    int xc = Mth::floor(e->x / 16);
    int zc = Mth::floor(e->z / 16);

    if (e == nullptr) {
        return false;
    }

    bool forced = e->forcedLoading;
    if (e->instanceof(eTYPE_PLAYER)) {
        forced = true;
    }

    if (forced || hasChunk(xc, zc)) {
        if (e->instanceof(eTYPE_PLAYER)) {
            std::shared_ptr<Player> player =
                std::dynamic_pointer_cast<Player>(e);

            
            
            if (find(players.begin(), players.end(), e) == players.end()) {
                players.push_back(player);
            }

            updateSleepingPlayerList();
        }
        getChunk(xc, zc)->addEntity(e);
        {
            std::lock_guard<std::recursive_mutex> lock(m_entitiesCS);
            entities.push_back(e);
        }
        entityAdded(e);
        return true;
    }
    return false;
}

#pragma optimize("", on)

void Level::entityAdded(std::shared_ptr<Entity> e) {
    auto itEnd = listeners.end();
    for (auto it = listeners.begin(); it != itEnd; it++) {
        (*it)->entityAdded(e);
    }
}

void Level::entityRemoved(std::shared_ptr<Entity> e) {
    auto itEnd = listeners.end();
    for (auto it = listeners.begin(); it != itEnd; it++) {
        (*it)->entityRemoved(e);
    }
}


void Level::playerRemoved(std::shared_ptr<Entity> e) {
    auto itEnd = listeners.end();
    for (auto it = listeners.begin(); it != itEnd; it++) {
        (*it)->playerRemoved(e);
    }
}

void Level::removeEntity(std::shared_ptr<Entity> e) {
    if (e->rider.lock() != nullptr) {
        e->rider.lock()->ride(nullptr);
    }
    if (e->riding != nullptr) {
        e->ride(nullptr);
    }
    e->remove();
    if (e->instanceof(eTYPE_PLAYER)) {
        std::vector<std::shared_ptr<Player> >::iterator it = players.begin();
        std::vector<std::shared_ptr<Player> >::iterator itEnd = players.end();
        while (it != itEnd && *it != std::dynamic_pointer_cast<Player>(e)) it++;

        if (it != itEnd) {
            players.erase(it);
        }

        updateSleepingPlayerList();
        playerRemoved(e);  
                           
                           
    }
}

void Level::removeEntityImmediately(std::shared_ptr<Entity> e) {
    e->remove();

    if (e->instanceof(eTYPE_PLAYER)) {
        std::vector<std::shared_ptr<Player> >::iterator it = players.begin();
        std::vector<std::shared_ptr<Player> >::iterator itEnd = players.end();
        while (it != itEnd && *it != std::dynamic_pointer_cast<Player>(e)) it++;

        if (it != itEnd) {
            players.erase(it);
        }

        updateSleepingPlayerList();
        playerRemoved(e);  
                           
                           
    }

    int xc = e->xChunk;
    int zc = e->zChunk;
    if (e->inChunk && hasChunk(xc, zc)) {
        getChunk(xc, zc)->removeEntity(e);
    }

    {
        std::lock_guard<std::recursive_mutex> lock(m_entitiesCS);
        std::vector<std::shared_ptr<Entity> >::iterator it = entities.begin();
        std::vector<std::shared_ptr<Entity> >::iterator endIt = entities.end();
        while (it != endIt && *it != e) it++;

        if (it != endIt) {
            entities.erase(it);
        }
    }
    entityRemoved(e);
}

void Level::addListener(LevelListener* listener) {
    listeners.push_back(listener);
}

void Level::removeListener(LevelListener* listener) {
    std::vector<LevelListener*>::iterator it = listeners.begin();
    std::vector<LevelListener*>::iterator itEnd = listeners.end();
    while (it != itEnd && *it != listener) it++;

    if (it != itEnd) listeners.erase(it);
}


std::vector<AABB>* Level::getCubes(std::shared_ptr<Entity> source, AABB* box,
                                   bool noEntities ,
                                   bool blockAtEdge ) {
    boxes.clear();
    int x0 = Mth::floor(box->x0);
    int x1 = Mth::floor(box->x1 + 1);
    int y0 = Mth::floor(box->y0);
    int y1 = Mth::floor(box->y1 + 1);
    int z0 = Mth::floor(box->z0);
    int z1 = Mth::floor(box->z1 + 1);

    int maxxz = (dimension->getXZSize() * 16) / 2;
    int minxz = -maxxz;
    for (int x = x0; x < x1; x++)
        for (int z = z0; z < z1; z++) {
            
            
            if (blockAtEdge &&
                ((x < minxz) || (x >= maxxz) || (z < minxz) || (z >= maxxz))) {
                for (int y = y0 - 1; y < y1; y++) {
                    Tile::stone->addAABBs(this, x, y, z, box, &boxes, source);
                }
            } else {
                if (hasChunkAt(x, 64, z)) {
                    for (int y = y0 - 1; y < y1; y++) {
                        Tile* tile = Tile::tiles[getTile(x, y, z)];
                        if (tile != nullptr) {
                            tile->addAABBs(this, x, y, z, box, &boxes, source);
                        }
                    }
                }
            }
        }
    
    
    
    if (blockAtEdge && ((y0 - 1) < 0) && dimension->id != 1) {
        for (int y = y0 - 1; y < 0; y++) {
            for (int x = x0; x < x1; x++)
                for (int z = z0; z < z1; z++) {
                    Tile::stone->addAABBs(this, x, y, z, box, &boxes, source);
                }
        }
    }
    
    
    if (blockAtEdge && (y1 > maxMovementHeight)) {
        for (int y = maxMovementHeight; y < y1; y++) {
            for (int x = x0; x < x1; x++)
                for (int z = z0; z < z1; z++) {
                    Tile::stone->addAABBs(this, x, y, z, box, &boxes, source);
                }
        }
    }
    
    
    
    
    
    
    
    Minecraft::GetInstance()->levelRenderer->destroyedTileManager->addAABBs(
        this, box, &boxes);

    
    if (noEntities) return &boxes;

    double r = 0.25;
    AABB grown = box->grow(r, r, r);
    std::vector<std::shared_ptr<Entity> > ee;
    ee.reserve(16);  
    getEntities(source, &grown, ee);
    std::vector<std::shared_ptr<Entity> >::iterator itEnd = ee.end();
    for (auto it = ee.begin(); it != itEnd; it++) {
        AABB* collideBox = (*it)->getCollideBox();
        if (collideBox != nullptr && collideBox->intersects(*box)) {
            boxes.push_back(*collideBox);
        }

        collideBox = source->getCollideAgainstBox(*it);
        if (collideBox != nullptr && collideBox->intersects(*box)) {
            boxes.push_back(*collideBox);
        }
    }

    return &boxes;
}



std::vector<AABB>* Level::getTileCubes(AABB* box,
                                       bool blockAtEdge ) {
    return getCubes(nullptr, box, true, blockAtEdge);
    
    
    
    
    
    
    

    
    
    
    
    
    
    
    
    

    
    
    
    
    
    
    
    
    

    
}


int Level::getOldSkyDarken(float a) {
    float td = getTimeOfDay(a);

    float br = 1 - (cosf(td * std::numbers::pi * 2) * 2 + 0.5f);
    if (br < 0.0f) br = 0.0f;
    if (br > 1.0f) br = 1.0f;

    br = 1 - br;

    br *= 1 - (getRainLevel(a) * 5 / 16.0f);
    br *= 1 - (getThunderLevel(a) * 5 / 16.0f);
    br = 1 - br;
    return ((int)(br * 11));
}


float Level::getSkyDarken(float a) {
    float td = getTimeOfDay(a);

    float br = 1 - (cosf(td * std::numbers::pi * 2) * 2 + 0.2f);
    if (br < 0.0f) br = 0.0f;
    if (br > 1.0f) br = 1.0f;

    br = 1.0f - br;

    br *= 1.0f - (getRainLevel(a) * 5.0f / 16.0f);
    br *= 1.0f - (getThunderLevel(a) * 5.0f / 16.0f);
    

    return br * 0.8f + 0.2f;
}

Vec3 Level::getSkyColor(std::shared_ptr<Entity> source, float a) {
    float td = getTimeOfDay(a);

    float br = cosf(td * std::numbers::pi * 2) * 2 + 0.5f;
    if (br < 0.0f) br = 0.0f;
    if (br > 1.0f) br = 1.0f;

    int xx = Mth::floor(source->x);
    int zz = Mth::floor(source->z);
    Biome* biome = getBiome(xx, zz);
    float temp = biome->getTemperature();
    int skyColor = biome->getSkyColor(temp);

    float r = ((skyColor >> 16) & 0xff) / 255.0f;
    float g = ((skyColor >> 8) & 0xff) / 255.0f;
    float b = ((skyColor) & 0xff) / 255.0f;
    r *= br;
    g *= br;
    b *= br;

    float rainLevel = getRainLevel(a);
    if (rainLevel > 0) {
        float mid = (r * 0.30f + g * 0.59f + b * 0.11f) * 0.6f;

        float ba = 1 - rainLevel * 0.75f;
        r = r * ba + mid * (1 - ba);
        g = g * ba + mid * (1 - ba);
        b = b * ba + mid * (1 - ba);
    }
    float thunderLevel = getThunderLevel(a);
    if (thunderLevel > 0) {
        float mid = (r * 0.30f + g * 0.59f + b * 0.11f) * 0.2f;

        float ba = 1 - thunderLevel * 0.75f;
        r = r * ba + mid * (1 - ba);
        g = g * ba + mid * (1 - ba);
        b = b * ba + mid * (1 - ba);
    }

    if (skyFlashTime > 0) {
        float f = (skyFlashTime - a);
        if (f > 1) f = 1;
        f = f * 0.45f;
        r = r * (1 - f) + 0.8f * f;
        g = g * (1 - f) + 0.8f * f;
        b = b * (1 - f) + 1 * f;
    }

    return Vec3(r, g, b);
}

float Level::getTimeOfDay(float a) {
    




    

    
    
    return dimension->getTimeOfDay(levelData->getDayTime(), a);
    ;
}

int Level::getMoonPhase() {
    return dimension->getMoonPhase(levelData->getDayTime());
}

float Level::getMoonBrightness() {
    return Dimension::MOON_BRIGHTNESS_PER_PHASE[dimension->getMoonPhase(
        levelData->getDayTime())];
}

float Level::getSunAngle(float a) {
    float td = getTimeOfDay(a);
    return td * std::numbers::pi * 2;
}

Vec3 Level::getCloudColor(float a) {
    float td = getTimeOfDay(a);

    float br = cosf(td * std::numbers::pi * 2) * 2.0f + 0.5f;
    if (br < 0.0f) br = 0.0f;
    if (br > 1.0f) br = 1.0f;

    int baseCloudColour = Minecraft::GetInstance()->getColourTable()->getColor(
        eMinecraftColour_In_Cloud_Base_Colour);

    float r = ((baseCloudColour >> 16) & 0xff) / 255.0f;
    float g = ((baseCloudColour >> 8) & 0xff) / 255.0f;
    float b = ((baseCloudColour) & 0xff) / 255.0f;

    float rainLevel = getRainLevel(a);
    if (rainLevel > 0) {
        float mid = (r * 0.30f + g * 0.59f + b * 0.11f) * 0.6f;

        float ba = 1 - rainLevel * 0.95f;
        r = r * ba + mid * (1 - ba);
        g = g * ba + mid * (1 - ba);
        b = b * ba + mid * (1 - ba);
    }

    r *= br * 0.90f + 0.10f;
    g *= br * 0.90f + 0.10f;
    b *= br * 0.85f + 0.15f;

    float thunderLevel = getThunderLevel(a);
    if (thunderLevel > 0) {
        float mid = (r * 0.30f + g * 0.59f + b * 0.11f) * 0.2f;

        float ba = 1 - thunderLevel * 0.95f;
        r = r * ba + mid * (1 - ba);
        g = g * ba + mid * (1 - ba);
        b = b * ba + mid * (1 - ba);
    }

    return Vec3(r, g, b);
}

Vec3 Level::getFogColor(float a) {
    float td = getTimeOfDay(a);
    return dimension->getFogColor(td, a);
}

int Level::getTopRainBlock(int x, int z) {
    
    
    return getChunkAt(x, z)->getTopRainBlock(x & 15, z & 15);
}


bool Level::biomeHasRain(int x, int z) {
    return getChunkAt(x, z)->biomeHasRain(x & 15, z & 15);
}


bool Level::biomeHasSnow(int x, int z) {
    return getChunkAt(x, z)->biomeHasSnow(x & 15, z & 15);
}

int Level::getTopSolidBlock(int x, int z) {
    LevelChunk* levelChunk = getChunkAt(x, z);

    int y = levelChunk->getHighestSectionPosition() + 15;

    x &= 15;
    z &= 15;

    while (y > 0) {
        int t = levelChunk->getTile(x, y, z);
        if (t == 0 || !(Tile::tiles[t]->material->blocksMotion()) ||
            Tile::tiles[t]->material == Material::leaves) {
            y--;
        } else {
            return y + 1;
        }
    }
    return -1;
}

int Level::getLightDepth(int x, int z) {
    return getChunkAt(x, z)->getHeightmap(x & 15, z & 15);
}

float Level::getStarBrightness(float a) {
    float td = getTimeOfDay(a);

    float br = 1 - (cosf(td * std::numbers::pi * 2) * 2 + 0.25f);
    if (br < 0.0f) br = 0.0f;
    if (br > 1.0f) br = 1.0f;

    return br * br * 0.5f;
}

void Level::addToTickNextTick(int x, int y, int z, int tileId, int tickDelay) {}

void Level::addToTickNextTick(int x, int y, int z, int tileId, int tickDelay,
                              int priorityTilt) {}

void Level::forceAddTileTick(int x, int y, int z, int tileId, int tickDelay,
                             int prioTilt) {}

void Level::tickEntities() {
    std::vector<std::shared_ptr<Entity> >::iterator itGE =
        globalEntities.begin();
    while (itGE != globalEntities.end()) {
        std::shared_ptr<Entity> e = *itGE;
        e->tickCount++;
        e->tick();
        if (e->removed) {
            itGE = globalEntities.erase(itGE);
        } else {
            itGE++;
        }
    }

    {
        std::lock_guard<std::recursive_mutex> lock(m_entitiesCS);

        for (auto it = entities.begin(); it != entities.end();) {
            bool found = false;
            for (auto it2 = entitiesToRemove.begin();
                 it2 != entitiesToRemove.end(); it2++) {
                if ((*it) == (*it2)) {
                    found = true;
                    break;
                }
            }
            if (found) {
                it = entities.erase(it);
            } else {
                it++;
            }
        }
    }

    auto itETREnd = entitiesToRemove.end();
    for (auto it = entitiesToRemove.begin(); it != itETREnd; it++) {
        std::shared_ptr<Entity> e = *it;  
        int xc = e->xChunk;
        int zc = e->zChunk;
        if (e->inChunk && hasChunk(xc, zc)) {
            getChunk(xc, zc)->removeEntity(e);
        }
    }

    itETREnd = entitiesToRemove.end();
    for (auto it = entitiesToRemove.begin(); it != itETREnd; it++) {
        entityRemoved(*it);
    }
    
    entitiesToRemove.clear();

    

    


    {
        std::lock_guard<std::recursive_mutex> lock(m_entitiesCS);

        for (unsigned int i = 0; i < entities.size();) {
            std::shared_ptr<Entity> e = entities.at(i);

            if (e->riding != nullptr) {
                if (e->riding->removed || e->riding->rider.lock() != e) {
                    e->riding->rider = std::weak_ptr<Entity>();
                    e->riding = nullptr;
                } else {
                    i++;
                    continue;
                }
            }

            if (!e->removed) {
#if !defined(_FINAL_BUILD)
                if (!(app.DebugSettingsOn() && app.GetMobsDontTickEnabled() &&
                      e->instanceof(eTYPE_MOB) && !e->instanceof(eTYPE_PLAYER)))
#endif
                {
                    tick(e);
                }
            }

            if (e->removed) {
                int xc = e->xChunk;
                int zc = e->zChunk;
                if (e->inChunk && hasChunk(xc, zc)) {
                    getChunk(xc, zc)->removeEntity(e);
                }
                
                

                
                
                
                auto it = find(entities.begin(), entities.end(), e);
                if (it != entities.end()) {
                    entities.erase(it);
                }

                entityRemoved(e);
            } else {
                i++;
            }
        }
    }

    {
        std::lock_guard<std::recursive_mutex> lock(m_tileEntityListCS);

        updatingTileEntities = true;
        for (auto it = tileEntityList.begin(); it != tileEntityList.end();) {
            std::shared_ptr<TileEntity> te =
                *it;  
            if (!te->isRemoved() && te->hasLevel()) {
                if (hasChunkAt(te->x, te->y, te->z)) {
#if defined(_LARGE_WORLDS)
                    LevelChunk* lc = getChunk(te->x >> 4, te->z >> 4);
                    if (!isClientSide || !lc->isUnloaded())
#endif
                    {
                        te->tick();
                    }
                }
            }

            if (te->isRemoved()) {
                it = tileEntityList.erase(it);
                if (hasChunk(te->x >> 4, te->z >> 4)) {
                    LevelChunk* lc = getChunk(te->x >> 4, te->z >> 4);
                    if (lc != nullptr)
                        lc->removeTileEntity(te->x & 15, te->y, te->z & 15);
                }
            } else {
                it++;
            }
        }
        updatingTileEntities = false;

        

        if (!tileEntitiesToUnload.empty()) {
            FRAME_PROFILE_SCOPE(TileEntityUnloadCleanup);

            for (auto it = tileEntityList.begin();
                 it != tileEntityList.end();) {
                if (tileEntitiesToUnload.find(*it) !=
                    tileEntitiesToUnload.end()) {
                    if (isClientSide) {
                        __debugbreak();
                    }
                    it = tileEntityList.erase(it);
                } else {
                    it++;
                }
            }
            tileEntitiesToUnload.clear();
        }

        if (!pendingTileEntities.empty()) {
            for (auto it = pendingTileEntities.begin();
                 it != pendingTileEntities.end(); it++) {
                std::shared_ptr<TileEntity> e = *it;
                if (!e->isRemoved()) {
                    if (find(tileEntityList.begin(), tileEntityList.end(), e) ==
                        tileEntityList.end()) {
                        tileEntityList.push_back(e);
                    }
                    if (hasChunk(e->x >> 4, e->z >> 4)) {
                        LevelChunk* lc = getChunk(e->x >> 4, e->z >> 4);
                        if (lc != nullptr)
                            lc->setTileEntity(e->x & 15, e->y, e->z & 15, e);
                    }

                    sendTileUpdated(e->x, e->y, e->z);
                }
            }
            pendingTileEntities.clear();
        }
    }
}

void Level::addAllPendingTileEntities(
    std::vector<std::shared_ptr<TileEntity> >& entities) {
    {
        std::lock_guard<std::recursive_mutex> lock(m_tileEntityListCS);
        if (updatingTileEntities) {
            for (auto it = entities.begin(); it != entities.end(); it++) {
                pendingTileEntities.push_back(*it);
            }
        } else {
            for (auto it = entities.begin(); it != entities.end(); it++) {
                tileEntityList.push_back(*it);
            }
        }
    }
}

void Level::tick(std::shared_ptr<Entity> e) { tick(e, true); }

void Level::tick(std::shared_ptr<Entity> e, bool actual) {
    int xc = Mth::floor(e->x);
    int zc = Mth::floor(e->z);
    int r = 32;
    if (actual && !hasChunksAt(xc - r, 0, zc - r, xc + r, 0, zc + r)) {
        return;
    }

    e->xOld = e->x;
    e->yOld = e->y;
    e->zOld = e->z;
    e->yRotO = e->yRot;
    e->xRotO = e->xRot;

    if (actual && e->inChunk) {
        e->tickCount++;
        if (e->riding != nullptr) {
            e->rideTick();
        } else {
            e->tick();
        }
    }

    

    if (!std::isfinite(e->x)) e->x = e->xOld;
    if (!std::isfinite(e->y)) e->y = e->yOld;
    if (!std::isfinite(e->z)) e->z = e->zOld;
    if (!std::isfinite(e->xRot)) e->xRot = e->xRotO;
    if (!std::isfinite(e->yRot)) e->yRot = e->yRotO;

    int xcn = Mth::floor(e->x / 16);
    int ycn = Mth::floor(e->y / 16);
    int zcn = Mth::floor(e->z / 16);

    if (!e->inChunk ||
        (e->xChunk != xcn || e->yChunk != ycn || e->zChunk != zcn)) {
        if (e->inChunk && hasChunk(e->xChunk, e->zChunk)) {
            getChunk(e->xChunk, e->zChunk)->removeEntity(e, e->yChunk);
        }

        if (hasChunk(xcn, zcn)) {
            e->inChunk = true;
            getChunk(xcn, zcn)->addEntity(e);
        } else {
            e->inChunk = false;
            
        }
    }

    if (actual && e->inChunk) {
        if (e->rider.lock() != nullptr) {
            if (e->rider.lock()->removed || e->rider.lock()->riding != e) {
                e->rider.lock()->riding = nullptr;
                e->rider = std::weak_ptr<Entity>();
            } else {
                tick(e->rider.lock());
            }
        }
    }
}

bool Level::isUnobstructed(AABB* aabb) { return isUnobstructed(aabb, nullptr); }

bool Level::isUnobstructed(AABB* aabb, std::shared_ptr<Entity> ignore) {
    std::vector<std::shared_ptr<Entity> > ents;
    getEntities(nullptr, aabb, ents);
    auto itEnd = ents.end();
    for (auto it = ents.begin(); it != itEnd; it++) {
        std::shared_ptr<Entity> e = *it;
        if (!e->removed && e->blocksBuilding && e != ignore) return false;
    }
    return true;
}

bool Level::containsAnyBlocks(AABB* box) {
    int x0 = Mth::floor(box->x0);
    int x1 = Mth::floor(box->x1 + 1);
    int y0 = Mth::floor(box->y0);
    int y1 = Mth::floor(box->y1 + 1);
    int z0 = Mth::floor(box->z0);
    int z1 = Mth::floor(box->z1 + 1);

    if (box->x0 < 0) x0--;
    if (box->y0 < 0) y0--;
    if (box->z0 < 0) z0--;

    for (int x = x0; x < x1; x++)
        for (int y = y0; y < y1; y++)
            for (int z = z0; z < z1; z++) {
                Tile* tile = Tile::tiles[getTile(x, y, z)];
                if (tile != nullptr) {
                    return true;
                }
            }
    return false;
}

bool Level::containsAnyLiquid(AABB* box) {
    int x0 = Mth::floor(box->x0);
    int x1 = Mth::floor(box->x1 + 1);
    int y0 = Mth::floor(box->y0);
    int y1 = Mth::floor(box->y1 + 1);
    int z0 = Mth::floor(box->z0);
    int z1 = Mth::floor(box->z1 + 1);

    if (box->x0 < 0) x0--;
    if (box->y0 < 0) y0--;
    if (box->z0 < 0) z0--;

    for (int x = x0; x < x1; x++)
        for (int y = y0; y < y1; y++)
            for (int z = z0; z < z1; z++) {
                Tile* tile = Tile::tiles[getTile(x, y, z)];
                if (tile != nullptr && tile->material->isLiquid()) {
                    return true;
                }
            }
    return false;
}






bool Level::containsAnyLiquid_NoLoad(AABB* box) {
    int x0 = Mth::floor(box->x0);
    int x1 = Mth::floor(box->x1 + 1);
    int y0 = Mth::floor(box->y0);
    int y1 = Mth::floor(box->y1 + 1);
    int z0 = Mth::floor(box->z0);
    int z1 = Mth::floor(box->z1 + 1);

    if (box->x0 < 0) x0--;
    if (box->y0 < 0) y0--;
    if (box->z0 < 0) z0--;

    for (int x = x0; x < x1; x++)
        for (int y = y0; y < y1; y++)
            for (int z = z0; z < z1; z++) {
                if (!hasChunkAt(x, y, z))
                    return true;  
                Tile* tile = Tile::tiles[getTile(x, y, z)];
                if (tile != nullptr && tile->material->isLiquid()) {
                    return true;
                }
            }
    return false;
}

bool Level::containsFireTile(AABB* box) {
    int x0 = Mth::floor(box->x0);
    int x1 = Mth::floor(box->x1 + 1);
    int y0 = Mth::floor(box->y0);
    int y1 = Mth::floor(box->y1 + 1);
    int z0 = Mth::floor(box->z0);
    int z1 = Mth::floor(box->z1 + 1);

    if (hasChunksAt(x0, y0, z0, x1, y1, z1)) {
        for (int x = x0; x < x1; x++)
            for (int y = y0; y < y1; y++)
                for (int z = z0; z < z1; z++) {
                    int t = getTile(x, y, z);

                    if (t == Tile::fire_Id || t == Tile::lava_Id ||
                        t == Tile::calmLava_Id)
                        return true;
                }
    }
    return false;
}

bool Level::checkAndHandleWater(AABB* box, Material* material,
                                std::shared_ptr<Entity> e) {
    int x0 = Mth::floor(box->x0);
    int x1 = Mth::floor(box->x1 + 1);

    int y0 = Mth::floor(box->y0);
    int y1 = Mth::floor(box->y1 + 1);

    int z0 = Mth::floor(box->z0);
    int z1 = Mth::floor(box->z1 + 1);

    if (!hasChunksAt(x0, y0, z0, x1, y1, z1)) {
        return false;
    }

    bool ok = false;
    Vec3 current(0, 0, 0);
    for (int x = x0; x < x1; x++) {
        for (int y = y0; y < y1; y++) {
            for (int z = z0; z < z1; z++) {
                Tile* tile = Tile::tiles[getTile(x, y, z)];
                if (tile != nullptr && tile->material == material) {
                    double yt0 =
                        y + 1 - LiquidTile::getHeight(getData(x, y, z));
                    if (y1 >= yt0) {
                        ok = true;
                        tile->handleEntityInside(this, x, y, z, e, &current);
                    }
                }
            }
        }
    }
    if (current.length() > 0 && e->isPushedByWater()) {
        current = current.normalize();
        double pow = 0.014;
        e->xd += current.x * pow;
        e->yd += current.y * pow;
        e->zd += current.z * pow;
    }
    return ok;
}

bool Level::containsMaterial(AABB* box, Material* material) {
    int x0 = Mth::floor(box->x0);
    int x1 = Mth::floor(box->x1 + 1);
    int y0 = Mth::floor(box->y0);
    int y1 = Mth::floor(box->y1 + 1);
    int z0 = Mth::floor(box->z0);
    int z1 = Mth::floor(box->z1 + 1);

    for (int x = x0; x < x1; x++) {
        for (int y = y0; y < y1; y++) {
            for (int z = z0; z < z1; z++) {
                Tile* tile = Tile::tiles[getTile(x, y, z)];
                if (tile != nullptr && tile->material == material) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool Level::containsLiquid(AABB* box, Material* material) {
    int x0 = Mth::floor(box->x0);
    int x1 = Mth::floor(box->x1 + 1);
    int y0 = Mth::floor(box->y0);
    int y1 = Mth::floor(box->y1 + 1);
    int z0 = Mth::floor(box->z0);
    int z1 = Mth::floor(box->z1 + 1);

    for (int x = x0; x < x1; x++) {
        for (int y = y0; y < y1; y++) {
            for (int z = z0; z < z1; z++) {
                Tile* tile = Tile::tiles[getTile(x, y, z)];
                if (tile != nullptr && tile->material == material) {
                    int data = getData(x, y, z);
                    double yh1 = y + 1;
                    if (data < 8) {
                        yh1 = y + 1 - data / 8.0;
                    }
                    if (yh1 >= box->y0) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

std::shared_ptr<Explosion> Level::explode(std::shared_ptr<Entity> source,
                                          double x, double y, double z, float r,
                                          bool destroyBlocks) {
    return explode(source, x, y, z, r, false, destroyBlocks);
}

std::shared_ptr<Explosion> Level::explode(std::shared_ptr<Entity> source,
                                          double x, double y, double z, float r,
                                          bool fire, bool destroyBlocks) {
    std::shared_ptr<Explosion> explosion =
        std::make_shared<Explosion>(this, source, x, y, z, r);
    explosion->fire = fire;
    explosion->destroyBlocks = destroyBlocks;
    explosion->explode();
    explosion->finalizeExplosion(true);
    return explosion;
}

float Level::getSeenPercent(Vec3* center, AABB* bb) {
    double xs = 1.0 / ((bb->x1 - bb->x0) * 2 + 1);
    double ys = 1.0 / ((bb->y1 - bb->y0) * 2 + 1);
    double zs = 1.0 / ((bb->z1 - bb->z0) * 2 + 1);
    int hits = 0;
    int count = 0;
    for (double xx = 0; xx <= 1;
         xx += xs)  
                    
        for (double yy = 0; yy <= 1; yy += ys)
            for (double zz = 0; zz <= 1; zz += zs) {
                double x = bb->x0 + (bb->x1 - bb->x0) * xx;
                double y = bb->y0 + (bb->y1 - bb->y0) * yy;
                double z = bb->z0 + (bb->z1 - bb->z0) * zz;
                Vec3 a(x, y, z);
                HitResult* res = clip(&a, center);
                if (res == nullptr) hits++;
                delete res;
                count++;
            }

    return hits / (float)count;
}

bool Level::extinguishFire(std::shared_ptr<Player> player, int x, int y, int z,
                           int face) {
    if (face == 0) y--;
    if (face == 1) y++;
    if (face == 2) z--;
    if (face == 3) z++;
    if (face == 4) x--;
    if (face == 5) x++;

    if (getTile(x, y, z) == Tile::fire_Id) {
        levelEvent(player, LevelEvent::SOUND_FIZZ, x, y, z, 0);
        removeTile(x, y, z);
        return true;
    }
    return false;
}








std::wstring Level::gatherStats() {
    wchar_t buf[64];
    {
        std::lock_guard<std::recursive_mutex> lock(m_entitiesCS);
        swprintf(buf, 64, L"All:%d", entities.size());
    }
    return std::wstring(buf);
}

std::wstring Level::gatherChunkSourceStats() {
    return chunkSource->gatherStats();
}

std::shared_ptr<TileEntity> Level::getTileEntity(int x, int y, int z) {
    if (y < minBuildHeight || y >= maxBuildHeight) {
        return nullptr;
    }
    std::shared_ptr<TileEntity> tileEntity = nullptr;

    if (updatingTileEntities) {
        {
            std::lock_guard<std::recursive_mutex> lock(m_tileEntityListCS);
            for (int i = 0; i < pendingTileEntities.size(); i++) {
                std::shared_ptr<TileEntity> e = pendingTileEntities.at(i);
                if (!e->isRemoved() && e->x == x && e->y == y && e->z == z) {
                    tileEntity = e;
                    break;
                }
            }
        }
    }

    if (tileEntity == nullptr) {
        LevelChunk* lc = getChunk(x >> 4, z >> 4);
        if (lc != nullptr) {
            tileEntity = lc->getTileEntity(x & 15, y, z & 15);
        }
    }

    if (tileEntity == nullptr) {
        {
            std::lock_guard<std::recursive_mutex> lock(m_tileEntityListCS);
            for (auto it = pendingTileEntities.begin();
                 it != pendingTileEntities.end(); it++) {
                std::shared_ptr<TileEntity> e = *it;

                if (!e->isRemoved() && e->x == x && e->y == y && e->z == z) {
                    tileEntity = e;
                    break;
                }
            }
        }
    }
    return tileEntity;
}

void Level::setTileEntity(int x, int y, int z,
                          std::shared_ptr<TileEntity> tileEntity) {
    if (tileEntity != nullptr && !tileEntity->isRemoved()) {
        {
            std::lock_guard<std::recursive_mutex> lock(m_tileEntityListCS);
            if (updatingTileEntities) {
                tileEntity->x = x;
                tileEntity->y = y;
                tileEntity->z = z;

                
                for (auto it = pendingTileEntities.begin();
                     it != pendingTileEntities.end();) {
                    std::shared_ptr<TileEntity> next = *it;
                    if (next->x == x && next->y == y && next->z == z) {
                        next->setRemoved();
                        it = pendingTileEntities.erase(it);
                    } else {
                        ++it;
                    }
                }

                pendingTileEntities.push_back(tileEntity);
            } else {
                tileEntityList.push_back(tileEntity);

                LevelChunk* lc = getChunk(x >> 4, z >> 4);
                if (lc != nullptr)
                    lc->setTileEntity(x & 15, y, z & 15, tileEntity);
            }
        }
    }
}

void Level::removeTileEntity(int x, int y, int z) {
    {
        std::lock_guard<std::recursive_mutex> lock(m_tileEntityListCS);
        std::shared_ptr<TileEntity> te = getTileEntity(x, y, z);
        if (te != nullptr && updatingTileEntities) {
            te->setRemoved();
            auto it = find(pendingTileEntities.begin(),
                           pendingTileEntities.end(), te);
            if (it != pendingTileEntities.end()) {
                pendingTileEntities.erase(it);
            }
        } else {
            if (te != nullptr) {
                auto it = find(pendingTileEntities.begin(),
                               pendingTileEntities.end(), te);
                if (it != pendingTileEntities.end()) {
                    pendingTileEntities.erase(it);
                }
                auto it2 =
                    find(tileEntityList.begin(), tileEntityList.end(), te);
                if (it2 != tileEntityList.end()) {
                    tileEntityList.erase(it2);
                }
            }
            LevelChunk* lc = getChunk(x >> 4, z >> 4);
            if (lc != nullptr) lc->removeTileEntity(x & 15, y, z & 15);
        }
    }
}

void Level::markForRemoval(std::shared_ptr<TileEntity> entity) {
    {
        std::lock_guard<std::recursive_mutex> lock(m_tileEntityListCS);
        tileEntitiesToUnload.insert(entity);
    }
}

bool Level::isSolidRenderTile(int x, int y, int z) {
    Tile* tile = Tile::tiles[getTile(x, y, z)];
    if (tile == nullptr) return false;

    
    
    
    
    
    
    
    
    
    

    if (tile->id == Tile::leaves_Id) {
        int axo[6] = {1, -1, 0, 0, 0, 0};
        int ayo[6] = {0, 0, 1, -1, 0, 0};
        int azo[6] = {0, 0, 0, 0, 1, -1};
        for (int i = 0; i < 6; i++) {
            int t = getTile(x + axo[i], y + ayo[i], z + azo[i]);
            if ((t != Tile::leaves_Id) && ((Tile::tiles[t] == nullptr) ||
                                           !Tile::tiles[t]->isSolidRender())) {
                return false;
            }
        }

        return true;
    }
    return tile->isSolidRender(!isClientSide);
}

bool Level::isSolidBlockingTile(int x, int y, int z) {
    return Tile::isSolidBlockingTile(getTile(x, y, z));
}







bool Level::isSolidBlockingTileInLoadedChunk(int x, int y, int z,
                                             bool valueIfNotLoaded) {
    if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE ||
        z >= MAX_LEVEL_SIZE) {
        return valueIfNotLoaded;
    }
    LevelChunk* chunk = chunkSource->getChunk(x >> 4, z >> 4);
    if (chunk == nullptr || chunk->isEmpty()) {
        return valueIfNotLoaded;
    }

    Tile* tile = Tile::tiles[getTile(x, y, z)];
    if (tile == nullptr) return false;
    return tile->material->isSolidBlocking() && tile->isCubeShaped();
}

bool Level::isFullAABBTile(int x, int y, int z) {
    int tile = getTile(x, y, z);
    if (tile == 0 || Tile::tiles[tile] == nullptr) {
        return false;
    }
    auto aabb = Tile::tiles[tile]->getAABB(this, x, y, z);
    return aabb.has_value() && aabb->getSize() >= 1;
}

bool Level::isTopSolidBlocking(int x, int y, int z) {
    
    Tile* tile = Tile::tiles[getTile(x, y, z)];
    return isTopSolidBlocking(tile, getData(x, y, z));
}

bool Level::isTopSolidBlocking(Tile* tile, int data) {
    if (tile == nullptr) return false;

    if (tile->material->isSolidBlocking() && tile->isCubeShaped()) return true;
    if (dynamic_cast<StairTile*>(tile) != nullptr) {
        return (data & StairTile::UPSIDEDOWN_BIT) == StairTile::UPSIDEDOWN_BIT;
    }
    if (dynamic_cast<HalfSlabTile*>(tile) != nullptr) {
        return (data & HalfSlabTile::TOP_SLOT_BIT) ==
               HalfSlabTile::TOP_SLOT_BIT;
    }
    if (dynamic_cast<HopperTile*>(tile) != nullptr) return true;
    if (dynamic_cast<TopSnowTile*>(tile) != nullptr)
        return (data & TopSnowTile::HEIGHT_MASK) == TopSnowTile::MAX_HEIGHT + 1;
    return false;
}

void Level::updateSkyBrightness() {
    int newDark = getOldSkyDarken(1);
    if (newDark != skyDarken) {
        skyDarken = newDark;
    }
}

void Level::setSpawnSettings(bool spawnEnemies, bool spawnFriendlies) {
    this->spawnEnemies = spawnEnemies;
    this->spawnFriendlies = spawnFriendlies;
}

void Level::tick() { tickWeather(); }

void Level::prepareWeather() {
    if (levelData->isRaining()) {
        rainLevel = 1;
        if (levelData->isThundering()) {
            thunderLevel = 1;
        }
    }
}

void Level::tickWeather() {
    if (dimension->hasCeiling) return;

#if !defined(_FINAL_BUILD)
    
    if (app.DebugSettingsOn()) {
        if (app.GetGameSettingsDebugMask(PlatformInput.GetPrimaryPad()) &
            (1L << eDebugSetting_DisableWeather)) {
            levelData->setThundering(false);
            levelData->setThunderTime(random->nextInt(TICKS_PER_DAY * 7) +
                                      TICKS_PER_DAY / 2);
            levelData->setRaining(false);
            levelData->setRainTime(random->nextInt(TICKS_PER_DAY * 7) +
                                   TICKS_PER_DAY / 2);
        }
    }
#endif

    int thunderTime = levelData->getThunderTime();
    if (thunderTime <= 0) {
        if (levelData->isThundering()) {
            levelData->setThunderTime(random->nextInt(20 * 60 * 10) +
                                      20 * 60 * 3);
        } else {
            levelData->setThunderTime(random->nextInt(TICKS_PER_DAY * 7) +
                                      TICKS_PER_DAY / 2);
        }
    } else {
        thunderTime--;
        levelData->setThunderTime(thunderTime);
        if (thunderTime <= 0) {
            levelData->setThundering(!levelData->isThundering());
        }
    }

    int rainTime = levelData->getRainTime();
    if (rainTime <= 0) {
        if (levelData->isRaining()) {
            levelData->setRainTime(random->nextInt(TICKS_PER_DAY / 2) +
                                   TICKS_PER_DAY / 2);
        } else {
            levelData->setRainTime(random->nextInt(TICKS_PER_DAY * 7) +
                                   TICKS_PER_DAY / 2);
        }
    } else {
        rainTime--;
        levelData->setRainTime(rainTime);
        if (rainTime <= 0) {
            levelData->setRaining(!levelData->isRaining());
        }
        



    }

    oRainLevel = rainLevel;
    if (levelData->isRaining()) {
        rainLevel += 0.01;
    } else {
        rainLevel -= 0.01;
    }
    if (rainLevel < 0) rainLevel = 0;
    if (rainLevel > 1) rainLevel = 1;

    oThunderLevel = thunderLevel;
    if (levelData->isThundering()) {
        thunderLevel += 0.01;
    } else {
        thunderLevel -= 0.01;
    }
    if (thunderLevel < 0) thunderLevel = 0;
    if (thunderLevel > 1) thunderLevel = 1;
}

void Level::toggleDownfall() {
    
    levelData->setRainTime(1);
}

void Level::buildAndPrepareChunksToPoll() {
    
    
    
    
    
    int playerCount = (int)players.size();
    int* xx = new int[playerCount];
    int* zz = new int[playerCount];
    for (int i = 0; i < playerCount; i++) {
        std::shared_ptr<Player> player = players[i];
        xx[i] = Mth::floor(player->x / 16);
        zz[i] = Mth::floor(player->z / 16);
        chunksToPoll.insert(ChunkPos(xx[i], zz[i]));
    }

    for (int r = 1; r <= 9; r++) {
        for (int l = 0; l < (r * 2); l++) {
            for (int i = 0; i < playerCount; i++) {
                chunksToPoll.insert(ChunkPos((xx[i] - r) + l, (zz[i] - r)));
                chunksToPoll.insert(ChunkPos((xx[i] + r), (zz[i] - r) + l));
                chunksToPoll.insert(ChunkPos((xx[i] + r) - l, (zz[i] + r)));
                chunksToPoll.insert(ChunkPos((xx[i] - r), (zz[i] + r) - l));
            }
        }
    }
    delete[] xx;
    delete[] zz;

    if (delayUntilNextMoodSound > 0) delayUntilNextMoodSound--;

    
    
    
    
    
    
    
    
    
    
    
    
}

void Level::tickClientSideTiles(int xo, int zo, LevelChunk* lc) {
    

    if (delayUntilNextMoodSound == 0 && !isClientSide) {
        randValue = randValue * 3 + addend;
        int val = (randValue >> 2);
        int x = (val & 15);
        int z = ((val >> 8) & 15);
        int y = ((val >> 16) & genDepthMinusOne);

        int id = lc->getTile(x, y, z);
        x += xo;
        z += zo;
        if (id == 0 &&
            this->getDaytimeRawBrightness(x, y, z) <= random->nextInt(8) &&
            getBrightness(LightLayer::Sky, x, y, z) <= 0) {
            std::shared_ptr<Player> player =
                getNearestPlayer(x + 0.5, y + 0.5, z + 0.5, 8);
            if (player != nullptr &&
                player->distanceToSqr(x + 0.5, y + 0.5, z + 0.5) > 2 * 2) {
                
                
                this->playSound(x + 0.5, y + 0.5, z + 0.5,
                                eSoundType_AMBIENT_CAVE_CAVE, 0.7f,
                                0.8f + random->nextFloat() * 0.2f);
                delayUntilNextMoodSound =
                    random->nextInt(SharedConstants::TICKS_PER_SECOND * 60 *
                                    10) +
                    SharedConstants::TICKS_PER_SECOND * 60 * 5;
            }
        }
    }

    
    
}

void Level::tickTiles() { buildAndPrepareChunksToPoll(); }

bool Level::shouldFreezeIgnoreNeighbors(int x, int y, int z) {
    return shouldFreeze(x, y, z, false);
}

bool Level::shouldFreeze(int x, int y, int z) {
    return shouldFreeze(x, y, z, true);
}

bool Level::shouldFreeze(int x, int y, int z, bool checkNeighbors) {
    Biome* biome = getBiome(x, z);
    float temp = biome->getTemperature();
    if (temp > 0.15f) return false;

    if (y >= 0 && y < maxBuildHeight &&
        getBrightness(LightLayer::Block, x, y, z) < 10) {
        int current = getTile(x, y, z);
        if ((current == Tile::calmWater_Id || current == Tile::water_Id) &&
            getData(x, y, z) == 0) {
            if (!checkNeighbors) return true;

            bool surroundedByWater = true;
            if (surroundedByWater &&
                getMaterial(x - 1, y, z) != Material::water)
                surroundedByWater = false;
            if (surroundedByWater &&
                getMaterial(x + 1, y, z) != Material::water)
                surroundedByWater = false;
            if (surroundedByWater &&
                getMaterial(x, y, z - 1) != Material::water)
                surroundedByWater = false;
            if (surroundedByWater &&
                getMaterial(x, y, z + 1) != Material::water)
                surroundedByWater = false;
            if (!surroundedByWater) return true;
        }
    }
    return false;
}

bool Level::shouldSnow(int x, int y, int z) {
    Biome* biome = getBiome(x, z);
    float temp = biome->getTemperature();
    if (temp > 0.15f) return false;

    if (y >= 0 && y < maxBuildHeight &&
        getBrightness(LightLayer::Block, x, y, z) < 10) {
        int below = getTile(x, y - 1, z);
        int current = getTile(x, y, z);
        if (current == 0) {
            if (Tile::topSnow->mayPlace(this, x, y, z) &&
                (below != 0 && below != Tile::ice_Id &&
                 Tile::tiles[below]->material->blocksMotion())) {
                return true;
            }
        }
    }

    return false;
}

void Level::checkLight(
    int x, int y, int z, bool force,
    bool rootOnlyEmissive)  
{
    if (!dimension->hasCeiling)
        checkLight(LightLayer::Sky, x, y, z, force, false);
    checkLight(LightLayer::Block, x, y, z, force, rootOnlyEmissive);
}

int Level::getExpectedLight(lightCache_t* cache, int x, int y, int z,
                            LightLayer::variety layer, bool propagatedOnly) {
    if (layer == LightLayer::Sky && canSeeSky(x, y, z)) return MAX_BRIGHTNESS;
    int id = getTile(x, y, z);
    int result = layer == LightLayer::Sky ? 0 : Tile::lightEmission[id];
    int block = Tile::lightBlock[id];
    if (block >= MAX_BRIGHTNESS && Tile::lightEmission[id] > 0) block = 1;
    if (block < 1) block = 1;
    if (block >= MAX_BRIGHTNESS) {
        return propagatedOnly ? 0 : getEmissionCached(cache, 0, x, y, z);
    }

    if (result >= MAX_BRIGHTNESS - 1) return result;

    for (int face = 0; face < 6; face++) {
        int xx = x + Facing::STEP_X[face];
        int yy = y + Facing::STEP_Y[face];
        int zz = z + Facing::STEP_Z[face];
        int brightness = getBrightnessCached(cache, layer, xx, yy, zz) - block;

        if (brightness > result) result = brightness;
        if (result >= MAX_BRIGHTNESS - 1) return result;
    }

    return result;
}



void Level::checkLight(LightLayer::variety layer, int xc, int yc, int zc,
                       bool force, bool rootOnlyEmissive) {
    lightCache_t* cache = m_tlsLightCache;
    uint64_t cacheUse = 0;

    if (force) {
        
        
        if (!hasChunksAt(xc, yc, zc, 0)) return;
    } else {
        
        if (!hasChunksAt(xc, yc, zc, 17)) return;
    }

    {
        std::lock_guard<std::recursive_mutex> lock(m_checkLightCS);

        initCachePartial(cache, xc, yc, zc);

        
        
        

        int* toCheck;
        if (cache == nullptr) {
            toCheck = toCheckLevel;
        } else {
            toCheck = (int*)(cache + (16 * 16 * 16));
        }

        int checkedPosition = 0;
        int toCheckCount = 0;
        

        
        int minXZ = -(dimension->getXZSize() * 16) / 2;
        int maxXZ = (dimension->getXZSize() * 16) / 2 - 1;
        if ((xc > maxXZ) || (xc < minXZ) || (zc > maxXZ) || (zc < minXZ)) {
            return;
        }

        
        
        
        
        

        {
            int centerCurrent = getBrightnessCached(cache, layer, xc, yc, zc);
            int centerExpected =
                getExpectedLight(cache, xc, yc, zc, layer, false);

            if (centerExpected != centerCurrent && cache) {
                initCacheComplete(cache, xc, yc, zc);
            }

            if (centerExpected > centerCurrent) {
                toCheck[toCheckCount++] = 32 | (32 << 6) | (32 << 12);
            } else if (centerExpected < centerCurrent) {
                
                
                
                
                
                
                
                
                
                
                
                int tcn = 0;
                if (layer == LightLayer::Block || true) {
                    toCheck[toCheckCount++] =
                        32 | (32 << 6) | (32 << 12) | (centerCurrent << 18);
                    while (checkedPosition < toCheckCount) {
                        int p = toCheck[checkedPosition++];
                        int x = ((p) & 63) - 32 + xc;
                        int y = ((p >> 6) & 63) - 32 + yc;
                        int z = ((p >> 12) & 63) - 32 + zc;
                        int expected = ((p >> 18) & 15);
                        int current =
                            getBrightnessCached(cache, layer, x, y, z);
                        if (current == expected) {
                            setBrightnessCached(cache, &cacheUse, layer, x, y,
                                                z, 0);
                            
                            
                            if (expected > 0) {
                                int xd = Mth::abs(x - xc);
                                int yd = Mth::abs(y - yc);
                                int zd = Mth::abs(z - zc);
                                if (xd + yd + zd < 17) {
                                    bool edge = false;
                                    for (int face = 0; face < 6; face++) {
                                        int xx = x + Facing::STEP_X[face];
                                        int yy = y + Facing::STEP_Y[face];
                                        int zz = z + Facing::STEP_Z[face];

                                        
                                        
                                        
                                        
                                        if ((xx > maxXZ) || (xx < minXZ) ||
                                            (zz > maxXZ) || (zz < minXZ))
                                            continue;
                                        if ((yy < 0) || (yy >= maxBuildHeight))
                                            continue;

                                        
                                        
                                        int block = std::max(
                                            1, getBlockingCached(cache, layer,
                                                                 nullptr, xx,
                                                                 yy, zz));
                                        current = getBrightnessCached(
                                            cache, layer, xx, yy, zz);
                                        if ((current == expected - block) &&
                                            (toCheckCount <
                                             (32 * 32 *
                                              32)))  
                                                     
                                        {
                                            toCheck[toCheckCount++] =
                                                (xx - xc + 32) |
                                                ((yy - yc + 32) << 6) |
                                                ((zz - zc + 32) << 12) |
                                                ((expected - block) << 18);
                                        } else {
                                            
                                            
                                            
                                            if (current > (expected - block)) {
                                                edge = true;
                                            }
                                        }
                                    }
                                    
                                    
                                    
                                    
                                    
                                    if (edge == true) {
                                        toCheck[tcn++] = p;
                                    }
                                }
                            }
                        }
                    }
                }
                checkedPosition = 0;
                
                
                toCheckCount =
                    tcn;  
                          
                          
                          
            }
        }

        while (checkedPosition < toCheckCount) {
            int p = toCheck[checkedPosition++];
            int x = ((p) & 63) - 32 + xc;
            int y = ((p >> 6) & 63) - 32 + yc;
            int z = ((p >> 12) & 63) - 32 + zc;

            
            
            
            
            if (force) {
                if (!hasChunkAt(x, y, z)) {
                    continue;
                }
            }
            int current = getBrightnessCached(cache, layer, x, y, z);

            
            
            bool propagatedOnly = false;
            if (layer == LightLayer::Block) {
                if (rootOnlyEmissive) {
                    propagatedOnly = (x != xc) || (y != yc) || (z != zc);
                }
            }
            int expected =
                getExpectedLight(cache, x, y, z, layer, propagatedOnly);

            if (expected != current) {
                setBrightnessCached(cache, &cacheUse, layer, x, y, z, expected);

                if (expected > current) {
                    int xd = abs(x - xc);
                    int yd = abs(y - yc);
                    int zd = abs(z - zc);
                    bool withinBounds =
                        toCheckCount <
                        (32 * 32 * 32) -
                            6;  
                    if (xd + yd + zd < 17 && withinBounds) {
                        
                        
                        
                        if ((x - 1) >= minXZ) {
                            if (getBrightnessCached(cache, layer, x - 1, y, z) <
                                expected)
                                toCheck[toCheckCount++] =
                                    (((x - 1 - xc) + 32)) +
                                    (((y - yc) + 32) << 6) +
                                    (((z - zc) + 32) << 12);
                        }
                        if ((x + 1) <= maxXZ) {
                            if (getBrightnessCached(cache, layer, x + 1, y, z) <
                                expected)
                                toCheck[toCheckCount++] =
                                    (((x + 1 - xc) + 32)) +
                                    (((y - yc) + 32) << 6) +
                                    (((z - zc) + 32) << 12);
                        }
                        if ((y - 1) >= 0) {
                            if (getBrightnessCached(cache, layer, x, y - 1, z) <
                                expected)
                                toCheck[toCheckCount++] =
                                    (((x - xc) + 32)) +
                                    (((y - 1 - yc) + 32) << 6) +
                                    (((z - zc) + 32) << 12);
                        }
                        if ((y + 1) < maxBuildHeight) {
                            if (getBrightnessCached(cache, layer, x, y + 1, z) <
                                expected)
                                toCheck[toCheckCount++] =
                                    (((x - xc) + 32)) +
                                    (((y + 1 - yc) + 32) << 6) +
                                    (((z - zc) + 32) << 12);
                        }
                        if ((z - 1) >= minXZ) {
                            if (getBrightnessCached(cache, layer, x, y, z - 1) <
                                expected)
                                toCheck[toCheckCount++] =
                                    (((x - xc) + 32)) + (((y - yc) + 32) << 6) +
                                    (((z - 1 - zc) + 32) << 12);
                        }
                        if ((z + 1) <= maxXZ) {
                            if (getBrightnessCached(cache, layer, x, y, z + 1) <
                                expected)
                                toCheck[toCheckCount++] =
                                    (((x - xc) + 32)) + (((y - yc) + 32) << 6) +
                                    (((z + 1 - zc) + 32) << 12);
                        }
                    }
                }
            }
        }
        

        flushCache(cache, cacheUse, layer);
    }
}

bool Level::tickPendingTicks(bool force) { return false; }

std::vector<TickNextTickData>* Level::fetchTicksInChunk(LevelChunk* chunk,
                                                        bool remove) {
    return nullptr;
}

void Level::getEntities(std::shared_ptr<Entity> except, AABB* bb,
                        std::vector<std::shared_ptr<Entity> >& out) {
    getEntities(except, bb, nullptr, out);
}

void Level::getEntities(std::shared_ptr<Entity> except, AABB* bb,
                        const EntitySelector* selector,
                        std::vector<std::shared_ptr<Entity> >& out) {
    out.clear();
    int xc0 = Mth::floor((bb->x0 - 2) / 16);
    int xc1 = Mth::floor((bb->x1 + 2) / 16);
    int zc0 = Mth::floor((bb->z0 - 2) / 16);
    int zc1 = Mth::floor((bb->z1 + 2) / 16);

    for (int xc = xc0; xc <= xc1; xc++)
        for (int zc = zc0; zc <= zc1; zc++) {
            if (hasChunk(xc, zc)) {
                getChunk(xc, zc)->getEntities(except, bb, out, selector);
            }
        }
}

std::vector<std::shared_ptr<Entity> >* Level::getEntitiesOfClass(
    const std::type_info& baseClass, AABB* bb) {
    return getEntitiesOfClass(baseClass, bb, nullptr);
}

std::vector<std::shared_ptr<Entity> >* Level::getEntitiesOfClass(
    const std::type_info& baseClass, AABB* bb, const EntitySelector* selector) {
    int xc0 = Mth::floor((bb->x0 - 2) / 16);
    int xc1 = Mth::floor((bb->x1 + 2) / 16);
    int zc0 = Mth::floor((bb->z0 - 2) / 16);
    int zc1 = Mth::floor((bb->z1 + 2) / 16);
    std::vector<std::shared_ptr<Entity> >* es =
        new std::vector<std::shared_ptr<Entity> >();

    for (int xc = xc0; xc <= xc1; xc++) {
        for (int zc = zc0; zc <= zc1; zc++) {
            if (hasChunk(xc, zc)) {
                getChunk(xc, zc)->getEntitiesOfClass(baseClass, bb, *es,
                                                     selector);
            }
        }
    }

    return es;
}

std::shared_ptr<Entity> Level::getClosestEntityOfClass(
    const std::type_info& baseClass, AABB* bb, std::shared_ptr<Entity> source) {
    std::vector<std::shared_ptr<Entity> >* entities =
        getEntitiesOfClass(baseClass, bb);
    std::shared_ptr<Entity> closest = nullptr;
    double closestDistSqr = std::numeric_limits<double>::max();
    
    for (auto it = entities->begin(); it != entities->end(); ++it) {
        std::shared_ptr<Entity> entity = *it;
        if (entity == source) continue;
        double distSqr = source->distanceToSqr(entity);
        if (distSqr > closestDistSqr) continue;
        closest = entity;
        closestDistSqr = distSqr;
    }
    delete entities;
    return closest;
}

std::vector<std::shared_ptr<Entity> > Level::getAllEntities() {
    std::lock_guard<std::recursive_mutex> lock(m_entitiesCS);
    std::vector<std::shared_ptr<Entity> > retVec = entities;
    return retVec;
}

void Level::tileEntityChanged(int x, int y, int z,
                              std::shared_ptr<TileEntity> te) {
    if (this->hasChunkAt(x, y, z)) {
        getChunkAt(x, z)->markUnsaved();
    }
}





unsigned int Level::countInstanceOf(
    eINSTANCEOF clas, bool singleType,
    unsigned int* protectedCount ,
    unsigned int* couldWanderCount ) {
    unsigned int count = 0;
    if (protectedCount) *protectedCount = 0;
    if (couldWanderCount) *couldWanderCount = 0;
    {
        std::lock_guard<std::recursive_mutex> lock(m_entitiesCS);
        auto itEnd = entities.end();
        for (auto it = entities.begin(); it != itEnd; it++) {
            std::shared_ptr<Entity> e = *it;  
            if (singleType) {
                if (e->GetType() == clas) {
                    if (protectedCount && e->isDespawnProtected()) {
                        (*protectedCount)++;
                    }

                    if (couldWanderCount && e->couldWander()) {
                        (*couldWanderCount)++;
                    }

                    count++;
                }
            } else {
                if (e->instanceof(clas)) count++;
            }
        }
    }

    return count;
}

unsigned int Level::countInstanceOfInRange(eINSTANCEOF clas, bool singleType,
                                           int range, int x, int y, int z) {
    unsigned int count = 0;
    {
        std::lock_guard<std::recursive_mutex> lock(m_entitiesCS);
        auto itEnd = entities.end();
        for (auto it = entities.begin(); it != itEnd; it++) {
            std::shared_ptr<Entity> e = *it;  

            float sd = e->distanceTo(x, y, z);
            if (sd * sd > range * range) {
                continue;
            }

            if (singleType) {
                if (e->GetType() == clas) {
                    count++;
                }
            } else {
                if (e->instanceof(clas)) count++;
            }
        }
    }

    return count;
}

void Level::addEntities(std::vector<std::shared_ptr<Entity> >* list) {
    
    {
        std::lock_guard<std::recursive_mutex> lock(m_entitiesCS);
        entities.insert(entities.end(), list->begin(), list->end());
        auto itEnd = list->end();
        bool deleteDragons = false;
        for (auto it = list->begin(); it != itEnd; it++) {
            entityAdded(*it);

            
            
            if ((*it)->GetType() == eTYPE_ENDERDRAGON) {
                deleteDragons = true;
            }
        }

        if (deleteDragons) {
            deleteDragons = false;
            for (auto it = entities.begin(); it != entities.end(); ++it) {
                
                
                if ((*it)->GetType() == eTYPE_ENDERDRAGON) {
                    if (deleteDragons) {
                        (*it)->remove();
                    } else {
                        deleteDragons = true;
                    }
                }
            }
        }
    }
}

void Level::removeEntities(std::vector<std::shared_ptr<Entity> >* list) {
    
    entitiesToRemove.insert(entitiesToRemove.end(), list->begin(), list->end());
}

bool Level::mayPlace(int tileId, int x, int y, int z, bool ignoreEntities,
                     int face, std::shared_ptr<Entity> ignoreEntity,
                     std::shared_ptr<ItemInstance> item) {
    int targetType = getTile(x, y, z);
    Tile* targetTile = Tile::tiles[targetType];

    Tile* tile = Tile::tiles[tileId];

    auto aabb = tile->getAABB(this, x, y, z);
    if (ignoreEntities) aabb = std::nullopt;
    if (aabb.has_value() && !isUnobstructed(&*aabb, ignoreEntity)) return false;
    if (targetTile != nullptr &&
        (targetTile == Tile::water || targetTile == Tile::calmWater ||
         targetTile == Tile::lava || targetTile == Tile::calmLava ||
         targetTile == Tile::fire || targetTile->material->isReplaceable())) {
        targetTile = nullptr;
    }
    if (targetTile != nullptr && targetTile->material == Material::decoration &&
        tile == Tile::anvil)
        return true;
    if (tileId > 0 && targetTile == nullptr) {
        if (tile->mayPlace(this, x, y, z, face, item)) {
            return true;
        }
    }
    return false;
}

int Level::getSeaLevel() { return seaLevel; }

Path* Level::findPath(std::shared_ptr<Entity> from, std::shared_ptr<Entity> to,
                      float maxDist, bool canPassDoors, bool canOpenDoors,
                      bool avoidWater, bool canFloat) {
    int x = Mth::floor(from->x);
    int y = Mth::floor(from->y + 1);
    int z = Mth::floor(from->z);

    int r = (int)(maxDist + 16);
    int x1 = x - r;
    int y1 = y - r;
    int z1 = z - r;
    int x2 = x + r;
    int y2 = y + r;
    int z2 = z + r;
    Region region = Region(this, x1, y1, z1, x2, y2, z2, 0);
    Path* path =
        (PathFinder(&region, canPassDoors, canOpenDoors, avoidWater, canFloat))
            .findPath(from.get(), to.get(), maxDist);
    return path;
}

Path* Level::findPath(std::shared_ptr<Entity> from, int xBest, int yBest,
                      int zBest, float maxDist, bool canPassDoors,
                      bool canOpenDoors, bool avoidWater, bool canFloat) {
    int x = Mth::floor(from->x);
    int y = Mth::floor(from->y);
    int z = Mth::floor(from->z);

    int r = (int)(maxDist + 8);
    int x1 = x - r;
    int y1 = y - r;
    int z1 = z - r;
    int x2 = x + r;
    int y2 = y + r;
    int z2 = z + r;
    Region region = Region(this, x1, y1, z1, x2, y2, z2, 0);
    Path* path =
        (PathFinder(&region, canPassDoors, canOpenDoors, avoidWater, canFloat))
            .findPath(from.get(), xBest, yBest, zBest, maxDist);
    return path;
}

int Level::getDirectSignal(int x, int y, int z, int dir) {
    int t = getTile(x, y, z);
    if (t == 0) return Redstone::SIGNAL_NONE;
    return Tile::tiles[t]->getDirectSignal(this, x, y, z, dir);
}

int Level::getDirectSignalTo(int x, int y, int z) {
    int result = Redstone::SIGNAL_NONE;
    result = std::max(result, getDirectSignal(x, y - 1, z, 0));
    if (result >= Redstone::SIGNAL_MAX) return result;
    result = std::max(result, getDirectSignal(x, y + 1, z, 1));
    if (result >= Redstone::SIGNAL_MAX) return result;
    result = std::max(result, getDirectSignal(x, y, z - 1, 2));
    if (result >= Redstone::SIGNAL_MAX) return result;
    result = std::max(result, getDirectSignal(x, y, z + 1, 3));
    if (result >= Redstone::SIGNAL_MAX) return result;
    result = std::max(result, getDirectSignal(x - 1, y, z, 4));
    if (result >= Redstone::SIGNAL_MAX) return result;
    result = std::max(result, getDirectSignal(x + 1, y, z, 5));
    if (result >= Redstone::SIGNAL_MAX) return result;
    return result;
}

bool Level::hasSignal(int x, int y, int z, int dir) {
    return getSignal(x, y, z, dir) > Redstone::SIGNAL_NONE;
}

int Level::getSignal(int x, int y, int z, int dir) {
    if (isSolidBlockingTile(x, y, z)) {
        return getDirectSignalTo(x, y, z);
    }
    int t = getTile(x, y, z);
    if (t == 0) return Redstone::SIGNAL_NONE;
    return Tile::tiles[t]->getSignal(this, x, y, z, dir);
}

bool Level::hasNeighborSignal(int x, int y, int z) {
    if (getSignal(x, y - 1, z, 0) > 0) return true;
    if (getSignal(x, y + 1, z, 1) > 0) return true;
    if (getSignal(x, y, z - 1, 2) > 0) return true;
    if (getSignal(x, y, z + 1, 3) > 0) return true;
    if (getSignal(x - 1, y, z, 4) > 0) return true;
    if (getSignal(x + 1, y, z, 5) > 0) return true;
    return false;
}

int Level::getBestNeighborSignal(int x, int y, int z) {
    int best = Redstone::SIGNAL_NONE;

    for (int i = 0; i < 6; i++) {
        int signal = getSignal(x + Facing::STEP_X[i], y + Facing::STEP_Y[i],
                               z + Facing::STEP_Z[i], i);

        if (signal >= Redstone::SIGNAL_MAX) return Redstone::SIGNAL_MAX;
        if (signal > best) best = signal;
    }

    return best;
}


std::shared_ptr<Player> Level::getNearestPlayer(std::shared_ptr<Entity> source,
                                                double maxDist,
                                                double maxYDist ) {
    return getNearestPlayer(source->x, source->y, source->z, maxDist, maxYDist);
}


std::shared_ptr<Player> Level::getNearestPlayer(double x, double y, double z,
                                                double maxDist,
                                                double maxYDist ) {
    double best = -1;
    std::shared_ptr<Player> result = nullptr;
    auto itEnd = players.end();
    for (auto it = players.begin(); it != itEnd; it++) {
        std::shared_ptr<Player> p = *it;  
        double dist = p->distanceToSqr(x, y, z);

        
        if (maxYDist > 0 && abs(p->y - y) > maxYDist) continue;

        
        if ((maxDist < 0 || dist < maxDist * maxDist) &&
            (best == -1 || dist < best) && p->isAlive()) {
            best = dist;
            result = p;
        }
    }
    return result;
}

std::shared_ptr<Player> Level::getNearestPlayer(double x, double z,
                                                double maxDist) {
    double best = -1;
    std::shared_ptr<Player> result = nullptr;
    auto itEnd = players.end();
    for (auto it = players.begin(); it != itEnd; it++) {
        std::shared_ptr<Player> p = *it;
        double dist = p->distanceToSqr(x, p->y, z);
        if ((maxDist < 0 || dist < maxDist * maxDist) &&
            (best == -1 || dist < best)) {
            best = dist;
            result = p;
        }
    }
    return result;
}

std::shared_ptr<Player> Level::getNearestAttackablePlayer(
    std::shared_ptr<Entity> source, double maxDist) {
    return getNearestAttackablePlayer(source->x, source->y, source->z, maxDist);
}

std::shared_ptr<Player> Level::getNearestAttackablePlayer(double x, double y,
                                                          double z,
                                                          double maxDist) {
    double best = -1;

    std::shared_ptr<Player> result = nullptr;
    auto itEnd = players.end();
    for (auto it = players.begin(); it != itEnd; it++) {
        std::shared_ptr<Player> p = *it;

        
        if (p->abilities.invulnerable || !p->isAlive() ||
            p->hasInvisiblePrivilege()) {
            continue;
        }

        double dist = p->distanceToSqr(x, y, z);
        double visibleDist = maxDist;

        
        
        if (p->isSneaking()) {
            visibleDist *= .8f;
        }
        if (p->isInvisible()) {
            float coverPercentage = p->getArmorCoverPercentage();
            if (coverPercentage < .1f) {
                coverPercentage = .1f;
            }
            visibleDist *= (.7f * coverPercentage);
        }

        if ((visibleDist < 0 || dist < visibleDist * visibleDist) &&
            (best == -1 || dist < best)) {
            best = dist;
            result = p;
        }
    }
    return result;
}

std::shared_ptr<Player> Level::getPlayerByName(const std::wstring& name) {
    auto itEnd = players.end();
    for (auto it = players.begin(); it != itEnd; it++) {
        if (name.compare((*it)->getName()) == 0) {
            return *it;  
        }
    }
    return std::shared_ptr<Player>();
}

std::shared_ptr<Player> Level::getPlayerByUUID(const std::wstring& name) {
    auto itEnd = players.end();
    for (auto it = players.begin(); it != itEnd; it++) {
        if (name.compare((*it)->getUUID()) == 0) {
            return *it;  
        }
    }
    return std::shared_ptr<Player>();
}


std::vector<uint8_t> Level::getBlocksAndData(int x, int y, int z, int xs,
                                             int ys, int zs,
                                             bool includeLighting ) {
    std::vector<uint8_t> result(xs * ys * zs * 5 / 2);
    int xc0 = x >> 4;
    int zc0 = z >> 4;
    int xc1 = (x + xs - 1) >> 4;
    int zc1 = (z + zs - 1) >> 4;

    int p = 0;

    int y0 = y;
    int y1 = y + ys;
    if (y0 < 0) y0 = 0;
    if (y1 > Level::maxBuildHeight) y1 = Level::maxBuildHeight;
    for (int xc = xc0; xc <= xc1; xc++) {
        int x0 = x - xc * 16;
        int x1 = x + xs - xc * 16;
        if (x0 < 0) x0 = 0;
        if (x1 > 16) x1 = 16;
        for (int zc = zc0; zc <= zc1; zc++) {
            int z0 = z - zc * 16;
            int z1 = z + zs - zc * 16;
            if (z0 < 0) z0 = 0;
            if (z1 > 16) z1 = 16;
            p = getChunk(xc, zc)->getBlocksAndData(&result, x0, y0, z0, x1, y1,
                                                   z1, p, includeLighting);
        }
    }

    return result;
}


void Level::setBlocksAndData(int x, int y, int z, int xs, int ys, int zs,
                             std::vector<uint8_t>& data,
                             bool includeLighting ) {
    int xc0 = x >> 4;
    int zc0 = z >> 4;
    int xc1 = (x + xs - 1) >> 4;
    int zc1 = (z + zs - 1) >> 4;

    int p = 0;

    int y0 = y;
    int y1 = y + ys;
    if (y0 < 0) y0 = 0;
    if (y1 > Level::maxBuildHeight) y1 = Level::maxBuildHeight;
    for (int xc = xc0; xc <= xc1; xc++) {
        int x0 = x - xc * 16;
        int x1 = x + xs - xc * 16;
        if (x0 < 0) x0 = 0;
        if (x1 > 16) x1 = 16;
        for (int zc = zc0; zc <= zc1; zc++) {
            int z0 = z - zc * 16;
            int z1 = z + zs - zc * 16;
            if (z0 < 0) z0 = 0;
            if (z1 > 16) z1 = 16;
            LevelChunk* lc = getChunk(xc, zc);
            
            
            
            
            
            bool forceUnshare = false;
            if (g_NetworkManager.IsHost() && isClientSide) {
                forceUnshare =
                    lc->testSetBlocksAndData(data, x0, y0, z0, x1, y1, z1, p);
            }
            if (forceUnshare) {
                int size = (x1 - x0) * (y1 - y0) * (z1 - z0);
                lc->stopSharingTilesAndData();
            }
            if (p < data.size())
                p = lc->setBlocksAndData(data, x0, y0, z0, x1, y1, z1, p,
                                         includeLighting);
            setTilesDirty(xc * 16 + x0, y0, zc * 16 + z0, xc * 16 + x1, y1,
                          zc * 16 + z1);

            if (g_NetworkManager.IsHost() && isClientSide) {
                lc->startSharingTilesAndData();
            }
        }
    }
}

void Level::disconnect(bool sendDisconnect ) {}

void Level::checkSession() { levelStorage->checkSession(); }

void Level::setGameTime(int64_t time) {
    
    if (time != 0)  
                    
    {
        
        
        int64_t timeDiff = time - levelData->getGameTime();

        if (timeDiff < 0) {
            timeDiff = 0;
        } else if (timeDiff > 100) {
            
            
            app.DebugPrintf(
                "Level::setTime: Massive time difference, ignoring for time "
                "passed stat (%lli)\n",
                timeDiff);
            timeDiff = 0;
        }

        
        if (timeDiff > 0 && levelData->getGameTime() != -1) {
            auto itEnd = players.end();
            for (std::vector<std::shared_ptr<Player> >::iterator it =
                     players.begin();
                 it != itEnd; it++) {
                (*it)->awardStat(GenericStats::timePlayed(),
                                 GenericStats::param_time(timeDiff));
            }
        }
    }

    levelData->setGameTime(time);
}

int64_t Level::getSeed() { return levelData->getSeed(); }

int64_t Level::getGameTime() { return levelData->getGameTime(); }

int64_t Level::getDayTime() { return levelData->getDayTime(); }

void Level::setDayTime(int64_t newTime) { levelData->setDayTime(newTime); }

Pos* Level::getSharedSpawnPos() {
    return new Pos(levelData->getXSpawn(), levelData->getYSpawn(),
                   levelData->getZSpawn());
}

void Level::setSpawnPos(int x, int y, int z) { levelData->setSpawn(x, y, z); }

void Level::setSpawnPos(Pos* spawnPos) {
    setSpawnPos(spawnPos->x, spawnPos->y, spawnPos->z);
}

void Level::ensureAdded(std::shared_ptr<Entity> entity) {
    int xc = Mth::floor(entity->x / 16);
    int zc = Mth::floor(entity->z / 16);
    int r = 2;
    for (int x = xc - r; x <= xc + r; x++) {
        for (int z = zc - r; z <= zc + r; z++) {
            getChunk(x, z);
        }
    }

    
    {
        std::lock_guard<std::recursive_mutex> lock(m_entitiesCS);
        if (find(entities.begin(), entities.end(), entity) == entities.end()) {
            entities.push_back(entity);
        }
    }
}

bool Level::mayInteract(std::shared_ptr<Player> player, int xt, int yt, int zt,
                        int content) {
    return true;
}

void Level::broadcastEntityEvent(std::shared_ptr<Entity> e, uint8_t event) {}

ChunkSource* Level::getChunkSource() { return chunkSource; }

void Level::tileEvent(int x, int y, int z, int tile, int b0, int b1) {
    if (tile > 0) Tile::tiles[tile]->triggerEvent(this, x, y, z, b0, b1);
}

LevelStorage* Level::getLevelStorage() { return levelStorage.get(); }

LevelData* Level::getLevelData() { return levelData; }

GameRules* Level::getGameRules() { return levelData->getGameRules(); }

void Level::updateSleepingPlayerList() {}

float Level::getThunderLevel(float a) {
    return (oThunderLevel + (thunderLevel - oThunderLevel) * a) *
           getRainLevel(a);
}

float Level::getRainLevel(float a) {
    return oRainLevel + (rainLevel - oRainLevel) * a;
}

void Level::setRainLevel(float rainLevel) {
    oRainLevel = rainLevel;
    this->rainLevel = rainLevel;
}





void Level::setThunderLevel(float thunderLevel) {
    oThunderLevel = thunderLevel;
    this->thunderLevel = thunderLevel;
}

bool Level::isThundering() { return getThunderLevel(1) > 0.9; }

bool Level::isRaining() { return getRainLevel(1) > 0.2; }

bool Level::isRainingAt(int x, int y, int z) {
    if (!isRaining()) return false;
    if (!canSeeSky(x, y, z)) return false;
    if (getTopRainBlock(x, z) > y) return false;

    
    
    if (biomeHasSnow(x, z)) return false;
    return biomeHasRain(x, z);
}

bool Level::isHumidAt(int x, int y, int z) {
    Biome* biome = getBiome(x, z);
    return biome->isHumid();
}

void Level::setSavedData(const std::wstring& id,
                         std::shared_ptr<SavedData> data) {
    savedDataStorage->set(id, data);
}

std::shared_ptr<SavedData> Level::getSavedData(const std::type_info& clazz,
                                               const std::wstring& id) {
    return savedDataStorage->get(clazz, id);
}

int Level::getFreeAuxValueFor(const std::wstring& id) {
    return savedDataStorage->getFreeAuxValueFor(id);
}


int Level::getAuxValueForMap(PlayerUID xuid, int dimension, int centreXC,
                             int centreZC, int scale) {
    return savedDataStorage->getAuxValueForMap(xuid, dimension, centreXC,
                                               centreZC, scale);
}

void Level::globalLevelEvent(int type, int sourceX, int sourceY, int sourceZ,
                             int data) {
    auto itEnd = listeners.end();
    for (auto it = listeners.begin(); it != itEnd; it++) {
        (*it)->globalLevelEvent(type, sourceX, sourceY, sourceZ, data);
    }
}

void Level::levelEvent(int type, int x, int y, int z, int data) {
    levelEvent(nullptr, type, x, y, z, data);
}

void Level::levelEvent(std::shared_ptr<Player> source, int type, int x, int y,
                       int z, int data) {
    auto itEnd = listeners.end();
    for (auto it = listeners.begin(); it != itEnd; it++) {
        (*it)->levelEvent(source, type, x, y, z, data);
    }
}

int Level::getMaxBuildHeight() { return maxBuildHeight; }

int Level::getHeight() {
    return dimension->hasCeiling ? genDepth : maxBuildHeight;
}

Tickable* Level::makeSoundUpdater(std::shared_ptr<Minecart> minecart) {
    return nullptr;
}

Random* Level::getRandomFor(int x, int z, int blend) {
    int64_t seed = (x * 341873128712l + z * 132897987541l) +
                   getLevelData()->getSeed() + blend;
    random->setSeed(seed);
    return random;
}

TilePos* Level::findNearestMapFeature(const std::wstring& featureName, int x,
                                      int y, int z) {
    return getChunkSource()->findNearestMapFeature(this, featureName, x, y, z);
}

bool Level::isAllEmpty() { return false; }

double Level::getHorizonHeight() {
    if (levelData->getGenerator() == LevelType::lvl_flat) {
        return 0.0;
    }
    return 63.0;
}

void Level::destroyTileProgress(int id, int x, int y, int z, int progress) {
    auto itEnd = listeners.end();
    for (auto it = listeners.begin(); it != itEnd; it++) {
        (*it)->destroyTileProgress(id, x, y, z, progress);
    }
}

void Level::createFireworks(double x, double y, double z, double xd, double yd,
                            double zd, CompoundTag* infoTag) {}

Scoreboard* Level::getScoreboard() { return scoreboard; }

void Level::updateNeighbourForOutputSignal(int x, int y, int z, int source) {
    for (int dir = 0; dir < 4; dir++) {
        int xx = x + Direction::STEP_X[dir];
        int zz = z + Direction::STEP_Z[dir];
        int id = getTile(xx, y, zz);
        if (id == 0) continue;
        Tile* tile = Tile::tiles[id];

        if (Tile::comparator_off->isSameDiode(id)) {
            tile->neighborChanged(this, xx, y, zz, source);
        } else if (Tile::isSolidBlockingTile(id)) {
            xx += Direction::STEP_X[dir];
            zz += Direction::STEP_Z[dir];
            id = getTile(xx, y, zz);
            tile = Tile::tiles[id];

            if (Tile::comparator_off->isSameDiode(id)) {
                tile->neighborChanged(this, xx, y, zz, source);
            }
        }
    }
}

float Level::getDifficulty(double x, double y, double z) {
    return getDifficulty(Mth::floor(x), Mth::floor(y), Mth::floor(z));
}





float Level::getDifficulty(int x, int y, int z) {
    float result = 0;
    bool isHard = difficulty == Difficulty::HARD;

    if (hasChunkAt(x, y, z)) {
        float moonBrightness = getMoonBrightness();

        result += Mth::clamp(getChunkAt(x, z)->inhabitedTime /
                                 (TICKS_PER_DAY * 150.0f),
                             0.0f, 1.0f) *
                  (isHard ? 1.0f : 0.75f);
        result += moonBrightness * 0.25f;
    }

    if (difficulty < Difficulty::NORMAL) {
        result *= difficulty / 2.0f;
    }

    return Mth::clamp(result, 0.0f, isHard ? 1.5f : 1.0f);
    ;
}

bool Level::useNewSeaLevel() { return levelData->useNewSeaLevel(); }

bool Level::getHasBeenInCreative() { return levelData->getHasBeenInCreative(); }

bool Level::isGenerateMapFeatures() {
    return levelData->isGenerateMapFeatures();
}

int Level::getSaveVersion() {
    return getLevelStorage()->getSaveFile()->getSaveVersion();
}

int Level::getOriginalSaveVersion() {
    return getLevelStorage()->getSaveFile()->getOriginalSaveVersion();
}





bool Level::isChunkPostPostProcessed(int x, int z) {
    if (!hasChunk(x, z))
        return false;  
                       

    LevelChunk* lc = getChunk(x, z);
    if (lc->isEmpty())
        return true;  
                      
                      

    return ((lc->terrainPopulated & LevelChunk::sTerrainPostPostProcessed) ==
            LevelChunk::sTerrainPostPostProcessed);
}






bool Level::isChunkFinalised(int x, int z) {
    for (int xo = -1; xo <= 1; xo++)
        for (int zo = -1; zo <= 1; zo++) {
            if (!isChunkPostPostProcessed(x + xo, z + zo)) return false;
        }

    return true;
}

int Level::getUnsavedChunkCount() { return m_unsavedChunkCount; }

void Level::incrementUnsavedChunkCount() { ++m_unsavedChunkCount; }

void Level::decrementUnsavedChunkCount() { --m_unsavedChunkCount; }

bool Level::canCreateMore(eINSTANCEOF type, ESPAWN_TYPE spawnType) {
    int count = 0;
    int max = 0;
    if (spawnType == eSpawnType_Egg || spawnType == eSpawnType_Portal) {
        switch (type) {
            case eTYPE_VILLAGER:
                count = countInstanceOf(eTYPE_VILLAGER, true);
                max = MobCategory::MAX_XBOX_VILLAGERS_WITH_SPAWN_EGG;
                break;
            case eTYPE_CHICKEN:
                count = countInstanceOf(eTYPE_CHICKEN, true);
                max = MobCategory::MAX_XBOX_CHICKENS_WITH_SPAWN_EGG;
                break;
            case eTYPE_WOLF:
                count = countInstanceOf(eTYPE_WOLF, true);
                max = MobCategory::MAX_XBOX_WOLVES_WITH_SPAWN_EGG;
                break;
            case eTYPE_MUSHROOMCOW:
                count = countInstanceOf(eTYPE_MUSHROOMCOW, true);
                max = MobCategory::MAX_XBOX_MUSHROOMCOWS_WITH_SPAWN_EGG;
                break;
            case eTYPE_SQUID:
                count = countInstanceOf(eTYPE_SQUID, true);
                max = MobCategory::MAX_XBOX_SQUIDS_WITH_SPAWN_EGG;
                break;
            case eTYPE_SNOWMAN:
                count = countInstanceOf(eTYPE_SNOWMAN, true);
                max = MobCategory::MAX_XBOX_SNOWMEN;
                break;
            case eTYPE_VILLAGERGOLEM:
                count = countInstanceOf(eTYPE_VILLAGERGOLEM, true);
                max = MobCategory::MAX_XBOX_IRONGOLEM;
                break;
            case eTYPE_WITHERBOSS:
                count = countInstanceOf(eTYPE_WITHERBOSS, true) +
                        countInstanceOf(eTYPE_ENDERDRAGON, true);
                max = MobCategory::MAX_CONSOLE_BOSS;
                break;
            default:
                if ((type & eTYPE_ANIMALS_SPAWN_LIMIT_CHECK) ==
                    eTYPE_ANIMALS_SPAWN_LIMIT_CHECK) {
                    count =
                        countInstanceOf(eTYPE_ANIMALS_SPAWN_LIMIT_CHECK, false);
                    max = MobCategory::MAX_XBOX_ANIMALS_WITH_SPAWN_EGG;
                }
                
                
                else if (Entity::instanceof(type, eTYPE_ENEMY)) {
                    count = countInstanceOf(eTYPE_ENEMY, false);
                    max = MobCategory::MAX_XBOX_MONSTERS_WITH_SPAWN_EGG;
                } else if ((type & eTYPE_AMBIENT) == eTYPE_AMBIENT) {
                    count = countInstanceOf(eTYPE_AMBIENT, false);
                    max = MobCategory::MAX_AMBIENT_WITH_SPAWN_EGG;
                }
                
                else if (Entity::instanceof(type, eTYPE_MINECART)) {
                    count = countInstanceOf(eTYPE_MINECART, false);
                    max = Level::MAX_CONSOLE_MINECARTS;
                } else if (Entity::instanceof(type, eTYPE_BOAT)) {
                    count = countInstanceOf(eTYPE_BOAT, true);
                    max = Level::MAX_XBOX_BOATS;
                }
        };
    } else if (spawnType == eSpawnType_Breed) {
        switch (type) {
            case eTYPE_VILLAGER:
                count = countInstanceOf(eTYPE_VILLAGER, true);
                max = MobCategory::MAX_VILLAGERS_WITH_BREEDING;
                break;
            case eTYPE_CHICKEN:
                count = countInstanceOf(eTYPE_CHICKEN, true);
                max = MobCategory::MAX_XBOX_CHICKENS_WITH_BREEDING;
                break;
            case eTYPE_WOLF:
                count = countInstanceOf(eTYPE_WOLF, true);
                max = MobCategory::MAX_XBOX_WOLVES_WITH_BREEDING;
                break;
            case eTYPE_MUSHROOMCOW:
                count = countInstanceOf(eTYPE_MUSHROOMCOW, true);
                max = MobCategory::MAX_XBOX_MUSHROOMCOWS_WITH_BREEDING;
                break;
            default:
                if ((type & eTYPE_ANIMALS_SPAWN_LIMIT_CHECK) ==
                    eTYPE_ANIMALS_SPAWN_LIMIT_CHECK) {
                    count =
                        countInstanceOf(eTYPE_ANIMALS_SPAWN_LIMIT_CHECK, false);
                    max = MobCategory::MAX_XBOX_ANIMALS_WITH_BREEDING;
                } else if ((type & eTYPE_MONSTER) == eTYPE_MONSTER) {
                }
                break;
        }
    }
    
    return max == 0 || count < max;
}
