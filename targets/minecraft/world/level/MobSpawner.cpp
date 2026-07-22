#include "MobSpawner.h"

#include <utility>
#include <vector>

#include "Level.h"
#include "java/Class.h"
#include "java/Random.h"
#include "minecraft/Pos.h"
#include "minecraft/server/level/ServerLevel.h"
#include "minecraft/util/Mth.h"
#include "minecraft/world/Difficulty.h"
#include "minecraft/world/entity/Entity.h"
#include "minecraft/world/entity/EntityIO.h"
#include "minecraft/world/entity/Mob.h"
#include "minecraft/world/entity/MobCategory.h"
#include "minecraft/world/entity/player/Player.h"
#include "minecraft/world/level/ChunkPos.h"
#include "minecraft/world/level/TilePos.h"
#include "minecraft/world/level/biome/Biome.h"
#include "minecraft/world/level/dimension/Dimension.h"
#include "minecraft/world/level/material/Material.h"
#include "minecraft/world/level/tile/Tile.h"

class MobGroupData;

const int MobSpawner::MIN_SPAWN_DISTANCE = 24;

TilePos MobSpawner::getRandomPosWithin(Level* level, int cx, int cz) {
    
    
    
    
    int x = cx * 16 + level->random->nextInt(16);
    int y = level->random->nextInt(level->getHeight());
    int z = cz * 16 + level->random->nextInt(16);

    return TilePos(x, y, z);
}

std::unordered_map<ChunkPos, bool, ChunkPosKeyHash, ChunkPosKeyEq>
    MobSpawner::chunksToPoll;

