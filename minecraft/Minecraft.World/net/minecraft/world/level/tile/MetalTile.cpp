#include "MetalTile.h"
#include "Minecraft.World/net/minecraft/world/level/material/Material.h"
#include "Minecraft.World/net/minecraft/world/level/tile/Tile.h"

MetalTile::MetalTile(int id) : Tile(id, Material::metal) {}