#include "PistonTileItem.h"

#include "minecraft/world/item/TileItem.h"
#include "minecraft/world/level/tile/piston/PistonBaseTile.h"

PistonTileItem::PistonTileItem(int id) : TileItem(id) {}

int PistonTileItem::getLevelDataForAuxValue(int auxValue) {
    
    return PistonBaseTile::UNDEFINED_FACING;
}