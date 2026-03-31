#include "PistonTileItem.h"
#include "Minecraft.World/net/minecraft/world/item/TileItem.h"
#include "Minecraft.World/net/minecraft/world/level/tile/piston/PistonBaseTile.h"

PistonTileItem::PistonTileItem(int id) : TileItem(id) {}

int PistonTileItem::getLevelDataForAuxValue(int auxValue) {
    // return an undefined facing until the setPlacedBy method is called
    return PistonBaseTile::UNDEFINED_FACING;
}