#pragma once

#include "../Entities/Mob.h"

#ifdef __PSVITA__
#include "../../Minecraft.Client/Platform/PSVita/PSVitaExtras/CustomMap.h"
#endif

class Player;
class Level;

class MobSpawner
{
private:
	static const int MIN_SPAWN_DISTANCE;

protected:
	static TilePos getRandomPosWithin(Level *level, int cx, int cz);

private:
#ifdef __PSVITA__
	// AP - See CustomMap.h for an explanation of this
	static CustomMap chunksToPoll;
#else
	static std::unordered_map<ChunkPos,bool,ChunkPosKeyHash,ChunkPosKeyEq> chunksToPoll;
#endif

public:
	static const int tick(ServerLevel *level, bool spawnEnemies, bool spawnFriendlies);
	static bool isSpawnPositionOk(MobCategory *category, Level *level, int x, int y, int z);

private:
	static void finalizeMobSettings(std::shared_ptr<Mob> mob, Level *level, float xx, float yy, float zz);

protected:
	// 4J Stu TODO This was an array of Class type. I haven't made a base Class type yet, but don't need to
	// as this can be an array of Mob type?
	static const int bedEnemyCount = 3;
	static eINSTANCEOF bedEnemies[bedEnemyCount];


public:
	static bool attackSleepingPlayers(Level *level, vector<std::shared_ptr<Player> > *players);

	static void postProcessSpawnMobs(Level *level, Biome *biome, int xo, int zo, int cellWidth, int cellHeight, Random *random);
};