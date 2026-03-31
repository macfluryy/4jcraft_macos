#include "StoneTile.h"
#include "Minecraft.World/net/minecraft/world/level/material/Material.h"
#include "Minecraft.World/net/minecraft/world/level/tile/Tile.h"

StoneTile::StoneTile(int id) : Tile(id, Material::stone) {}

int StoneTile::getResource(int data, Random* random, int playerBonusLevel) {
    return Tile::cobblestone_Id;
}