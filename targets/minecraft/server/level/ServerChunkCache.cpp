#include "ServerChunkCache.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <algorithm>

#include "app/mac/MacGame.h"
#include "app/mac/Stubs/winapi_stubs.h"
#include "ServerLevel.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/compression.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/util/ProgressListener.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/biome/Biome.h"
#include "minecraft/world/level/chunk/ChunkSource.h"
#include "minecraft/world/level/chunk/EmptyLevelChunk.h"
#include "minecraft/world/level/chunk/LevelChunk.h"
#include "minecraft/world/level/chunk/storage/ChunkStorage.h"
#include "minecraft/world/level/chunk/storage/OldChunkStorage.h"
#include "minecraft/world/level/dimension/Dimension.h"
#include "minecraft/world/level/tile/Tile.h"

ServerChunkCache::ServerChunkCache(ServerLevel* level, ChunkStorage* storage,
                                   ChunkSource* source) {
    XZSIZE = source->m_XZSize;  
    XZOFFSET = XZSIZE / 2;      

    autoCreate = false;  

    std::vector<uint8_t> emptyBlocks(Level::CHUNK_TILE_COUNT);
    emptyChunk = new EmptyLevelChunk(level, emptyBlocks, 0, 0);

    this->level = level;
    this->storage = storage;
    this->source = source;
    this->m_XZSize = source->m_XZSize;

    this->cache = new LevelChunk*[XZSIZE * XZSIZE];
    memset(this->cache, 0, XZSIZE * XZSIZE * sizeof(LevelChunk*));

#if defined(_LARGE_WORLDS)
    m_unloadedCache = new LevelChunk*[XZSIZE * XZSIZE];
    memset(m_unloadedCache, 0, XZSIZE * XZSIZE * sizeof(LevelChunk*));
#endif
}


ServerChunkCache::~ServerChunkCache() {
    storage->WaitForAll();  
    delete emptyChunk;
    delete[] cache;  
    delete source;

#if defined(_LARGE_WORLDS)
    for (unsigned int i = 0; i < XZSIZE * XZSIZE; ++i) {
        delete m_unloadedCache[i];
    }
    delete m_unloadedCache;
#endif

    auto itEnd = m_loadedChunkList.end();
    for (auto it = m_loadedChunkList.begin(); it != itEnd; it++) delete *it;
}

bool ServerChunkCache::hasChunk(int x, int z) {
    int ix = x + XZOFFSET;
    int iz = z + XZOFFSET;
    
    
    
    
    
    if ((ix < 0) || (ix >= XZSIZE)) return true;
    if ((iz < 0) || (iz >= XZSIZE)) return true;
    int idx = ix * XZSIZE + iz;
    LevelChunk* lc = cache[idx];
    if (lc == nullptr) return false;
    return true;
}

std::vector<LevelChunk*>* ServerChunkCache::getLoadedChunkList() {
    return &m_loadedChunkList;
}

void ServerChunkCache::drop(int x, int z) {
    
    
#if defined(_LARGE_WORLDS)

    bool canDrop = false;
    
    
    
    
    
    
    
    
    
    
    
    
    
    {
        canDrop = true;
    }
    if (canDrop) {
        int ix = x + XZOFFSET;
        int iz = z + XZOFFSET;
        
        if ((ix < 0) || (ix >= XZSIZE)) return;
        if ((iz < 0) || (iz >= XZSIZE)) return;
        int idx = ix * XZSIZE + iz;
        LevelChunk* chunk = cache[idx];

        if (chunk) {
            m_toDrop.push_back(chunk);
        }
    }
#endif
}

void ServerChunkCache::dropAll() {
#if defined(_LARGE_WORLDS)
    for (LevelChunk* chunk : m_loadedChunkList) {
        drop(chunk->x, chunk->z);
    }
#endif
}


LevelChunk* ServerChunkCache::create(int x, int z) {
    return create(x, z, false);
}

