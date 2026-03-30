#include "../../../../../Header Files/stdafx.h"
#include "RainforestBiome.h"
#include "../levelgen/feature/net.minecraft.world.level.levelgen.feature.h"

RainforestBiome::RainforestBiome(int id) : Biome(id) {}

Feature* RainforestBiome::getTreeFeature(Random* random) {
    if (random->nextInt(3) == 0) {
        return new BasicTree(false);
    }
    return new TreeFeature(false);
}