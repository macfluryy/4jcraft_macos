#pragma once
#include "Biome.h"
#include "Minecraft.World/net/minecraft/world/level/biome/Biome.h"

class ForestBiome : public Biome {
public:
    ForestBiome(int id);

    virtual Feature* getTreeFeature(Random* random);
};
