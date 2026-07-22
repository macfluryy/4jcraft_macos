#include "HellBiome.h"

#include <vector>

#include "java/Class.h"
#include "minecraft/world/level/biome/Biome.h"

HellBiome::HellBiome(int id) : Biome(id) {
    enemies.clear();
    friendlies.clear();
    friendlies_chicken.clear();  
    friendlies_wolf.clear();     
    waterFriendlies.clear();
    ambientFriendlies.clear();

    enemies.push_back(new MobSpawnerData(eTYPE_GHAST, 50, 4, 4));
    enemies.push_back(new MobSpawnerData(eTYPE_PIGZOMBIE, 100, 4, 4));
    enemies.push_back(new MobSpawnerData(eTYPE_LAVASLIME, 1, 4, 4));
}