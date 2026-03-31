#include "LockedChestTile.h"
#include "Minecraft.World/net/minecraft/world/level/Level.h"
#include "Minecraft.World/net/minecraft/world/level/material/Material.h"
#include "Minecraft.World/net/minecraft/world/level/tile/Tile.h"

LockedChestTile::LockedChestTile(int id) : Tile(id, Material::wood) {}

bool LockedChestTile::mayPlace(Level* level, int x, int y, int z) {
    return true;
}

void LockedChestTile::tick(Level* level, int x, int y, int z, Random* random) {
    level->removeTile(x, y, z);
}

void LockedChestTile::registerIcons(IconRegister* iconRegister) {
    // None
}