LevelChunk* ServerChunkCache::create(
    int x, int z, bool asyncPostProcess)  
{
    int ix = x + XZOFFSET;
    int iz = z + XZOFFSET;
    
    if ((ix < 0) || (ix >= XZSIZE)) return emptyChunk;
    if ((iz < 0) || (iz >= XZSIZE)) return emptyChunk;
    int idx = ix * XZSIZE + iz;

    LevelChunk* chunk = cache[idx];
    LevelChunk* lastChunk = chunk;

    if ((chunk == nullptr) || (chunk->x != x) || (chunk->z != z)) {
        {
            std::lock_guard<std::recursive_mutex> lock(m_csLoadCreate);
            chunk = load(x, z);
            if (chunk == nullptr) {
                if (source == nullptr) {
                    chunk = emptyChunk;
                } else {
                    chunk = source->getChunk(x, z);
                }
            }
            if (chunk != nullptr) {
                chunk->load();
            }
        }

#if defined(_WIN64) || defined(__LP64__)
        if (InterlockedCompareExchangeRelease64(
                (int64_t*)&cache[idx], (int64_t)chunk, (int64_t)lastChunk) ==
            (int64_t)lastChunk)
#else
        if (InterlockedCompareExchangeRelease(
                (int32_t*)&cache[idx], (int32_t)chunk, (int32_t)lastChunk) ==
            (int32_t)lastChunk)
#endif
        {
            
            std::lock_guard<std::recursive_mutex> lock(m_csLoadCreate);
            
            
            
            
            
            source->lightChunk(chunk);

            updatePostProcessFlags(x, z);

            m_loadedChunkList.push_back(chunk);

            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            

            if (asyncPostProcess) {
                
                
                
                if (((chunk->terrainPopulated &
                      LevelChunk::sTerrainPopulatedFromHere) == 0) &&
                    hasChunk(x + 1, z + 1) && hasChunk(x, z + 1) &&
                    hasChunk(x + 1, z))
                    MinecraftServer::getInstance()->addPostProcessRequest(this,
                                                                          x, z);
                if (hasChunk(x - 1, z) &&
                    ((getChunk(x - 1, z)->terrainPopulated &
                      LevelChunk::sTerrainPopulatedFromHere) == 0) &&
                    hasChunk(x - 1, z + 1) && hasChunk(x, z + 1) &&
                    hasChunk(x - 1, z))
                    MinecraftServer::getInstance()->addPostProcessRequest(
                        this, x - 1, z);
                if (hasChunk(x, z - 1) &&
                    ((getChunk(x, z - 1)->terrainPopulated &
                      LevelChunk::sTerrainPopulatedFromHere) == 0) &&
                    hasChunk(x + 1, z - 1) && hasChunk(x, z - 1) &&
                    hasChunk(x + 1, z))
                    MinecraftServer::getInstance()->addPostProcessRequest(
                        this, x, z - 1);
                if (hasChunk(x - 1, z - 1) &&
                    ((getChunk(x - 1, z - 1)->terrainPopulated &
                      LevelChunk::sTerrainPopulatedFromHere) == 0) &&
                    hasChunk(x - 1, z - 1) && hasChunk(x, z - 1) &&
                    hasChunk(x - 1, z))
                    MinecraftServer::getInstance()->addPostProcessRequest(
                        this, x - 1, z - 1);
            } else {
                chunk->checkPostProcess(this, this, x, z);
            }

            
            
            
            
            
            
            
            

            
            
            
            
            
            

            if (hasChunk(x - 1, z) && hasChunk(x - 2, z) &&
                hasChunk(x - 1, z + 1) && hasChunk(x - 1, z - 1))
                chunk->checkChests(this, x - 1, z);
            if (hasChunk(x, z + 1) && hasChunk(x, z + 2) &&
                hasChunk(x - 1, z + 1) && hasChunk(x + 1, z + 1))
                chunk->checkChests(this, x, z + 1);
            if (hasChunk(x + 1, z) && hasChunk(x + 2, z) &&
                hasChunk(x + 1, z + 1) && hasChunk(x + 1, z - 1))
                chunk->checkChests(this, x + 1, z);
            if (hasChunk(x, z - 1) && hasChunk(x, z - 2) &&
                hasChunk(x - 1, z - 1) && hasChunk(x + 1, z - 1))
                chunk->checkChests(this, x, z - 1);
            if (hasChunk(x - 1, z) && hasChunk(x + 1, z) &&
                hasChunk(x, z - 1) && hasChunk(x, z + 1))
                chunk->checkChests(this, x, z);

        } else {
            
            
            chunk->unload(true);
            delete chunk;
            return cache[idx];
        }
    }

    return chunk;
}



