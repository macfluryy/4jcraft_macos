#include "ServerLevel.h"

#include <assert.h>

#include <algorithm>
#include <mutex>

#include "platform/sdl2/Input.h"
#include "platform/sdl2/Storage.h"
#include "EntityTracker.h"
#include "platform/ShutdownManager.h"
#include "app/common/src/Console_Debug_enum.h"
#include "app/common/src/DLC/DLCManager.h"
#include "app/common/src/DLC/DLCPack.h"
#include "app/common/src/Network/NetworkPlayerInterface.h"
#include "app/mac/MacGame.h"
#include "PlayerChunkMap.h"
#include "Pos.h"
#include "ServerChunkCache.h"
#include "ServerLevelListener.h"
#include "ServerPlayer.h"
#include "java/Class.h"
#include "java/Random.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/skins/DLCTexturePack.h"
#include "minecraft/client/skins/TexturePackRepository.h"
#include "minecraft/network/packet/AddGlobalEntityPacket.h"
#include "minecraft/network/packet/EntityEventPacket.h"
#include "minecraft/network/packet/ExplodePacket.h"
#include "minecraft/network/packet/GameEventPacket.h"
#include "minecraft/network/packet/LevelParticlesPacket.h"
#include "minecraft/network/packet/TileEventPacket.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/server/PlayerList.h"
#include "minecraft/server/ServerScoreboard.h"
#include "minecraft/server/network/PlayerConnection.h"
#include "minecraft/util/ProgressListener.h"
#include "minecraft/util/WeighedRandom.h"
#include "minecraft/util/WeighedTreasure.h"
#include "minecraft/world/entity/ai/village/VillageSiege.h"
#include "minecraft/world/entity/ai/village/Villages.h"
#include "minecraft/world/entity/global/LightningBolt.h"
#include "minecraft/world/entity/npc/Npc.h"
#include "minecraft/world/entity/player/Player.h"
#include "minecraft/world/item/DyePowderItem.h"
#include "minecraft/world/item/Item.h"
#include "minecraft/world/level/ChunkPos.h"
#include "minecraft/world/level/Explosion.h"
#include "minecraft/world/level/GameRules.h"
#include "minecraft/world/level/LevelListener.h"
#include "minecraft/world/level/LevelSettings.h"
#include "minecraft/world/level/MobSpawner.h"
#include "minecraft/world/level/PortalForcer.h"
#include "minecraft/world/level/TilePos.h"
#include "minecraft/world/level/biome/BiomeSource.h"
#include "minecraft/world/level/chunk/ChunkSource.h"
#include "minecraft/world/level/chunk/LevelChunk.h"
#include "minecraft/world/level/dimension/Dimension.h"
#include "minecraft/world/level/levelgen/feature/BonusChestFeature.h"
#include "minecraft/world/level/storage/LevelData.h"
#include "minecraft/world/level/storage/LevelStorage.h"
#include "minecraft/world/level/storage/SavedDataStorage.h"
#include "minecraft/world/level/tile/Tile.h"
#include "minecraft/world/level/tile/entity/ChestTileEntity.h"
#include "minecraft/world/level/tile/entity/TileEntity.h"
#include "minecraft/world/phys/Vec3.h"
#include "strings.h"

class ChunkStorage;
class MobCategory;
class Packet;
class TexturePack;

std::vector<WeighedTreasure*> ServerLevel::RANDOM_BONUS_ITEMS;

C4JThread* ServerLevel::m_updateThread = nullptr;
C4JThread::EventArray* ServerLevel::m_updateTrigger;
std::recursive_mutex ServerLevel::m_updateCS[3];

Level* ServerLevel::m_level[3];
int ServerLevel::m_updateChunkX[3][LEVEL_CHUNKS_TO_UPDATE_MAX];
int ServerLevel::m_updateChunkZ[3][LEVEL_CHUNKS_TO_UPDATE_MAX];
int ServerLevel::m_updateChunkCount[3];
int ServerLevel::m_updateTileX[3][MAX_UPDATES];
int ServerLevel::m_updateTileY[3][MAX_UPDATES];
int ServerLevel::m_updateTileZ[3][MAX_UPDATES];
int ServerLevel::m_updateTileCount[3];
int ServerLevel::m_randValue[3];

void ServerLevel::staticCtor() {
    m_updateTrigger = new C4JThread::EventArray(3);

    m_updateThread = new C4JThread(runUpdate, nullptr, "Tile update");
    m_updateThread->run();

    RANDOM_BONUS_ITEMS = std::vector<WeighedTreasure*>(20);

    RANDOM_BONUS_ITEMS[0] = new WeighedTreasure(Item::stick_Id, 0, 1, 3, 10);
    RANDOM_BONUS_ITEMS[1] = new WeighedTreasure(Tile::wood_Id, 0, 1, 3, 10);
    RANDOM_BONUS_ITEMS[2] =
        new WeighedTreasure(Tile::treeTrunk_Id, 0, 1, 3, 10);
    RANDOM_BONUS_ITEMS[3] =
        new WeighedTreasure(Item::hatchet_stone_Id, 0, 1, 1, 3);
    RANDOM_BONUS_ITEMS[4] =
        new WeighedTreasure(Item::hatchet_wood_Id, 0, 1, 1, 5);
    RANDOM_BONUS_ITEMS[5] =
        new WeighedTreasure(Item::pickAxe_stone_Id, 0, 1, 1, 3);
    RANDOM_BONUS_ITEMS[6] =
        new WeighedTreasure(Item::pickAxe_wood_Id, 0, 1, 1, 5);
    RANDOM_BONUS_ITEMS[7] = new WeighedTreasure(Item::apple_Id, 0, 2, 3, 5);
    RANDOM_BONUS_ITEMS[8] = new WeighedTreasure(Item::bread_Id, 0, 2, 3, 3);
    
    RANDOM_BONUS_ITEMS[9] = new WeighedTreasure(Tile::sapling_Id, 0, 4, 4, 2);
    RANDOM_BONUS_ITEMS[10] = new WeighedTreasure(Tile::sapling_Id, 1, 4, 4, 2);
    RANDOM_BONUS_ITEMS[11] = new WeighedTreasure(Tile::sapling_Id, 2, 4, 4, 2);
    RANDOM_BONUS_ITEMS[12] = new WeighedTreasure(Tile::sapling_Id, 3, 4, 4, 4);
    RANDOM_BONUS_ITEMS[13] =
        new WeighedTreasure(Item::seeds_melon_Id, 0, 1, 2, 3);
    RANDOM_BONUS_ITEMS[14] =
        new WeighedTreasure(Item::seeds_pumpkin_Id, 0, 1, 2, 3);
    RANDOM_BONUS_ITEMS[15] = new WeighedTreasure(Tile::cactus_Id, 0, 1, 2, 3);
    RANDOM_BONUS_ITEMS[16] =
        new WeighedTreasure(Item::dye_powder_Id, DyePowderItem::BROWN, 1, 2, 2);
    RANDOM_BONUS_ITEMS[17] = new WeighedTreasure(Item::potato_Id, 0, 1, 2, 3);
    RANDOM_BONUS_ITEMS[18] = new WeighedTreasure(Item::carrots_Id, 0, 1, 2, 3);
    RANDOM_BONUS_ITEMS[19] =
        new WeighedTreasure(Tile::mushroom_brown_Id, 0, 1, 2, 2);
};

