#include "../Build/stdafx.h"
#include "../Headers/net.minecraft.world.level.h"

#include "SpongeTile.h"

const int RANGE = 2;


Sponge::Sponge(int id) : Tile(id, Material::sponge)
{
}