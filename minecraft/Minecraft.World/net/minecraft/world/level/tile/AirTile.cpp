#include "AirTile.h"
#include "Minecraft.World/net/minecraft/world/level/material/Material.h"
#include "Minecraft.World/net/minecraft/world/level/tile/Tile.h"

AirTile::AirTile(int id) : Tile(id, Material::air) {}