ServerLevel::ServerLevel(MinecraftServer* server,
                         std::shared_ptr<LevelStorage> levelStorage,
                         const std::wstring& levelName, int dimension,
                         LevelSettings* levelSettings)
    : Level(levelStorage, levelName, levelSettings,
            Dimension::getNew(dimension), false) {
    m_fallingTileCount = 0;
    m_primedTntCount = 0;

    
    chunkSource = createChunkSource();
    
    chunkSourceCache = chunkSource->getCache();
    chunkSourceXZSize = chunkSource->m_XZSize;

    
    
    
    
    this->server = server;
    server->setLevel(dimension,
                     this);  
                             
                             
    addListener(new ServerLevelListener(server, this));

    tracker = new EntityTracker(this);
    chunkMap = new PlayerChunkMap(this, dimension,
                                  server->getPlayers()->getViewDistance());

    mobSpawner = new MobSpawner();
    portalForcer = new PortalForcer(this);
    scoreboard = new ServerScoreboard(server);

    
    
    
    
    
    
    
    
    
    
    

    
    
    if (!levelData->isInitialized()) {
        initializeLevel(levelSettings);
        levelData->setInitialized(true);
    } else if ((dimension == 0) &&
               levelData->getSpawnBonusChest())  
                                                 
    {
        
        
        
        isFindingSpawn = true;
        generateBonusItemsNearSpawn();
        isFindingSpawn = false;
    }

    
    
    
    canEditSpawn = true;  
    noSave = false;
    allPlayersSleeping = false;
    m_bAtLeastOnePlayerSleeping = false;
    emptyTime = 0;
    activeTileEventsList = 0;

#if defined(_LARGE_WORLDS)
    saveInterval = 3;
#else
    saveInterval = 20 * 2;
#endif
}

ServerLevel::~ServerLevel() {
    delete portalForcer;
    delete mobSpawner;

    {
        std::lock_guard<std::recursive_mutex> lock(m_csQueueSendTileUpdates);
        for (auto it = m_queuedSendTileUpdates.begin();
             it != m_queuedSendTileUpdates.end(); ++it) {
            Pos* p = *it;
            delete p;
        }
        m_queuedSendTileUpdates.clear();

        delete this->tracker;  
                               
        delete this->chunkMap;
    }

    
    {
        std::lock_guard<std::recursive_mutex> lock(m_updateCS[0]);
    }
    {
        std::lock_guard<std::recursive_mutex> lock(m_updateCS[1]);
    }
    {
        std::lock_guard<std::recursive_mutex> lock(m_updateCS[2]);
    }
    m_updateTrigger->clearAll();
}

void ServerLevel::tick() {
    Level::tick();
    if (getLevelData()->isHardcore() && difficulty < 3) {
        difficulty = 3;
    }

    dimension->biomeSource->update();

    if (allPlayersAreSleeping()) {
        if (getGameRules()->getBoolean(GameRules::RULE_DAYLIGHT)) {
            
            int64_t newTime = levelData->getDayTime() + TICKS_PER_DAY;

            
            
            
            setDayTime(newTime - (newTime % TICKS_PER_DAY));
        }
        awakenAllPlayers();
    }

    
    
    
    if (getGameRules()->getBoolean(GameRules::RULE_DOMOBSPAWNING)) {
        
        
        
        
        
        bool finalSpawnEnemies =
            spawnEnemies && ((levelData->getGameTime() % 2) ==
                             0);  
        bool finalSpawnFriendlies =
            spawnFriendlies && ((levelData->getGameTime() % 40) ==
                                0);  
        bool finalSpawnPersistent =
            finalSpawnFriendlies &&
            ((levelData->getGameTime() % 80) ==
             0);  
                  
        mobSpawner->tick(this, finalSpawnEnemies, finalSpawnFriendlies,
                         finalSpawnPersistent);
    }

    chunkSource->tick();

    int newDark = getOldSkyDarken(1);
    if (newDark != skyDarken) {
        skyDarken = newDark;
        if (!SharedConstants::TEXTURE_LIGHTING)  
                                                 
        {
            auto itEnd = listeners.end();
            for (auto it = listeners.begin(); it != itEnd; it++) {
                (*it)->skyColorChanged();
            }
        }
    }

    
    

    int64_t time = levelData->getGameTime() + 1;
    
    
#if defined(_LARGE_WORLDS)
    if (time % (saveInterval) == (dimension->id + 1))
#else
    if (time % (saveInterval) ==
        (dimension->id * dimension->id * (saveInterval / 2)))
#endif
    {
        
        save(false, nullptr);
    }

    
    
    
    setGameTime(levelData->getGameTime() + 1);
    if (getGameRules()->getBoolean(GameRules::RULE_DAYLIGHT)) {
        
#if !defined(_FINAL_BUILD)
        bool freezeTime =
            app.DebugSettingsOn() &&
            app.GetGameSettingsDebugMask(InputManager.GetPrimaryPad()) &
                (1L << eDebugSetting_FreezeTime);
        if (!freezeTime)
#endif
        {
            setDayTime(levelData->getDayTime() + 1);
        }
    }

    
    tickPendingTicks(false);

    tickTiles();

    chunkMap->tick();

    villages->tick();
    villageSiege->tick();

    portalForcer->tick(getGameTime());

    
    runTileEvents();

    
    runQueuedSendTileUpdates();
}

