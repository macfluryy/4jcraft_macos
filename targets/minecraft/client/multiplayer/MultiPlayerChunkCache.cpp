#include "MultiPlayerChunkCache.h"

#include <stdint.h>
#include <string.h>

#include "app/common/src/Network/GameNetworkManager.h"
#include "app/mac/Stubs/winapi_stubs.h"
#include "util/StringHelpers.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/server/level/ServerChunkCache.h"
#include "minecraft/server/level/ServerLevel.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/LevelType.h"
#include "minecraft/world/level/LightLayer.h"
#include "minecraft/world/level/biome/Biome.h"
#include "minecraft/world/level/chunk/EmptyLevelChunk.h"
#include "minecraft/world/level/chunk/LevelChunk.h"
#include "minecraft/world/level/chunk/WaterLevelChunk.h"
#include "minecraft/world/level/dimension/Dimension.h"
#include "minecraft/world/level/storage/LevelData.h"
#include "minecraft/world/level/tile/Tile.h"

MultiPlayerChunkCache::MultiPlayerChunkCache(Level* level) {
    XZSIZE = level->dimension->getXZSize();  
    XZOFFSET = XZSIZE / 2;                   
    m_XZSize = XZSIZE;
    hasData = new bool[XZSIZE * XZSIZE];
    memset(hasData, 0, sizeof(bool) * XZSIZE * XZSIZE);

    std::vector<uint8_t> emptyBlocks(16 * 16 * Level::maxBuildHeight);
    emptyChunk = new EmptyLevelChunk(level, emptyBlocks, 0, 0);

    
    
    if (level->dimension->id == 0) {
        std::vector<uint8_t> bytes = std::vector<uint8_t>(16 * 16 * 128);

        
        if (level->getLevelData()->getGenerator() == LevelType::lvl_flat) {
            for (int x = 0; x < 16; x++)
                for (int y = 0; y < 128; y++)
                    for (int z = 0; z < 16; z++) {
                        unsigned char tileId = 0;
                        if (y == 3)
                            tileId = Tile::grass_Id;
                        else if (y <= 2)
                            tileId = Tile::dirt_Id;

                        bytes[x << 11 | z << 7 | y] = tileId;
                    }
        } else {
            for (int x = 0; x < 16; x++)
                for (int y = 0; y < 128; y++)
                    for (int z = 0; z < 16; z++) {
                        unsigned char tileId = 0;
                        if (y <= (level->getSeaLevel() - 10))
                            tileId = Tile::stone_Id;
                        else if (y < level->getSeaLevel())
                            tileId = Tile::calmWater_Id;

                        bytes[x << 11 | z << 7 | y] = tileId;
                    }
        }

        waterChunk = new WaterLevelChunk(level, bytes, 0, 0);

        if (level->getLevelData()->getGenerator() == LevelType::lvl_flat) {
            for (int x = 0; x < 16; x++)
                for (int y = 0; y < 128; y++)
                    for (int z = 0; z < 16; z++) {
                        if (y >= 3) {
                            ((WaterLevelChunk*)waterChunk)
                                ->setLevelChunkBrightness(LightLayer::Sky, x, y,
                                                          z, 15);
                        }
                    }
        } else {
            for (int x = 0; x < 16; x++)
                for (int y = 0; y < 128; y++)
                    for (int z = 0; z < 16; z++) {
                        if (y >= (level->getSeaLevel() - 1)) {
                            ((WaterLevelChunk*)waterChunk)
                                ->setLevelChunkBrightness(LightLayer::Sky, x, y,
                                                          z, 15);
                        } else {
                            ((WaterLevelChunk*)waterChunk)
                                ->setLevelChunkBrightness(LightLayer::Sky, x, y,
                                                          z, 2);
                        }
                    }
        }
    } else {
        waterChunk = nullptr;
    }

    this->level = level;

    this->cache = new LevelChunk*[XZSIZE * XZSIZE];
    memset(this->cache, 0, XZSIZE * XZSIZE * sizeof(LevelChunk*));
}

MultiPlayerChunkCache::~MultiPlayerChunkCache() {
    delete emptyChunk;
    delete waterChunk;
    delete cache;
    delete hasData;

    auto itEnd = loadedChunkList.end();
    for (auto it = loadedChunkList.begin(); it != itEnd; it++) delete *it;
}

bool MultiPlayerChunkCache::hasChunk(int x, int z) {
    
    
    return true;
}


bool MultiPlayerChunkCache::reallyHasChunk(int x, int z) {
    int ix = x + XZOFFSET;
    int iz = z + XZOFFSET;
    
    
    if ((ix < 0) || (ix >= XZSIZE)) return true;
    if ((iz < 0) || (iz >= XZSIZE)) return true;
    int idx = ix * XZSIZE + iz;

    LevelChunk* chunk = cache[idx];
    if (chunk == nullptr) {
        return false;
    }
    return hasData[idx];
}

