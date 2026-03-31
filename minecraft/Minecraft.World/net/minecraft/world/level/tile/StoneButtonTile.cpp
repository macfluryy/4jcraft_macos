#include "StoneButtonTile.h"
#include "Minecraft.World/net/minecraft/Facing.h"
#include "Minecraft.World/net/minecraft/world/level/tile/ButtonTile.h"
#include "Minecraft.World/net/minecraft/world/level/tile/Tile.h"

StoneButtonTile::StoneButtonTile(int id) : ButtonTile(id, false) {}

Icon* StoneButtonTile::getTexture(int face, int data) {
    return Tile::stone->getTexture(Facing::UP);
}