Biome::MobSpawnerData* ServerLevel::getRandomMobSpawnAt(
    MobCategory* mobCategory, int x, int y, int z) {
    std::vector<Biome::MobSpawnerData*>* mobList =
        getChunkSource()->getMobsAt(mobCategory, x, y, z);
    if (mobList == nullptr || mobList->empty()) return nullptr;

    return (Biome::MobSpawnerData*)WeighedRandom::getRandomItem(
        random, (std::vector<WeighedRandomItem*>*)mobList);
}

void ServerLevel::updateSleepingPlayerList() {
    allPlayersSleeping = !players.empty();
    m_bAtLeastOnePlayerSleeping = false;

    auto itEnd = players.end();
    for (auto it = players.begin(); it != itEnd; it++) {
        if (!(*it)->isSleeping()) {
            allPlayersSleeping = false;
            
        } else {
            m_bAtLeastOnePlayerSleeping = true;
        }
        if (m_bAtLeastOnePlayerSleeping && !allPlayersSleeping) break;
    }
}

void ServerLevel::awakenAllPlayers() {
    allPlayersSleeping = false;
    m_bAtLeastOnePlayerSleeping = false;

    auto itEnd = players.end();
    for (std::vector<std::shared_ptr<Player> >::iterator it = players.begin();
         it != itEnd; it++) {
        if ((*it)->isSleeping()) {
            (*it)->stopSleepInBed(false, false, true);
        }
    }

    stopWeather();
}

void ServerLevel::stopWeather() {
    levelData->setRainTime(0);
    levelData->setRaining(false);
    levelData->setThunderTime(0);
    levelData->setThundering(false);
}

bool ServerLevel::allPlayersAreSleeping() {
    if (allPlayersSleeping && !isClientSide) {
        
        auto itEnd = players.end();
        for (std::vector<std::shared_ptr<Player> >::iterator it =
                 players.begin();
             it != itEnd; it++) {
            
            
            if (!(*it)->isSleepingLongEnough()) {
                return false;
            }
        }
        
        return true;
    }
    return false;
}

void ServerLevel::validateSpawn() {
    if (levelData->getYSpawn() <= 0) {
        levelData->setYSpawn(genDepth / 2);
    }
    int xSpawn = levelData->getXSpawn();
    int zSpawn = levelData->getZSpawn();
    int tries = 0;
    while (getTopTile(xSpawn, zSpawn) == 0) {
        xSpawn += random->nextInt(8) - random->nextInt(8);
        zSpawn += random->nextInt(8) - random->nextInt(8);
        if (++tries == 10000) break;
    }
    levelData->setXSpawn(xSpawn);
    levelData->setZSpawn(zSpawn);
}









void ServerLevel::tickTiles() {
    
    int iLev = 0;
    if (dimension->id == -1) {
        iLev = 1;
    } else if (dimension->id == 1) {
        iLev = 2;
    }
    chunksToPoll.clear();

    unsigned int tickCount = 0;

    {
        std::lock_guard<std::recursive_mutex> lock(m_updateCS[iLev]);
        
        
        
        


        for (int i = 0; i < m_updateTileCount[iLev]; i++) {
            int x = m_updateTileX[iLev][i];
            int y = m_updateTileY[iLev][i];
            int z = m_updateTileZ[iLev][i];
            if (hasChunkAt(x, y, z)) {
                int id = getTile(x, y, z);
                if (Tile::tiles[id] != nullptr &&
                    Tile::tiles[id]->isTicking()) {
                    


                    Tile::tiles[id]->tick(this, x, y, z, random);
                }
            }
        }
        
        
        
        m_updateTileCount[iLev] = 0;
        m_updateChunkCount[iLev] = 0;
    }

    Level::tickTiles();

    
    int prob = 100000;
    if (app.GetGameSettingsDebugMask() & (1L << eDebugSetting_RegularLightning))
        prob = 100;

    auto itEndCtp = chunksToPoll.end();
    for (auto it = chunksToPoll.begin(); it != itEndCtp; it++) {
        ChunkPos cp = *it;
        int xo = cp.x * 16;
        int zo = cp.z * 16;

        
        
        
        if (!this->hasChunk(cp.x, cp.z)) continue;

        
        
        
        
        
        

        
        
        
        
        

        
        
        assert(m_updateChunkCount[iLev] < LEVEL_CHUNKS_TO_UPDATE_MAX);

        m_updateChunkX[iLev][m_updateChunkCount[iLev]] = cp.x;
        m_updateChunkZ[iLev][m_updateChunkCount[iLev]++] = cp.z;

        LevelChunk* lc = getChunk(cp.x, cp.z);
        tickClientSideTiles(xo, zo, lc);

        if (random->nextInt(prob) == 0 && isRaining() && isThundering()) {
            randValue = randValue * 3 + addend;
            int val = (randValue >> 2);
            int x = xo + (val & 15);
            int z = zo + ((val >> 8) & 15);
            int y = getTopRainBlock(x, z);

            if (isRainingAt(x, y, z)) {
                addGlobalEntity(std::shared_ptr<LightningBolt>(
                    new LightningBolt(this, x, y, z)));
            }
        }

        
        if (random->nextInt(16) == 0) {
            randValue = randValue * 3 + addend;
            int val = (randValue >> 2);
            int x = (val & 15);
            int z = ((val >> 8) & 15);
            int yy = getTopRainBlock(x + xo, z + zo);
            if (shouldFreeze(x + xo, yy - 1, z + zo)) {
                setTileAndUpdate(x + xo, yy - 1, z + zo, Tile::ice_Id);
            }
            if (isRaining() && shouldSnow(x + xo, yy, z + zo)) {
                setTileAndUpdate(x + xo, yy, z + zo, Tile::topSnow_Id);
            }
            if (isRaining()) {
                Biome* b = getBiome(x + xo, z + zo);
                if (b->hasRain()) {
                    int tile = getTile(x + xo, yy - 1, z + zo);
                    if (tile != 0) {
                        Tile::tiles[tile]->handleRain(this, x + xo, yy - 1,
                                                      z + zo);
                    }
                }
            }
        }

        
        checkLight(xo + random->nextInt(16), random->nextInt(128),
                   zo + random->nextInt(16));
    }

    m_level[iLev] = this;
    m_randValue[iLev] = randValue;
    
    m_updateTrigger->set(iLev);
}

bool ServerLevel::isTileToBeTickedAt(int x, int y, int z, int tileId) {
    TickNextTickData td = TickNextTickData(x, y, z, tileId);
    return find(toBeTicked.begin(), toBeTicked.end(), td) != toBeTicked.end();
}