const int MobSpawner::tick(ServerLevel* level, bool spawnEnemies,
                           bool spawnFriendlies, bool spawnPersistent) {
    if (!spawnEnemies && !spawnFriendlies && !spawnPersistent) {
        return 0;
    }
    chunksToPoll.clear();

    
    
    
    
    
    int playerCount = (int)level->players.size();
    int* xx = new int[playerCount];
    int* zz = new int[playerCount];
    for (int i = 0; i < playerCount; i++) {
        std::shared_ptr<Player> player = level->players[i];
        xx[i] = Mth::floor(player->x / 16);
        zz[i] = Mth::floor(player->z / 16);
        chunksToPoll.insert(
            std::pair<ChunkPos, bool>(ChunkPos(xx[i], zz[i]), false));
    }

    for (int r = 1; r <= 8; r++) {
        for (int l = 0; l < (r * 2); l++) {
            for (int i = 0; i < playerCount; i++) {
                bool edgeChunk = (r == 8);

                
                
                
                if (!edgeChunk) {
                    chunksToPoll.insert(std::pair<ChunkPos, bool>(
                        ChunkPos((xx[i] - r) + l, (zz[i] - r)), false));
                    chunksToPoll.insert(std::pair<ChunkPos, bool>(
                        ChunkPos((xx[i] + r), (zz[i] - r) + l), false));
                    chunksToPoll.insert(std::pair<ChunkPos, bool>(
                        ChunkPos((xx[i] + r) - l, (zz[i] + r)), false));
                    chunksToPoll.insert(std::pair<ChunkPos, bool>(
                        ChunkPos((xx[i] - r), (zz[i] + r) - l), false));
                } else {
                    ChunkPos cp = ChunkPos((xx[i] - r) + l, (zz[i] - r));
                    if (chunksToPoll.find(cp) == chunksToPoll.end())
                        chunksToPoll.insert(
                            std::pair<ChunkPos, bool>(cp, true));
                    cp = ChunkPos((xx[i] + r), (zz[i] - r) + l);
                    if (chunksToPoll.find(cp) == chunksToPoll.end())
                        chunksToPoll.insert(
                            std::pair<ChunkPos, bool>(cp, true));
                    cp = ChunkPos((xx[i] + r) - l, (zz[i] + r));
                    if (chunksToPoll.find(cp) == chunksToPoll.end())
                        chunksToPoll.insert(
                            std::pair<ChunkPos, bool>(cp, true));
                    cp = ChunkPos((xx[i] - r), (zz[i] + r) - l);
                    if (chunksToPoll.find(cp) == chunksToPoll.end())
                        chunksToPoll.insert(
                            std::pair<ChunkPos, bool>(cp, true));
                }
            }
        }
    }
    delete[] xx;
    delete[] zz;
    int count = 0;
    Pos* spawnPos = level->getSharedSpawnPos();

    for (unsigned int i = 0; i < MobCategory::values.size(); i++) {
        MobCategory* mobCategory = MobCategory::values[i];
        if ((mobCategory->isFriendly() && !spawnFriendlies) ||
            (!mobCategory->isFriendly() && !spawnEnemies) ||
            (mobCategory->isPersistent() && !spawnPersistent)) {
            continue;
        }

        
        if (mobCategory->isFriendly()) {
            if (level->dimension->id != 0) {
                continue;
            }
        }

        
        
        
        int categoryCount = level->countInstanceOf(
            mobCategory->getEnumBaseClass(), mobCategory->isSingleType());
        if (categoryCount >= mobCategory->getMaxInstancesPerLevel()) {
            continue;
        }

        auto itEndCTP = chunksToPoll.end();
        for (auto it = chunksToPoll.begin(); it != itEndCTP; it++) {
            if (it->second) {
                
                
                continue;
            }
            ChunkPos* cp = (ChunkPos*)(&it->first);

            
            
            
            if (!level->hasChunk(cp->x, cp->z)) continue;

            TilePos start = getRandomPosWithin(level, cp->x, cp->z);
            int xStart = start.x;
            int yStart = start.y;
            int zStart = start.z;

            if (level->isSolidBlockingTile(xStart, yStart, zStart)) continue;
            if (level->getMaterial(xStart, yStart, zStart) !=
                mobCategory->getSpawnPositionMaterial())
                continue;
            int clusterSize = 0;

            for (int dd = 0; dd < 3; dd++) {
                int x = xStart;
                int y = yStart;
                int z = zStart;
                int ss = 6;

                Biome::MobSpawnerData* currentMobType = nullptr;
                MobGroupData* groupData = nullptr;

                for (int ll = 0; ll < 4; ll++) {
                    x +=
                        level->random->nextInt(ss) - level->random->nextInt(ss);
                    y += level->random->nextInt(1) - level->random->nextInt(1);
                    z +=
                        level->random->nextInt(ss) - level->random->nextInt(ss);
                    

                    
                    
                    
                    
                    if (!level->hasChunkAt(x, y, z)) continue;

                    if (isSpawnPositionOk(mobCategory, level, x, y, z)) {
                        float xx = x + 0.5f;
                        float yy = (float)y;
                        float zz = z + 0.5f;
                        if (level->getNearestPlayer(
                                xx, yy, zz, MIN_SPAWN_DISTANCE) != nullptr) {
                            continue;
                        } else {
                            float xd = xx - spawnPos->x;
                            float yd = yy - spawnPos->y;
                            float zd = zz - spawnPos->z;
                            float sd = xd * xd + yd * yd + zd * zd;
                            if (sd < MIN_SPAWN_DISTANCE * MIN_SPAWN_DISTANCE) {
                                continue;
                            }
                        }

                        if (currentMobType == nullptr) {
                            currentMobType = level->getRandomMobSpawnAt(
                                mobCategory, x, y, z);
                            if (currentMobType == nullptr) {
                                break;
                            }
                        }

                        std::shared_ptr<Mob> mob;
                        
                        
                        
                        
                        
                        mob = std::dynamic_pointer_cast<Mob>(
                            EntityIO::newByEnumType(currentMobType->mobClass,
                                                    level));
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        

                        
                        
                        
                        
                        
                        eINSTANCEOF mobType = mob->GetType();

                        if ((mobType & eTYPE_ANIMALS_SPAWN_LIMIT_CHECK) ||
                            (mobType & eTYPE_MONSTER)) {
                            
                            
                            
                            
                            
                            
                            
                            if (mobType == eTYPE_GHAST) {
                                if (level->countInstanceOf(mobType, true) >= 4)
                                    continue;
                            } else if (mobType == eTYPE_ENDERMAN &&
                                       level->dimension->id == 1) {
                                
                                
                                
                                int maxEndermen =
                                    mobCategory->getMaxInstancesPerLevel();

                                if (level->difficulty == Difficulty::NORMAL) {
                                    maxEndermen -=
                                        mobCategory->getMaxInstancesPerLevel() /
                                        4;
                                } else if (level->difficulty <=
                                           Difficulty::EASY) {
                                    maxEndermen -=
                                        mobCategory->getMaxInstancesPerLevel() /
                                        2;
                                }

                                if (level->countInstanceOf(mobType, true) >=
                                    maxEndermen)
                                    continue;
                            } else if (level->countInstanceOf(mobType, true) >=
                                       (mobCategory->getMaxInstancesPerLevel() /
                                        2))
                                continue;
                        }

                        mob->moveTo(xx, yy, zz,
                                    level->random->nextFloat() * 360, 0);

                        if (mob->canSpawn()) {
                            
                            
                            
                            
                            mob->checkDespawn();
                            if (!mob->removed) {
                                clusterSize++;
                                categoryCount++;
                                mob->setDespawnProtected();  
                                                             
                                                             
                                                             
                                                             
                                level->addEntity(mob);
                                groupData = mob->finalizeMobSpawn(groupData);
                                
                                
                                
                                
                                
                                
                                
                                if (categoryCount >=
                                    mobCategory->getMaxInstancesPerLevel())
                                    goto categoryLoop;
                                if (clusterSize >=
                                    mob->getMaxSpawnClusterSize())
                                    goto chunkLoop;
                            }
                        }
                        count += clusterSize;
                    }
                }
            }
        chunkLoop:
            continue;
        }
    categoryLoop:
        continue;
    }
    delete spawnPos;

    return count;
}

bool MobSpawner::isSpawnPositionOk(MobCategory* category, Level* level, int x,
                                   int y, int z) {
    
    
    
    if (!level->hasChunkAt(x, y, z)) return false;

    if (category->getSpawnPositionMaterial() == Material::water) {
        
        int yo = 0;
        int liquidCount = 0;

        while ((y - yo) >= 0 && (yo < 5)) {
            if (level->getMaterial(x, y - yo, z)->isLiquid()) liquidCount++;
            yo++;
        }

        
        
        bool inEnoughWater = false;
        if (liquidCount == 5) {
            if (level->getMaterial(x + 5, y, z)->isLiquid() &&
                level->getMaterial(x - 5, y, z)->isLiquid() &&
                level->getMaterial(x, y, z + 5)->isLiquid() &&
                level->getMaterial(x, y, z - 5)->isLiquid()) {
                inEnoughWater = true;
            }
        }

        return inEnoughWater && !level->isSolidBlockingTile(x, y + 1, z);
    } else {
        if (!level->isTopSolidBlocking(x, y - 1, z)) return false;
        int tt = level->getTile(x, y - 1, z);
        return tt != Tile::unbreakable_Id &&
               !level->isSolidBlockingTile(x, y, z) &&
               !level->getMaterial(x, y, z)->isLiquid() &&
               !level->isSolidBlockingTile(x, y + 1, z);
    }
}

void MobSpawner::postProcessSpawnMobs(Level* level, Biome* biome, int xo,
                                      int zo, int cellWidth, int cellHeight,
                                      Random* random) {
    
}
