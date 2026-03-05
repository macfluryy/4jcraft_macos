#include "../../Build/stdafx.h"
#include "../../Headers/net.minecraft.world.level.tile.h"
#include "../../Headers/net.minecraft.world.item.h"
#include "PistonTileItem.h"

PistonTileItem::PistonTileItem(int id) : TileItem(id)
{
}

int PistonTileItem::getLevelDataForAuxValue(int auxValue)
{
	// return an undefined facing until the setPlacedBy method is called
	return PistonBaseTile::UNDEFINED_FACING;
}