
#include <vector>

#include "java/Class.h"
#include "Minecraft.World/net/minecraft/world/level/biome/Biome.h"
#include "Minecraft.World/net/minecraft/world/level/biome/BiomeDecorator.h"
#include "Minecraft.World/net/minecraft/world/level/biome/PlainsBiome.h"

PlainsBiome::PlainsBiome(int id) : Biome(id) {
    friendlies.push_back(new MobSpawnerData(eTYPE_HORSE, 5, 2, 6));

    decorator->treeCount = -999;
    decorator->flowerCount = 4;
    decorator->grassCount = 10;
}