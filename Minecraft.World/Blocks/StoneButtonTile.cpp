#include "../Platform/stdafx.h"
#include "../Headers/net.minecraft.h"
#include "StoneButtonTile.h"

StoneButtonTile::StoneButtonTile(int id) : ButtonTile(id, false) {}

Icon* StoneButtonTile::getTexture(int face, int data) {
    return Tile::stone->getTexture(Facing::UP);
}