void ServerLevel::addToTickNextTick(int x, int y, int z, int tileId,
                                    int tickDelay) {
    addToTickNextTick(x, y, z, tileId, tickDelay, 0);
}

void ServerLevel::addToTickNextTick(int x, int y, int z, int tileId,
                                    int tickDelay, int priorityTilt) {
    TickNextTickData td = TickNextTickData(x, y, z, tileId);
    int r = 0;
    if (getInstaTick() && tileId > 0) {
        if (Tile::tiles[tileId]->canInstantlyTick()) {
            r = 8;
            if (hasChunksAt(td.x - r, td.y - r, td.z - r, td.x + r, td.y + r,
                            td.z + r)) {
                int id = getTile(td.x, td.y, td.z);
                if (id == td.tileId && id > 0) {
                    Tile::tiles[id]->tick(this, td.x, td.y, td.z, random);
                }
            }
            return;
        } else {
            tickDelay = 1;
        }
    }

    if (hasChunksAt(x - r, y - r, z - r, x + r, y + r, z + r)) {
        if (tileId > 0) {
            td.delay(tickDelay + levelData->getGameTime());
            td.setPriorityTilt(priorityTilt);
        }
        {
            std::lock_guard<std::recursive_mutex> lock(m_tickNextTickCS);
            if (tickNextTickSet.find(td) == tickNextTickSet.end()) {
                tickNextTickSet.insert(td);
                tickNextTickList.insert(td);
            }
        }
    }
}

void ServerLevel::forceAddTileTick(int x, int y, int z, int tileId,
                                   int tickDelay, int prioTilt) {
    TickNextTickData td = TickNextTickData(x, y, z, tileId);
    td.setPriorityTilt(prioTilt);

    if (tileId > 0) {
        td.delay(tickDelay + levelData->getGameTime());
    }
    {
        std::lock_guard<std::recursive_mutex> lock(m_tickNextTickCS);
        if (tickNextTickSet.find(td) == tickNextTickSet.end()) {
            tickNextTickSet.insert(td);
            tickNextTickList.insert(td);
        }
    }
}

void ServerLevel::tickEntities() {
    if (players.empty()) {
        if (emptyTime++ >= EMPTY_TIME_NO_TICK) {
            return;
        }
    } else {
        resetEmptyTime();
    }

    Level::tickEntities();
}

void ServerLevel::resetEmptyTime() { emptyTime = 0; }

bool ServerLevel::tickPendingTicks(bool force) {
    std::lock_guard<std::recursive_mutex> lock(m_tickNextTickCS);
    int count = (int)tickNextTickList.size();
    int count2 = (int)tickNextTickSet.size();
    if (count != tickNextTickSet.size()) {
        
        
    }
    if (count > MAX_TICK_TILES_PER_TICK) count = MAX_TICK_TILES_PER_TICK;

    auto itTickList = tickNextTickList.begin();
    for (int i = 0; i < count; i++) {
        TickNextTickData td = *(itTickList);
        if (!force && td.m_delay > levelData->getGameTime()) {
            break;
        }

        itTickList = tickNextTickList.erase(itTickList);
        tickNextTickSet.erase(td);
        toBeTicked.push_back(td);
    }

    for (auto it = toBeTicked.begin(); it != toBeTicked.end();) {
        TickNextTickData td = *it;
        it = toBeTicked.erase(it);

        int r = 0;
        if (hasChunksAt(td.x - r, td.y - r, td.z - r, td.x + r, td.y + r,
                        td.z + r)) {
            int id = getTile(td.x, td.y, td.z);
            if (id > 0 && Tile::isMatching(id, td.tileId)) {
                Tile::tiles[id]->tick(this, td.x, td.y, td.z, random);
            }
        } else {
            addToTickNextTick(td.x, td.y, td.z, td.tileId, 0);
        }
    }

    toBeTicked.clear();

    int count3 = (int)tickNextTickList.size();
    int count4 = (int)tickNextTickSet.size();

    bool retval = tickNextTickList.size() != 0;

    return retval;
}

std::vector<TickNextTickData>* ServerLevel::fetchTicksInChunk(LevelChunk* chunk,
                                                              bool remove) {
    std::lock_guard<std::recursive_mutex> lock(m_tickNextTickCS);
    std::vector<TickNextTickData>* results = new std::vector<TickNextTickData>;

    ChunkPos* pos = chunk->getPos();
    
    int xMin = ((unsigned)pos->x << 4) - 2;
    int xMax = (xMin + 16) + 2;
    int zMin = ((unsigned)pos->z << 4) - 2;
    int zMax = (zMin + 16) + 2;
    delete pos;

    for (int i = 0; i < 2; i++) {
        if (i == 0) {
            for (auto it = tickNextTickList.begin();
                 it != tickNextTickList.end();) {
                TickNextTickData td = *it;

                if (td.x >= xMin && td.x < xMax && td.z >= zMin &&
                    td.z < zMax) {
                    if (remove) {
                        tickNextTickSet.erase(td);
                        it = tickNextTickList.erase(it);
                    } else {
                        it++;
                    }

                    results->push_back(td);
                } else {
                    it++;
                }
            }
        } else {
            if (!toBeTicked.empty()) {
                app.DebugPrintf("To be ticked size: %d\n", toBeTicked.size());
            }
            for (auto it = toBeTicked.begin(); it != toBeTicked.end();) {
                TickNextTickData td = *it;

                if (td.x >= xMin && td.x < xMax && td.z >= zMin &&
                    td.z < zMax) {
                    if (remove) {
                        tickNextTickList.erase(td);
                        it = toBeTicked.erase(it);
                    } else {
                        it++;
                    }

                    results->push_back(td);
                } else {
                    it++;
                }
            }
        }
    }

    return results;
}

void ServerLevel::tick(std::shared_ptr<Entity> e, bool actual) {
    if (!server->isAnimals() &&
        (e->instanceof(eTYPE_ANIMAL) || e->instanceof(eTYPE_WATERANIMAL))) {
        e->remove();
    }
    if (!server->isNpcsEnabled() &&
        (std::dynamic_pointer_cast<Npc>(e) != nullptr)) {
        e->remove();
    }
    Level::tick(e, actual);
}

void ServerLevel::forceTick(std::shared_ptr<Entity> e, bool actual) {
    Level::tick(e, actual);
}

