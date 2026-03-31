#pragma once
#include "Minecraft.World/net/minecraft/world/level/levelgen/feature/Feature.h"

class WaterlilyFeature : public Feature {
    virtual bool place(Level* level, Random* random, int x, int y, int z);
};