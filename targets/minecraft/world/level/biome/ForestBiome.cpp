#include "ForestBiome.h"

#include <vector>

#include "java/Class.h"
#include "java/Random.h"
#include "minecraft/world/level/biome/Biome.h"
#include "minecraft/world/level/biome/BiomeDecorator.h"
#include "minecraft/world/level/levelgen/feature/BasicTreeFeature.h"
#include "minecraft/world/level/levelgen/feature/BirchFeature.h"
#include "minecraft/world/level/levelgen/feature/TreeFeature.h"

ForestBiome::ForestBiome(int id) : Biome(id) {
    friendlies_wolf.push_back(new MobSpawnerData(
        eTYPE_WOLF, 5, 4, 4));  
    decorator->treeCount = 10;
    decorator->grassCount = 2;
}

Feature* ForestBiome::getTreeFeature(Random* random) {
    if (random->nextInt(5) == 0) {
        return new BirchFeature(
            false);  
                     
                     
    }
    if (random->nextInt(10) == 0) {
        return new BasicTree(
            false);  
                     
                     
    }
    return new TreeFeature(
        false);  
                 
                 
}