ChunkSource* ServerLevel::createChunkSource() {
    ChunkStorage* storage = levelStorage->createChunkStorage(dimension);
    cache = new ServerChunkCache(this, storage,
                                 dimension->createRandomLevelSource());
    return cache;
}

std::vector<std::shared_ptr<TileEntity> >* ServerLevel::getTileEntitiesInRegion(
    int x0, int y0, int z0, int x1, int y1, int z1) {
    std::vector<std::shared_ptr<TileEntity> >* result =
        new std::vector<std::shared_ptr<TileEntity> >;
    for (unsigned int i = 0; i < tileEntityList.size(); i++) {
        std::shared_ptr<TileEntity> te = tileEntityList[i];
        if (te->x >= x0 && te->y >= y0 && te->z >= z0 && te->x < x1 &&
            te->y < y1 && te->z < z1) {
            result->push_back(te);
        }
    }
    return result;
}

bool ServerLevel::mayInteract(std::shared_ptr<Player> player, int xt, int yt,
                              int zt, int content) {
    
    
    

    
    if (content != Tile::lava_Id) {
        
        return true;
    } else if (dimension->id == 0)  
    {
        return !server->isUnderSpawnProtection(this, xt, yt, zt, player);
    }
    return true;
}

void ServerLevel::initializeLevel(LevelSettings* settings) {
    setInitialSpawn(settings);

    Level::initializeLevel(settings);
}





void ServerLevel::setInitialSpawn(LevelSettings* levelSettings) {
    if (!dimension->mayRespawn()) {
        levelData->setSpawn(0, dimension->getSpawnYPosition(), 0);
        return;
    }

    isFindingSpawn = true;

    BiomeSource* biomeSource = dimension->biomeSource;
    std::vector<Biome*> playerSpawnBiomes = biomeSource->getPlayerSpawnBiomes();
    Random random(getSeed());

    TilePos* findBiome =
        biomeSource->findBiome(0, 0, 16 * 16, playerSpawnBiomes, &random);

    int xSpawn = 0;  
    int ySpawn = dimension->getSpawnYPosition();
    int zSpawn = 0;  
    int minXZ = -(dimension->getXZSize() * 16) / 2;
    int maxXZ = (dimension->getXZSize() * 16) / 2 - 1;

    if (findBiome != nullptr) {
        xSpawn = findBiome->x;
        zSpawn = findBiome->z;
        delete findBiome;
    } else {
        app.DebugPrintf(
            "Level::setInitialSpawn - Unable to find spawn biome\n");
    }

    int tries = 0;

    while (!dimension->isValidSpawn(xSpawn, zSpawn)) {
        
        xSpawn += random.nextInt(64) - random.nextInt(64);
        if (xSpawn > maxXZ) xSpawn = 0;
        if (xSpawn < minXZ) xSpawn = 0;
        zSpawn += random.nextInt(64) - random.nextInt(64);
        if (zSpawn > maxXZ) zSpawn = 0;
        if (zSpawn < minXZ) zSpawn = 0;

        if (++tries == 1000) break;
    }

    levelData->setSpawn(xSpawn, ySpawn, zSpawn);
    if (levelSettings->hasStartingBonusItems()) {
        generateBonusItemsNearSpawn();
    }
    isFindingSpawn = false;
}


void ServerLevel::generateBonusItemsNearSpawn() {
    
    
    
    

    static const int r = 20;
    int xs = levelData->getXSpawn();
    int zs = levelData->getZSpawn();
    for (int xx = -r; xx <= r; xx++)
        for (int zz = -r; zz <= r; zz++) {
            int x = xx + xs;
            int z = zz + zs;
            int y = getTopSolidBlock(x, z) - 1;

            if (getTile(x, y, z) == Tile::chest_Id) {
                std::shared_ptr<ChestTileEntity> chest =
                    std::dynamic_pointer_cast<ChestTileEntity>(
                        getTileEntity(x, y, z));
                if (chest != nullptr) {
                    if (chest->isBonusChest) {
                        return;
                    }
                }
            }
        }

    BonusChestFeature* feature = new BonusChestFeature(RANDOM_BONUS_ITEMS, 16);
    for (int attempt = 0; attempt < 16; attempt++) {
        int x =
            levelData->getXSpawn() + random->nextInt(6) - random->nextInt(6);
        int z =
            levelData->getZSpawn() + random->nextInt(6) - random->nextInt(6);
        int y = getTopSolidBlock(x, z) + 1;

        if (feature->place(this, random, x, y, z, (attempt == 15))) {
            break;
        }
    }
    delete feature;
}

Pos* ServerLevel::getDimensionSpecificSpawn() {
    return dimension->getSpawnPos();
}


void ServerLevel::Suspend() {
    if (StorageManager.GetSaveDisabled()) return;
    saveLevelData();
    chunkSource->saveAllEntities();
}

void ServerLevel::save(bool force, ProgressListener* progressListener,
                       bool bAutosave) {
    if (!chunkSource->shouldSave()) return;

    
    if (StorageManager.GetSaveDisabled()) return;

    if (progressListener != nullptr) {
        if (bAutosave) {
            progressListener->progressStartNoAbort(
                IDS_PROGRESS_AUTOSAVING_LEVEL);
        } else {
            progressListener->progressStartNoAbort(IDS_PROGRESS_SAVING_LEVEL);
        }
    }
    saveLevelData();

    if (progressListener != nullptr)
        progressListener->progressStage(IDS_PROGRESS_SAVING_CHUNKS);

    {
        chunkSource->save(force, progressListener);

#if defined(_LARGE_WORLDS)
        
        if (chunkMap->players.size() > 0) {
            
            
            std::vector<LevelChunk*>* loadedChunkList =
                cache->getLoadedChunkList();
            for (auto it = loadedChunkList->begin();
                 it != loadedChunkList->end(); ++it) {
                LevelChunk* lc = *it;
                if (!chunkMap->hasChunk(lc->x, lc->z)) {
                    cache->drop(lc->x, lc->z);
                }
            }
        }
#endif
    }

    
    
    
    
    
    
}


void ServerLevel::saveToDisc(ProgressListener* progressListener,
                             bool autosave) {
    
    if (StorageManager.GetSaveDisabled()) return;

    
    
    if (!Minecraft::GetInstance()->skins->isUsingDefaultSkin()) {
        TexturePack* tPack = Minecraft::GetInstance()->skins->getSelected();
        DLCTexturePack* pDLCTexPack = (DLCTexturePack*)tPack;

        DLCPack* pDLCPack = pDLCTexPack->getDLCInfoParentPack();

        if (!pDLCPack->hasPurchasedFile(DLCManager::e_DLCType_Texture, L"")) {
            return;
        }
    }

    if (progressListener != nullptr)
        progressListener->progressStage(IDS_PROGRESS_SAVING_TO_DISC);
    levelStorage->flushSaveFile(autosave);
}

