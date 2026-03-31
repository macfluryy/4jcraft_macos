#pragma once
#include "Feature.h"
#include "Minecraft.World/net/minecraft/world/level/material/Material.h"

class ReedsFeature : public Feature {
public:
    virtual bool place(Level* level, Random* random, int x, int y, int z);
};