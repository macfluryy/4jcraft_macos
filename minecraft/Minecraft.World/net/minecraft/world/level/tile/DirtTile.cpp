#include "DirtTile.h"
#include "Minecraft.World/net/minecraft/world/level/material/Material.h"
#include "Minecraft.World/net/minecraft/world/level/tile/Tile.h"

DirtTile::DirtTile(int id) : Tile(id, Material::dirt) {}