#include "../Platform/stdafx.h"
#include "../Headers/net.minecraft.world.entity.player.h"
#include "ItemInstance.h"
#include "BowlFoodItem.h"

BowlFoodItem::BowlFoodItem(int id, int nutrition) : FoodItem( id, nutrition, false )
{
	setMaxStackSize(1);
}

std::shared_ptr<ItemInstance> BowlFoodItem::useTimeDepleted(std::shared_ptr<ItemInstance> instance, Level *level, std::shared_ptr<Player> player)
{
	FoodItem::useTimeDepleted(instance, level, player);

	return std::shared_ptr<ItemInstance>(new ItemInstance(Item::bowl));
}