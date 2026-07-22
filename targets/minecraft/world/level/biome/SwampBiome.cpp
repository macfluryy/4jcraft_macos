#include "minecraft/world/level/biome/SwampBiome.h"

#include <vector>

#include "java/Class.h"
#include "minecraft/world/level/biome/Biome.h"
#include "minecraft/world/level/biome/BiomeDecorator.h"
#include "minecraft/world/level/levelgen/feature/SwampTreeFeature.h"

SwampBiome::SwampBiome(int id) : Biome(id) {
    decorator->treeCount = 2;
    decorator->flowerCount = -999;
    decorator->deadBushCount = 1;
    decorator->mushroomCount = 8;
    decorator->reedsCount = 10;
    decorator->clayCount = 1;
    decorator->waterlilyCount = 4;

    

    enemies.push_back(new MobSpawnerData(eTYPE_SLIME, 1, 1, 1));
}

Feature* SwampBiome::getTreeFeature(Random* random) {
    return new SwampTreeFeature();  
                                    
                                    
                                    
}
















