#include "GravelTile.h"
#include "java/Random.h"
#include "Minecraft.World/net/minecraft/world/item/Item.h"
#include "Minecraft.World/net/minecraft/world/level/tile/HeavyTile.h"

GravelTile::GravelTile(int type) : HeavyTile(type) {}

int GravelTile::getResource(int data, Random* random, int playerBonusLevel) {
    if (playerBonusLevel > 3) playerBonusLevel = 3;
    if (random->nextInt(10 - playerBonusLevel * 3) == 0) return Item::flint->id;
    return id;
}