void ServerLevel::saveLevelData() {
    checkSession();

    levelStorage->saveLevelData(levelData, &players);
    savedDataStorage->save();
}

void ServerLevel::entityAdded(std::shared_ptr<Entity> e) {
    Level::entityAdded(e);
    entitiesById[e->entityId] = e;
    std::vector<std::shared_ptr<Entity> >* es = e->getSubEntities();
    if (es != nullptr) {
        
        for (auto it = es->begin(); it != es->end(); ++it) {
            entitiesById.insert(
                intEntityMap::value_type((*it)->entityId, (*it)));
        }
    }
    entityAddedExtra(e);  
}

void ServerLevel::entityRemoved(std::shared_ptr<Entity> e) {
    Level::entityRemoved(e);
    entitiesById.erase(e->entityId);
    std::vector<std::shared_ptr<Entity> >* es = e->getSubEntities();
    if (es != nullptr) {
        
        for (auto it = es->begin(); it != es->end(); ++it) {
            entitiesById.erase((*it)->entityId);
        }
    }
    entityRemovedExtra(e);  
}

std::shared_ptr<Entity> ServerLevel::getEntity(int id) {
    return entitiesById[id];
}

bool ServerLevel::addGlobalEntity(std::shared_ptr<Entity> e) {
    if (Level::addGlobalEntity(e)) {
        server->getPlayers()->broadcast(e->x, e->y, e->z, 512, dimension->id,
                                        std::shared_ptr<AddGlobalEntityPacket>(
                                            new AddGlobalEntityPacket(e)));
        return true;
    }
    return false;
}

void ServerLevel::broadcastEntityEvent(std::shared_ptr<Entity> e,
                                       uint8_t event) {
    std::shared_ptr<Packet> p = std::shared_ptr<EntityEventPacket>(
        new EntityEventPacket(e->entityId, event));
    server->getLevel(dimension->id)->getTracker()->broadcastAndSend(e, p);
}

std::shared_ptr<Explosion> ServerLevel::explode(std::shared_ptr<Entity> source,
                                                double x, double y, double z,
                                                float r, bool fire,
                                                bool destroyBlocks) {
    
    
    std::shared_ptr<Explosion> explosion =
        std::make_shared<Explosion>(this, source, x, y, z, r);
    explosion->fire = fire;
    explosion->destroyBlocks = destroyBlocks;
    explosion->explode();
    explosion->finalizeExplosion(false);

    if (!destroyBlocks) {
        explosion->toBlow.clear();
    }

    std::vector<std::shared_ptr<ServerPlayer> > sentTo;
    for (auto it = players.begin(); it != players.end(); ++it) {
        std::shared_ptr<ServerPlayer> player =
            std::dynamic_pointer_cast<ServerPlayer>(*it);
        if (player->dimension != dimension->id) continue;

        bool knockbackOnly = false;
        if (sentTo.size()) {
            INetworkPlayer* thisPlayer = player->connection->getNetworkPlayer();
            if (thisPlayer == nullptr) {
                continue;
            } else {
                for (unsigned int j = 0; j < sentTo.size(); j++) {
                    std::shared_ptr<ServerPlayer> player2 = sentTo[j];
                    INetworkPlayer* otherPlayer =
                        player2->connection->getNetworkPlayer();
                    if (otherPlayer != nullptr &&
                        thisPlayer->IsSameSystem(otherPlayer)) {
                        knockbackOnly = true;
                    }
                }
            }
        }

        if (player->distanceToSqr(x, y, z) < 64 * 64) {
            Vec3 knockbackVec = explosion->getHitPlayerKnockback(player);
            
            
            
            
            player->connection->send(std::shared_ptr<ExplodePacket>(
                new ExplodePacket(x, y, z, r, &explosion->toBlow, &knockbackVec,
                                  knockbackOnly)));
            sentTo.push_back(player);
        }
    }

    return explosion;
}

void ServerLevel::tileEvent(int x, int y, int z, int tile, int b0, int b1) {
    
    
    
    TileEventData newEvent(x, y, z, tile, b0, b1);
    
    for (auto it = tileEvents[activeTileEventsList].begin();
         it != tileEvents[activeTileEventsList].end(); ++it) {
        if ((*it).equals(newEvent)) {
            return;
        }
    }
    tileEvents[activeTileEventsList].push_back(newEvent);
}

void ServerLevel::runTileEvents() {
    
    
    while (!tileEvents[activeTileEventsList].empty()) {
        int runList = activeTileEventsList;
        activeTileEventsList ^= 1;

        
        for (auto it = tileEvents[runList].begin();
             it != tileEvents[runList].end(); ++it) {
            if (doTileEvent(&(*it))) {
                TileEventData te = *it;
                server->getPlayers()->broadcast(
                    te.getX(), te.getY(), te.getZ(), 64, dimension->id,
                    std::make_shared<TileEventPacket>(
                        te.getX(), te.getY(), te.getZ(), te.getTile(),
                        te.getParamA(), te.getParamB()));
            }
        }
        tileEvents[runList].clear();
    }
}

bool ServerLevel::doTileEvent(TileEventData* te) {
    int t = getTile(te->getX(), te->getY(), te->getZ());
    if (t == te->getTile()) {
        return Tile::tiles[t]->triggerEvent(this, te->getX(), te->getY(),
                                            te->getZ(), te->getParamA(),
                                            te->getParamB());
    }
    return false;
}

void ServerLevel::closeLevelStorage() { levelStorage->closeAll(); }

void ServerLevel::tickWeather() {
    bool wasRaining = isRaining();
    Level::tickWeather();

    if (wasRaining != isRaining()) {
        if (wasRaining) {
            server->getPlayers()->broadcastAll(std::shared_ptr<GameEventPacket>(
                new GameEventPacket(GameEventPacket::STOP_RAINING, 0)));
        } else {
            server->getPlayers()->broadcastAll(std::shared_ptr<GameEventPacket>(
                new GameEventPacket(GameEventPacket::START_RAINING, 0)));
        }
    }
}

