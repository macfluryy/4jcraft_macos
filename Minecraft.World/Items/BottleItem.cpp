#include "../Build/stdafx.h"
#include "../Headers/net.minecraft.world.phys.h"
#include "../Headers/net.minecraft.world.entity.player.h"
#include "../Headers/net.minecraft.world.level.h"
#include "../Headers/net.minecraft.world.level.material.h"
#include "../Headers/net.minecraft.world.item.h"
#include "BottleItem.h"

BottleItem::BottleItem(int id) : Item(id)
{
}

Icon *BottleItem::getIcon(int auxValue)
{
	return Item::potion->getIcon(0);
}

std::shared_ptr<ItemInstance> BottleItem::use(std::shared_ptr<ItemInstance> itemInstance, Level *level, std::shared_ptr<Player> player)
{
	HitResult *hr = getPlayerPOVHitResult(level, player, true);
	if (hr == NULL) return itemInstance;

	if (hr->type == HitResult::TILE)
	{
		int xt = hr->x;
		int yt = hr->y;
		int zt = hr->z;
		delete hr;

		if (!level->mayInteract(player, xt, yt, zt, 0))
		{
			return itemInstance;
		}
		if (!player->mayBuild(xt, yt, zt))
		{
			return itemInstance;
		}
		if (level->getMaterial(xt, yt, zt) == Material::water)
		{
			itemInstance->count--;
			if (itemInstance->count <= 0)
			{
				return std::shared_ptr<ItemInstance>( new ItemInstance( (Item *)Item::potion) );
			}
			else
			{
				if (!player->inventory->add(std::shared_ptr<ItemInstance>( new ItemInstance( (Item *)Item::potion) )))
				{
					player->drop( std::shared_ptr<ItemInstance>( new ItemInstance(Item::potion_Id, 1, 0) ));
				}
			}
		}
	}
	else
	{
		delete hr;
	}

	return itemInstance;
}

// 4J-PB - added to allow tooltips
bool BottleItem::TestUse(Level *level, std::shared_ptr<Player> player)
{
	HitResult *hr = getPlayerPOVHitResult(level, player, true);
	if (hr == NULL) return false;

	if (hr->type == HitResult::TILE)
	{
		int xt = hr->x;
		int yt = hr->y;
		int zt = hr->z;
		delete hr;

		if (!level->mayInteract(player, xt, yt, zt, 0))
		{
			return false;
		}
		if (!player->mayBuild(xt, yt, zt))
		{
			return false;
		}
		if (level->getMaterial(xt, yt, zt) == Material::water)
		{
			return true;
		}
	}
	else
	{
		delete hr;
	}

	return false;
}

void BottleItem::registerIcons(IconRegister *iconRegister)
{
	// We reuse another texture.
}