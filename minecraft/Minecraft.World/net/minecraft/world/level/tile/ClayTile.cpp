#include "ClayTile.h"
#include "Minecraft.World/net/minecraft/world/item/Item.h"
#include "Minecraft.World/net/minecraft/world/level/material/Material.h"
#include "Minecraft.World/net/minecraft/world/level/tile/Tile.h"

ClayTile::ClayTile(int id) : Tile(id, Material::clay) {}

int ClayTile::getResource(int data, Random* random, int playerBonusLevel) {
    return Item::clay->id;
}

int ClayTile::getResourceCount(Random* random) { return 4; }
