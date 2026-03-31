#include "DirectionalTile.h"
#include "Minecraft.World/net/minecraft/world/level/tile/Tile.h"

class Material;

DirectionalTile::DirectionalTile(int id, Material* material, bool isSolidRender)
    : Tile(id, material, isSolidRender) {}

int DirectionalTile::getDirection(int data) { return data & DIRECTION_MASK; }