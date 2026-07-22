#include "MushroomIslandBiome.h"

#include <stdint.h>

#include <vector>

#include "BiomeDecorator.h"
#include "java/Class.h"
#include "minecraft/world/level/biome/Biome.h"
#include "minecraft/world/level/tile/Tile.h"

MushroomIslandBiome::MushroomIslandBiome(int id) : Biome(id) {
    decorator->treeCount = -100;
    decorator->flowerCount = -100;
    decorator->grassCount = -100;

    decorator->mushroomCount = 1;
    decorator->hugeMushrooms = 1;

    topMaterial = (uint8_t)Tile::mycel_Id;

    enemies.clear();
    friendlies.clear();
    friendlies_chicken.clear();  
    friendlies_wolf.clear();     
    waterFriendlies.clear();

    friendlies_mushroomcow.push_back(new MobSpawnerData(
        eTYPE_MUSHROOMCOW, 8, 4, 8));  
}