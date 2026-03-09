

#include "../Platform/stdafx.h"
#include "Item.h"
#include "../Headers/net.minecraft.world.entity.player.h"
#include "../Headers/net.minecraft.world.level.h"
#include "../Headers/net.minecraft.world.level.tile.h"
#include "ItemInstance.h"
#include "SeedItem.h"

SeedItem::SeedItem(int id, int resultId, int targetLand) : Item(id)
{
    this->resultId = resultId;
	this->targetLand = targetLand;
}

bool SeedItem::useOn(std::shared_ptr<ItemInstance> instance, std::shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	// 4J-PB - Adding a test only version to allow tooltips to be displayed
    if (face != 1) return false;

	if (!player->mayBuild(x, y, z) || !player->mayBuild(x, y + 1, z)) return false;

    int targetType = level->getTile(x, y, z);

    if (targetType == targetLand && level->isEmptyTile(x, y + 1, z)) 
	{
		if(!bTestUseOnOnly)
		{
			level->setTile(x, y + 1, z, resultId);
			instance->count--;
		}
        return true;
    }
    return false;
}