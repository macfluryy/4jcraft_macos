#include "NetherrackTile.h"
#include "Minecraft.World/net/minecraft/world/level/material/Material.h"
#include "Minecraft.World/net/minecraft/world/level/tile/Tile.h"

NetherrackTile::NetherrackTile(int id) : Tile(id, Material::stone) {}