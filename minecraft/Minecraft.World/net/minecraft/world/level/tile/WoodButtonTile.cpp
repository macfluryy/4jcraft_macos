#include "WoodButtonTile.h"
#include "Minecraft.World/net/minecraft/Facing.h"
#include "Minecraft.World/net/minecraft/world/level/tile/ButtonTile.h"
#include "Minecraft.World/net/minecraft/world/level/tile/Tile.h"

WoodButtonTile::WoodButtonTile(int id) : ButtonTile(id, true) {}

Icon* WoodButtonTile::getTexture(int face, int data) {
    return Tile::wood->getTexture(Facing::UP);
}