MinecraftServer* ServerLevel::getServer() { return server; }

EntityTracker* ServerLevel::getTracker() { return tracker; }

void ServerLevel::setTimeAndAdjustTileTicks(int64_t newTime) {
    int64_t delta = newTime - levelData->getGameTime();
    
    
    
    
    std::vector<TickNextTickData> temp;
    for (auto it = tickNextTickList.begin(); it != tickNextTickList.end();
         ++it) {
        temp.push_back(*it);
        temp.back().m_delay += delta;
    }
    tickNextTickList.clear();
    for (unsigned int i = 0; i < temp.size(); i++) {
        tickNextTickList.insert(temp[i]);
    }
    setGameTime(newTime);
}

PlayerChunkMap* ServerLevel::getChunkMap() { return chunkMap; }

PortalForcer* ServerLevel::getPortalForcer() { return portalForcer; }

void ServerLevel::sendParticles(const std::wstring& name, double x, double y,
                                double z, int count) {
    sendParticles(name, x + 0.5f, y + 0.5f, z + 0.5f, count, 0.5f, 0.5f, 0.5f,
                  0.02f);
}

void ServerLevel::sendParticles(const std::wstring& name, double x, double y,
                                double z, int count, double xDist, double yDist,
                                double zDist, double speed) {
    std::shared_ptr<Packet> packet = std::make_shared<LevelParticlesPacket>(
        name, (float)x, (float)y, (float)z, (float)xDist, (float)yDist,
        (float)zDist, (float)speed, count);

    for (auto it = players.begin(); it != players.end(); ++it) {
        std::shared_ptr<ServerPlayer> player =
            std::dynamic_pointer_cast<ServerPlayer>(*it);
        player->connection->send(packet);
    }
}



void ServerLevel::queueSendTileUpdate(int x, int y, int z) {
    std::lock_guard<std::recursive_mutex> lock(m_csQueueSendTileUpdates);
    m_queuedSendTileUpdates.push_back(new Pos(x, y, z));
}

void ServerLevel::runQueuedSendTileUpdates() {
    std::lock_guard<std::recursive_mutex> lock(m_csQueueSendTileUpdates);
    for (auto it = m_queuedSendTileUpdates.begin();
         it != m_queuedSendTileUpdates.end(); ++it) {
        Pos* p = *it;
        sendTileUpdated(p->x, p->y, p->z);
        delete p;
    }
    m_queuedSendTileUpdates.clear();
}



bool ServerLevel::addEntity(std::shared_ptr<Entity> e) {
    
    if (e->instanceof(eTYPE_ITEMENTITY)) {
        
        
        std::lock_guard<std::recursive_mutex> lock(m_limiterCS);
        if (m_itemEntities.size() >= MAX_ITEM_ENTITIES) {
            
            removeEntityImmediately(m_itemEntities.front());
        }
    }
    
    
    else if (e->instanceof(eTYPE_HANGING_ENTITY)) {
        
        
        std::lock_guard<std::recursive_mutex> lock(m_limiterCS);
        if (m_hangingEntities.size() >= MAX_HANGING_ENTITIES) {
            

            
            
            return false;

            
        }
    }
    
    else if (e->instanceof(eTYPE_ARROW)) {
        
        
        std::lock_guard<std::recursive_mutex> lock(m_limiterCS);
        if (m_arrowEntities.size() >= MAX_ARROW_ENTITIES) {
            
            removeEntityImmediately(m_arrowEntities.front());
        }
    }
    
    
    else if (e->instanceof(eTYPE_EXPERIENCEORB)) {
        
        
        std::lock_guard<std::recursive_mutex> lock(m_limiterCS);
        if (m_experienceOrbEntities.size() >= MAX_EXPERIENCEORB_ENTITIES) {
            
            removeEntityImmediately(m_experienceOrbEntities.front());
        }
    }
    return Level::addEntity(e);
}



bool ServerLevel::atEntityLimit(std::shared_ptr<Entity> e) {
    

    bool atLimit = false;

    if (e->instanceof(eTYPE_ITEMENTITY)) {
        std::lock_guard<std::recursive_mutex> lock(m_limiterCS);
        atLimit = m_itemEntities.size() >= MAX_ITEM_ENTITIES;
    } else if (e->instanceof(eTYPE_HANGING_ENTITY)) {
        std::lock_guard<std::recursive_mutex> lock(m_limiterCS);
        atLimit = m_hangingEntities.size() >= MAX_HANGING_ENTITIES;
    } else if (e->instanceof(eTYPE_ARROW)) {
        std::lock_guard<std::recursive_mutex> lock(m_limiterCS);
        atLimit = m_arrowEntities.size() >= MAX_ARROW_ENTITIES;
    } else if (e->instanceof(eTYPE_EXPERIENCEORB)) {
        std::lock_guard<std::recursive_mutex> lock(m_limiterCS);
        atLimit = m_experienceOrbEntities.size() >= MAX_EXPERIENCEORB_ENTITIES;
    }

    return atLimit;
}


void ServerLevel::entityAddedExtra(std::shared_ptr<Entity> e) {
    if (e->instanceof(eTYPE_ITEMENTITY)) {
        std::lock_guard<std::recursive_mutex> lock(m_limiterCS);
        m_itemEntities.push_back(e);
        
        
    } else if (e->instanceof(eTYPE_HANGING_ENTITY)) {
        std::lock_guard<std::recursive_mutex> lock(m_limiterCS);
        m_hangingEntities.push_back(e);
        
        
    } else if (e->instanceof(eTYPE_ARROW)) {
        std::lock_guard<std::recursive_mutex> lock(m_limiterCS);
        m_arrowEntities.push_back(e);
        
        
    } else if (e->instanceof(eTYPE_EXPERIENCEORB)) {
        std::lock_guard<std::recursive_mutex> lock(m_limiterCS);
        m_experienceOrbEntities.push_back(e);
        
        
    } else if (e->instanceof(eTYPE_PRIMEDTNT)) {
        std::lock_guard<std::recursive_mutex> lock(m_limiterCS);
        m_primedTntCount++;
    } else if (e->instanceof(eTYPE_FALLINGTILE)) {
        std::lock_guard<std::recursive_mutex> lock(m_limiterCS);
        m_fallingTileCount++;
    }
}