void MultiPlayerChunkCache::drop(int x, int z) {
    
    
    
    LevelChunk* chunk = getChunk(x, z);
    if (!chunk->isEmpty()) {
        
        
        
        
        chunk->unload(false);

        
        
        chunk->loaded = true;
    }
}

LevelChunk* MultiPlayerChunkCache::create(int x, int z) {
    int ix = x + XZOFFSET;
    int iz = z + XZOFFSET;
    
    if ((ix < 0) || (ix >= XZSIZE))
        return (waterChunk ? waterChunk : emptyChunk);
    if ((iz < 0) || (iz >= XZSIZE))
        return (waterChunk ? waterChunk : emptyChunk);
    int idx = ix * XZSIZE + iz;
    LevelChunk* chunk = cache[idx];
    LevelChunk* lastChunk = chunk;

    if (chunk == nullptr) {
        {
            std::unique_lock<std::mutex> lock(m_csLoadCreate);

            
            if (g_NetworkManager
                    .IsHost())  
            {
                
                
                if (MinecraftServer::getInstance()->serverHalted())
                    return nullptr;

                
                
#ifdef _LARGE_WORLDS
                LevelChunk* serverChunk =
                    MinecraftServer::getInstance()
                        ->getLevel(level->dimension->id)
                        ->cache->getChunkLoadedOrUnloaded(x, z);
#else
                LevelChunk* serverChunk = MinecraftServer::getInstance()
                                              ->getLevel(level->dimension->id)
                                              ->cache->getChunk(x, z);
#endif
                chunk = new LevelChunk(level, x, z, serverChunk);
                
                
                
                level->setTilesDirty(x * 16, 0, z * 16, x * 16 + 15, 127,
                                     z * 16 + 15);
                hasData[idx] = true;
            } else {
                
                
                std::vector<uint8_t> bytes;

                chunk = new LevelChunk(level, bytes, x, z);

                
                chunk->setSkyLightDataAllBright();
                
                
            }

            chunk->loaded = true;
        }

#if (defined _WIN64 || defined __LP64__)
        if (InterlockedCompareExchangeRelease64(
                (int64_t*)&cache[idx], (int64_t)chunk, (int64_t)lastChunk) ==
            (int64_t)lastChunk)
#else
        if (InterlockedCompareExchangeRelease(
                (int32_t*)&cache[idx], (int32_t)chunk, (int32_t)lastChunk) ==
            (int32_t)lastChunk)
#endif  
        {
            
            
            
            if (g_NetworkManager.IsHost()) {
                chunk->recalcHeightmapOnly();
            }

            
            {
                std::lock_guard<std::mutex> lock(m_csLoadCreate);
                loadedChunkList.push_back(chunk);
            }
        } else {
            
            
            
            delete chunk;
            return cache[idx];
        }

    } else {
        chunk->load();
    }

    return chunk;
}

LevelChunk* MultiPlayerChunkCache::getChunk(int x, int z) {
    int ix = x + XZOFFSET;
    int iz = z + XZOFFSET;
    
    if ((ix < 0) || (ix >= XZSIZE))
        return (waterChunk ? waterChunk : emptyChunk);
    if ((iz < 0) || (iz >= XZSIZE))
        return (waterChunk ? waterChunk : emptyChunk);
    int idx = ix * XZSIZE + iz;

    LevelChunk* chunk = cache[idx];
    if (chunk == nullptr) {
        return emptyChunk;
    } else {
        return chunk;
    }
}

bool MultiPlayerChunkCache::save(bool force,
                                 ProgressListener* progressListener) {
    return true;
}

bool MultiPlayerChunkCache::tick() { return false; }

bool MultiPlayerChunkCache::shouldSave() { return false; }

void MultiPlayerChunkCache::postProcess(ChunkSource* parent, int x, int z) {}

std::vector<Biome::MobSpawnerData*>* MultiPlayerChunkCache::getMobsAt(
    MobCategory* mobCategory, int x, int y, int z) {
    return nullptr;
}

TilePos* MultiPlayerChunkCache::findNearestMapFeature(
    Level* level, const std::wstring& featureName, int x, int y, int z) {
    return nullptr;
}

void MultiPlayerChunkCache::recreateLogicStructuresForChunk(int chunkX,
                                                            int chunkZ) {}

std::wstring MultiPlayerChunkCache::gatherStats() {
    int size;
    {
        std::lock_guard<std::mutex> lock(m_csLoadCreate);
        size = (int)loadedChunkList.size();
    }
    return L"MultiplayerChunkCache: " + toWString<int>(size);
}

void MultiPlayerChunkCache::dataReceived(int x, int z) {
    int ix = x + XZOFFSET;
    int iz = z + XZOFFSET;
    
    if ((ix < 0) || (ix >= XZSIZE)) return;
    if ((iz < 0) || (iz >= XZSIZE)) return;
    int idx = ix * XZSIZE + iz;
    hasData[idx] = true;
}