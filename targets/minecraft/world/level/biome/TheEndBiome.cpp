#include "TheEndBiome.h"

#include <stdint.h>

#include <vector>

#include "TheEndBiomeDecorator.h"
#include "java/Class.h"
#include "minecraft/world/level/biome/Biome.h"
#include "minecraft/world/level/tile/Tile.h"

TheEndBiome::TheEndBiome(int id) : Biome(id) {
    enemies.clear();
    friendlies.clear();
    friendlies_chicken.clear();  
    friendlies_wolf.clear();     
    waterFriendlies.clear();
    ambientFriendlies.clear();

    enemies.push_back(new MobSpawnerData(eTYPE_ENDERMAN, 10, 4, 4));
    topMaterial = (uint8_t)Tile::dirt_Id;
    material = (uint8_t)Tile::dirt_Id;

    decorator = new TheEndBiomeDecorator(this);
}





