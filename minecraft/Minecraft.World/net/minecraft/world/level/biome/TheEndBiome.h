#pragma once
#include "Biome.h"
#include "Minecraft.World/net/minecraft/world/level/biome/Biome.h"

class TheEndBiome : public Biome {
public:
    TheEndBiome(int id);

    // 4J Stu - Don't need override
    // virtual int getSkyColor(float temp);
};