LevelChunk* ServerChunkCache::getChunk(int x, int z) {
    int ix = x + XZOFFSET;
    int iz = z + XZOFFSET;
    
    if ((ix < 0) || (ix >= XZSIZE)) return emptyChunk;
    if ((iz < 0) || (iz >= XZSIZE)) return emptyChunk;
    int idx = ix * XZSIZE + iz;

    LevelChunk* lc = cache[idx];
    if (lc) {
        return lc;
    }

    if (level->isFindingSpawn || autoCreate) {
        return create(x, z);
    }

    return emptyChunk;
}

#if defined(_LARGE_WORLDS)







LevelChunk* ServerChunkCache::getChunkLoadedOrUnloaded(int x, int z) {
    int ix = x + XZOFFSET;
    int iz = z + XZOFFSET;
    
    if ((ix < 0) || (ix >= XZSIZE)) return emptyChunk;
    if ((iz < 0) || (iz >= XZSIZE)) return emptyChunk;
    int idx = ix * XZSIZE + iz;

    LevelChunk* lc = cache[idx];
    if (lc) {
        return lc;
    }

    lc = m_unloadedCache[idx];
    if (lc) {
        return lc;
    }

    if (level->isFindingSpawn || autoCreate) {
        return create(x, z);
    }

    return emptyChunk;
}
#endif



#if defined(_LARGE_WORLDS)
void ServerChunkCache::overwriteLevelChunkFromSource(int x, int z) {
    int ix = x + XZOFFSET;
    int iz = z + XZOFFSET;
    
    if ((ix < 0) || (ix >= XZSIZE)) assert(0);
    if ((iz < 0) || (iz >= XZSIZE)) assert(0);
    int idx = ix * XZSIZE + iz;

    LevelChunk* chunk = nullptr;
    chunk = source->getChunk(x, z);
    assert(chunk);
    if (chunk) {
        save(chunk);
    }
}

void ServerChunkCache::updateOverwriteHellChunk(LevelChunk* origChunk,
                                                LevelChunk* playerChunk,
                                                int xMin, int xMax, int zMin,
                                                int zMax) {
    
    
    for (int x = xMin; x < xMax; x++) {
        for (int z = zMin; z < zMax; z++) {
            for (int y = 0; y < 256; y++) {
                int playerTile = playerChunk->getTile(x, y, z);
                if (playerTile ==
                    Tile::unbreakable_Id)  
                                           
                                           
                    playerChunk->setTileAndData(x, y, z,
                                                origChunk->getTile(x, y, z),
                                                origChunk->getData(x, y, z));
            }
        }
    }
}

void ServerChunkCache::overwriteHellLevelChunkFromSource(int x, int z,
                                                         int minVal,
                                                         int maxVal) {
    int ix = x + XZOFFSET;
    int iz = z + XZOFFSET;
    
    if ((ix < 0) || (ix >= XZSIZE)) assert(0);
    if ((iz < 0) || (iz >= XZSIZE)) assert(0);
    int idx = ix * XZSIZE + iz;
    autoCreate = true;
    LevelChunk* playerChunk = getChunk(x, z);
    autoCreate = false;
    LevelChunk* origChunk = source->getChunk(x, z);
    assert(origChunk);
    if (playerChunk != emptyChunk) {
        if (x == minVal)
            updateOverwriteHellChunk(origChunk, playerChunk, 0, 4, 0, 16);
        if (x == maxVal)
            updateOverwriteHellChunk(origChunk, playerChunk, 12, 16, 0, 16);
        if (z == minVal)
            updateOverwriteHellChunk(origChunk, playerChunk, 0, 16, 0, 4);
        if (z == maxVal)
            updateOverwriteHellChunk(origChunk, playerChunk, 0, 16, 12, 16);
    }
    save(playerChunk);
}

#endif


#if defined(_LARGE_WORLDS)
void ServerChunkCache::dontDrop(int x, int z) {
    LevelChunk* chunk = getChunk(x, z);
    m_toDrop.erase(std::remove(m_toDrop.begin(), m_toDrop.end(), chunk),
                   m_toDrop.end());
}
#endif

