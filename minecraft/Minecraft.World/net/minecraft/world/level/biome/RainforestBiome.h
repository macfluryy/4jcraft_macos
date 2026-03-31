#pragma once
#include "Biome.h"
#include "Minecraft.World/net/minecraft/world/level/biome/Biome.h"

class RainforestBiome : public Biome {
public:
    RainforestBiome(int id);
    virtual Feature* getTreeFeature(Random* random);
};