void ServerLevel::entityRemovedExtra(std::shared_ptr<Entity> e) {
    if (e->instanceof(eTYPE_ITEMENTITY)) {
        std::lock_guard<std::recursive_mutex> lock(m_limiterCS);
        
        
        auto it = find(m_itemEntities.begin(), m_itemEntities.end(), e);
        if (it != m_itemEntities.end()) {
            
            m_itemEntities.erase(it);
        }
        
        
    } else if (e->instanceof(eTYPE_HANGING_ENTITY)) {
        std::lock_guard<std::recursive_mutex> lock(m_limiterCS);
        
        
        auto it = find(m_hangingEntities.begin(), m_hangingEntities.end(), e);
        if (it != m_hangingEntities.end()) {
            
            m_hangingEntities.erase(it);
        }
        
        
    } else if (e->instanceof(eTYPE_ARROW)) {
        std::lock_guard<std::recursive_mutex> lock(m_limiterCS);
        
        
        auto it = find(m_arrowEntities.begin(), m_arrowEntities.end(), e);
        if (it != m_arrowEntities.end()) {
            
            m_arrowEntities.erase(it);
        }
        
        
    } else if (e->instanceof(eTYPE_EXPERIENCEORB)) {
        std::lock_guard<std::recursive_mutex> lock(m_limiterCS);
        
        
        auto it = find(m_experienceOrbEntities.begin(),
                       m_experienceOrbEntities.end(), e);
        if (it != m_experienceOrbEntities.end()) {
            
            m_experienceOrbEntities.erase(it);
        }
        
        
    } else if (e->instanceof(eTYPE_PRIMEDTNT)) {
        std::lock_guard<std::recursive_mutex> lock(m_limiterCS);
        m_primedTntCount--;
    } else if (e->instanceof(eTYPE_FALLINGTILE)) {
        std::lock_guard<std::recursive_mutex> lock(m_limiterCS);
        m_fallingTileCount--;
    }
}

bool ServerLevel::newPrimedTntAllowed() {
    std::lock_guard<std::recursive_mutex> lock(m_limiterCS);
    bool retval = m_primedTntCount < MAX_PRIMED_TNT;
    return retval;
}

bool ServerLevel::newFallingTileAllowed() {
    std::lock_guard<std::recursive_mutex> lock(m_limiterCS);
    bool retval = m_fallingTileCount < MAX_FALLING_TILE;
    return retval;
}

int ServerLevel::runUpdate(void* lpParam) {
    ShutdownManager::HasStarted(ShutdownManager::eRunUpdateThread,
                                m_updateTrigger);
    while (ShutdownManager::ShouldRun(ShutdownManager::eRunUpdateThread)) {
        m_updateTrigger->waitForAll(C4JThread::kInfiniteTimeout);

        if (!ShutdownManager::ShouldRun(ShutdownManager::eRunUpdateThread))
            break;

        
        
        int grassTicks = 0;
        int lavaTicks = 0;
        for (unsigned int iLev = 0; iLev < 3; ++iLev) {
            std::lock_guard<std::recursive_mutex> lock(m_updateCS[iLev]);
            for (int i = 0; i < m_updateChunkCount[iLev]; i++) {
                
                
                
                
                
                int cx = m_updateChunkX[iLev][i];
                int cz = m_updateChunkZ[iLev][i];
                int minx = 0;
                int maxx = 15;
                int minz = 0;
                int maxz = 15;
                if (!m_level[iLev]->hasChunk(cx, cz)) continue;

                if (!m_level[iLev]->hasChunk(cx + 1, cz + 0)) {
                    maxx = 11;
                }
                if (!m_level[iLev]->hasChunk(cx + 0, cz + 1)) {
                    maxz = 11;
                }
                if (!m_level[iLev]->hasChunk(cx - 1, cz + 0)) {
                    minx = 4;
                }
                if (!m_level[iLev]->hasChunk(cx + 0, cz - 1)) {
                    minz = 4;
                }
                if (!m_level[iLev]->hasChunk(cx + 1, cz + 1)) {
                    maxx = 11;
                    maxz = 11;
                }
                if (!m_level[iLev]->hasChunk(cx + 1, cz - 1)) {
                    maxx = 11;
                    minz = 4;
                }
                if (!m_level[iLev]->hasChunk(cx - 1, cz - 1)) {
                    minx = 4;
                    minz = 4;
                }
                if (!m_level[iLev]->hasChunk(cx - 1, cz + 1)) {
                    minx = 4;
                    maxz = 11;
                }

                LevelChunk* lc = m_level[iLev]->getChunk(cx, cz);

                for (int j = 0; j < 80; j++) {
                    
                    m_randValue[iLev] = (unsigned)m_randValue[iLev] * 3 +
                                        (unsigned)m_level[iLev]->addend;
                    int val = (m_randValue[iLev] >> 2);
                    int x = (val & 15);
                    if ((x < minx) || (x > maxx)) continue;
                    int z = ((val >> 8) & 15);
                    if ((z < minz) || (z > maxz)) continue;
                    int y = ((val >> 16) & (Level::maxBuildHeight - 1));

                    
                    int id = lc->getTile(x, y, z);
                    if (m_updateTileCount[iLev] >= MAX_UPDATES) break;

                    
                    
                    if ((id == Tile::grass_Id &&
                         grassTicks >= MAX_GRASS_TICKS) ||
                        (id == Tile::calmLava_Id &&
                         lavaTicks >= MAX_LAVA_TICKS))
                        continue;

                    
                    
                    
                    if (Tile::tiles[id] != nullptr &&
                        Tile::tiles[id]->isTicking() &&
                        Tile::tiles[id]->shouldTileTick(
                            m_level[iLev], x + (cx * 16), y, z + (cz * 16))) {
                        if (id == Tile::grass_Id)
                            ++grassTicks;
                        else if (id == Tile::calmLava_Id)
                            ++lavaTicks;
                        m_updateTileX[iLev][m_updateTileCount[iLev]] =
                            x + (cx * 16);
                        m_updateTileY[iLev][m_updateTileCount[iLev]] = y;
                        m_updateTileZ[iLev][m_updateTileCount[iLev]] =
                            z + (cz * 16);

                        m_updateTileCount[iLev]++;
                    }
                }
            }
        }
    }

    ShutdownManager::HasFinished(ShutdownManager::eRunUpdateThread);

    return 0;
}

void ServerLevel::flagEntitiesToBeRemoved(unsigned int* flags,
                                          bool* removedFound) {
    if (chunkMap) {
        chunkMap->flagEntitiesToBeRemoved(flags, removedFound);
    }
}