LevelChunk* ServerChunkCache::load(int x, int z) {
    if (storage == nullptr) return nullptr;

    LevelChunk* levelChunk = nullptr;

#if defined(_LARGE_WORLDS)
    int ix = x + XZOFFSET;
    int iz = z + XZOFFSET;
    int idx = ix * XZSIZE + iz;
    levelChunk = m_unloadedCache[idx];
    m_unloadedCache[idx] = nullptr;
    if (levelChunk == nullptr)
#endif
    {
        levelChunk = storage->load(level, x, z);
    }
    if (levelChunk != nullptr) {
        levelChunk->lastSaveTime = level->getGameTime();
    }
    return levelChunk;
}

void ServerChunkCache::saveEntities(LevelChunk* levelChunk) {
    if (storage == nullptr) return;

    storage->saveEntities(level, levelChunk);
}

void ServerChunkCache::save(LevelChunk* levelChunk) {
    if (storage == nullptr) return;

    levelChunk->lastSaveTime = level->getGameTime();
    storage->save(level, levelChunk);
}


void ServerChunkCache::updatePostProcessFlag(short flag, int x, int z, int xo,
                                             int zo, LevelChunk* lc) {
    if (hasChunk(x + xo, z + zo)) {
        LevelChunk* lc2 = getChunk(x + xo, z + zo);
        if (lc2 != emptyChunk)  
                                
                                
        {
            if (lc2->terrainPopulated & LevelChunk::sTerrainPopulatedFromHere) {
                lc->terrainPopulated |= flag;
            }
        } else {
            
            lc->terrainPopulated |= flag;
        }
    }
}





void ServerChunkCache::updatePostProcessFlags(int x, int z) {
    LevelChunk* lc = getChunk(x, z);
    if (lc != emptyChunk) {
        
        
        updatePostProcessFlag(LevelChunk::sTerrainPopulatedFromS, x, z, 0, -1,
                              lc);
        updatePostProcessFlag(LevelChunk::sTerrainPopulatedFromSW, x, z, -1, -1,
                              lc);
        updatePostProcessFlag(LevelChunk::sTerrainPopulatedFromW, x, z, -1, 0,
                              lc);
        updatePostProcessFlag(LevelChunk::sTerrainPopulatedFromNW, x, z, -1, 1,
                              lc);
        updatePostProcessFlag(LevelChunk::sTerrainPopulatedFromN, x, z, 0, 1,
                              lc);
        updatePostProcessFlag(LevelChunk::sTerrainPopulatedFromNE, x, z, 1, 1,
                              lc);
        updatePostProcessFlag(LevelChunk::sTerrainPopulatedFromE, x, z, 1, 0,
                              lc);
        updatePostProcessFlag(LevelChunk::sTerrainPopulatedFromSE, x, z, 1, -1,
                              lc);

        
        
        if (lc->terrainPopulated & LevelChunk::sTerrainPopulatedFromHere) {
            flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromW, x + 1,
                                    z + 0);
            flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromSW, x + 1,
                                    z + 1);
            flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromS, x + 0,
                                    z + 1);
            flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromSE, x - 1,
                                    z + 1);
            flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromE, x - 1,
                                    z + 0);
            flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromNE, x - 1,
                                    z - 1);
            flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromN, x + 0,
                                    z - 1);
            flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromNW, x + 1,
                                    z - 1);
        }
    }

    flagPostProcessComplete(0, x, z);
}








void ServerChunkCache::flagPostProcessComplete(short flag, int x, int z) {
    
    
    if (!hasChunk(x, z)) return;

    LevelChunk* lc = level->getChunk(x, z);
    if (lc == emptyChunk) return;

    lc->terrainPopulated |= flag;

    
    
    if ((lc->terrainPopulated & LevelChunk::sTerrainPopulatedAllAffecting) ==
        LevelChunk::sTerrainPopulatedAllAffecting) {
        

        
        
        
        
        if (!lc->isLowerBlockStorageCompressed()) lc->compressBlocks();
        if (!lc->isLowerBlockLightStorageCompressed()) lc->compressLighting();
        if (!lc->isLowerDataStorageCompressed()) lc->compressData();
    }

    
    if (lc->terrainPopulated == LevelChunk::sTerrainPopulatedAllNeighbours) {
        
        app.processSchematicsLighting(lc);

        
        
        if (lc->level->dimension->id != 1) {
            lc->recheckGaps(true);
        }

        
        lc->lightLava();

        
        lc->terrainPopulated |= LevelChunk::sTerrainPostPostProcessed;
    }
}

