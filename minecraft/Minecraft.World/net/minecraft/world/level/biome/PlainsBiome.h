#pragma once

#include "Biome.h"
#include "Minecraft.World/net/minecraft/world/level/biome/Biome.h"

class PlainsBiome : public Biome {
    friend class Biome;

protected:
    PlainsBiome(int id);
};