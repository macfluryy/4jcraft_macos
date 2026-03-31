#include <vector>

#include "TaigaBiome.h"
#include "java/Class.h"
#include "java/Random.h"
#include "Minecraft.World/net/minecraft/world/level/biome/BiomeDecorator.h"
#include "Minecraft.World/net/minecraft/world/level/levelgen/feature/PineFeature.h"
#include "Minecraft.World/net/minecraft/world/level/levelgen/feature/SpruceFeature.h"

TaigaBiome::TaigaBiome(int id) : Biome(id) {
    friendlies_wolf.push_back(new MobSpawnerData(
        eTYPE_WOLF, 8, 4, 4));  // 4J - moved to their own category

    decorator->treeCount = 10;
    decorator->grassCount = 1;
}

Feature* TaigaBiome::getTreeFeature(Random* random) {
    if (random->nextInt(3) == 0) {
        return new PineFeature();
    }
    return new SpruceFeature(false);
}