#pragma once

#include "../Entities/Mob.h"

class Player;
class Level;

class MobSpawner {
private:
    static const int MIN_SPAWN_DISTANCE;

protected:
    static TilePos getRandomPosWithin(Level* level, int cx, int cz);

private:
#ifdef __PSVITA__
    // AP - See CustomMap.h for an explanation of this
    static CustomMap chunksToPoll;
#else
    static std::unordered_map<ChunkPos, bool, ChunkPosKeyHash, ChunkPosKeyEq>
        chunksToPoll;
#endif

public:
    static const int tick(ServerLevel* level, bool spawnEnemies,
                          bool spawnFriendlies, bool spawnPersistent);
    static bool isSpawnPositionOk(MobCategory* category, Level* level, int x,
                                  int y, int z);

public:
    static bool attackSleepingPlayers(
        Level* level, std::vector<std::shared_ptr<Player> >* players);

    static void postProcessSpawnMobs(Level* level, Biome* biome, int xo, int zo,
                                     int cellWidth, int cellHeight,
                                     Random* random);
};