void ServerChunkCache::postProcess(ChunkSource* parent, int x, int z) {
    LevelChunk* chunk = getChunk(x, z);
    if ((chunk->terrainPopulated & LevelChunk::sTerrainPopulatedFromHere) ==
        0) {
        if (source != nullptr) {
            source->postProcess(parent, x, z);

            chunk->markUnsaved();
        }

        
        
        
        
        chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromHere;

        
        
        if (x == -XZOFFSET)  
        {
            chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromW;
            chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromSW;
            chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromNW;
        }
        if (x == (XZOFFSET - 1))  
        {
            chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromE;
            chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromSE;
            chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromNE;
        }
        if (z == -XZOFFSET)  
        {
            chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromS;
            chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromSW;
            chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromSE;
        }
        if (z == (XZOFFSET - 1))  
        {
            chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromN;
            chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromNW;
            chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromNE;
        }

        
        
        
        flagPostProcessComplete(0, x, z);
        flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromW, x + 1,
                                z + 0);
        flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromSW, x + 1,
                                z + 1);
        flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromS, x + 0,
                                z + 1);
        flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromSE, x - 1,
                                z + 1);
        flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromE, x - 1,
                                z + 0);
        flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromNE, x - 1,
                                z - 1);
        flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromN, x + 0,
                                z - 1);
        flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromNW, x + 1,
                                z - 1);
    }
}


bool ServerChunkCache::saveAllEntities() {
    {
        std::lock_guard<std::recursive_mutex> lock(m_csLoadCreate);
        for (auto it = m_loadedChunkList.begin(); it != m_loadedChunkList.end();
             ++it) {
            storage->saveEntities(level, *it);
        }
    }

    storage->flush();

    return true;
}

bool ServerChunkCache::save(bool force, ProgressListener* progressListener) {
    std::lock_guard<std::recursive_mutex> lock(m_csLoadCreate);
    int saves = 0;

    
    int count = 0;
    if (progressListener != nullptr) {
        auto itEnd = m_loadedChunkList.end();
        for (auto it = m_loadedChunkList.begin(); it != itEnd; it++) {
            LevelChunk* chunk = *it;
            if (chunk->shouldSave(force)) {
                count++;
            }
        }
    }
    int cc = 0;

    bool maxSavesReached = false;

    if (!force) {
        
        
        
        for (unsigned int i = 0; i < m_loadedChunkList.size(); i++) {
            LevelChunk* chunk = m_loadedChunkList[i];
#if !defined(SPLIT_SAVES)
            if (force && !chunk->dontSave) saveEntities(chunk);
#endif
            if (chunk->shouldSave(force)) {
                save(chunk);
                chunk->setUnsaved(false);
                if (++saves == MAX_SAVES && !force) {
                    return false;
                }

                
                if (progressListener != nullptr) {
                    if (++cc % 10 == 0) {
                        progressListener->progressStagePercentage(cc * 100 /
                                                                  count);
                    }
                }
            }
        }
    } else {
        
        

        
        
        
        

        std::vector<LevelChunk*> sortedChunkList;

        for (int i = 0; i < m_loadedChunkList.size(); i++) {
            if ((m_loadedChunkList[i]->x < 0) && (m_loadedChunkList[i]->z < 0))
                sortedChunkList.push_back(m_loadedChunkList[i]);
        }
        for (int i = 0; i < m_loadedChunkList.size(); i++) {
            if ((m_loadedChunkList[i]->x >= 0) && (m_loadedChunkList[i]->z < 0))
                sortedChunkList.push_back(m_loadedChunkList[i]);
        }
        for (int i = 0; i < m_loadedChunkList.size(); i++) {
            if ((m_loadedChunkList[i]->x >= 0) &&
                (m_loadedChunkList[i]->z >= 0))
                sortedChunkList.push_back(m_loadedChunkList[i]);
        }
        for (int i = 0; i < m_loadedChunkList.size(); i++) {
            if ((m_loadedChunkList[i]->x < 0) && (m_loadedChunkList[i]->z >= 0))
                sortedChunkList.push_back(m_loadedChunkList[i]);
        }

        
        for (unsigned int i = 0; i < sortedChunkList.size(); ++i) {
            LevelChunk* chunk = sortedChunkList[i];
            if (force && !chunk->dontSave) saveEntities(chunk);
            if (chunk->shouldSave(force)) {
                save(chunk);
                chunk->setUnsaved(false);
                if (++saves == MAX_SAVES && !force) {
                    return false;
                }

                
                if (progressListener != nullptr) {
                    if (++cc % 10 == 0) {
                        progressListener->progressStagePercentage(cc * 100 /
                                                                  count);
                    }
                }
            }
            
            
            
            
            storage->WaitIfTooManyQueuedChunks();
        }

        
        storage->WaitForAll();
    }

    if (force) {
        if (storage == nullptr) {
            return true;
        }
        storage->flush();
    }

    return !maxSavesReached;
}

bool ServerChunkCache::tick() {
    if (!level->noSave) {
#if defined(_LARGE_WORLDS)
        for (int i = 0; i < 100; i++) {
            if (!m_toDrop.empty()) {
                LevelChunk* chunk = m_toDrop.front();
                if (!chunk->isUnloaded()) {
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    if (!chunk->containsPlayer()) {
                        save(chunk);
                        saveEntities(chunk);
                        chunk->unload(true);

                        
                        
                        auto it = find(m_loadedChunkList.begin(),
                                       m_loadedChunkList.end(), chunk);
                        if (it != m_loadedChunkList.end())
                            m_loadedChunkList.erase(it);

                        int ix = chunk->x + XZOFFSET;
                        int iz = chunk->z + XZOFFSET;
                        int idx = ix * XZSIZE + iz;
                        m_unloadedCache[idx] = chunk;
                        cache[idx] = nullptr;
                    }
                }
                m_toDrop.pop_front();
            }
        }
#endif
        if (storage != nullptr) storage->tick();
    }

    return source->tick();
}

bool ServerChunkCache::shouldSave() { return !level->noSave; }

std::wstring ServerChunkCache::gatherStats() {
    return L"ServerChunkCache: ";  
                                   
}

std::vector<Biome::MobSpawnerData*>* ServerChunkCache::getMobsAt(
    MobCategory* mobCategory, int x, int y, int z) {
    return source->getMobsAt(mobCategory, x, y, z);
}

TilePos* ServerChunkCache::findNearestMapFeature(
    Level* level, const std::wstring& featureName, int x, int y, int z) {
    return source->findNearestMapFeature(level, featureName, x, y, z);
}

void ServerChunkCache::recreateLogicStructuresForChunk(int chunkX, int chunkZ) {
}

int ServerChunkCache::runSaveThreadProc(void* lpParam) {
    SaveThreadData* params = (SaveThreadData*)lpParam;

    if (params->useSharedThreadStorage) {
        Compression::UseDefaultThreadStorage();
        OldChunkStorage::UseDefaultThreadStorage();
    } else {
        Compression::CreateNewThreadStorage();
        OldChunkStorage::CreateNewThreadStorage();
    }

    
    params->wakeEvent->waitForSignal(
        C4JThread::
            kInfiniteTimeout);  

    

    while (params->chunkToSave != nullptr) {
        
        if (params->saveEntities)
            params->cache->saveEntities(params->chunkToSave);

        params->cache->save(params->chunkToSave);
        params->chunkToSave->setUnsaved(false);

        
        params->notificationEvent
            ->set();  

        
        

        
        params->wakeEvent->waitForSignal(
            C4JThread::
                kInfiniteTimeout);  
    }

    

    if (!params->useSharedThreadStorage) {
        Compression::ReleaseThreadStorage();
        OldChunkStorage::ReleaseThreadStorage();
    }

    return 0;
}
