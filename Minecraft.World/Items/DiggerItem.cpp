#include "../Build/stdafx.h"
#include "../Headers/net.minecraft.world.entity.h"
#include "../Headers/net.minecraft.world.item.h"
#include "../Headers/net.minecraft.world.level.tile.h"
#include "DiggerItem.h"

DiggerItem::DiggerItem(int id, int attackDamage, const Tier *tier, TileArray *tiles) : Item( id ), tier( tier )
{
	//this->tier = tier;
	this->tiles = tiles;
	maxStackSize = 1;
	setMaxDamage(tier->getUses());
	this->speed = tier->getSpeed();
	this->attackDamage = attackDamage + tier->getAttackDamageBonus();
}

float DiggerItem::getDestroySpeed(std::shared_ptr<ItemInstance> itemInstance, Tile *tile)
{
	for (unsigned int i = 0; i < tiles->length; i++)
		if ( (*tiles)[i] == tile) return speed;
	return 1;
}

bool DiggerItem::hurtEnemy(std::shared_ptr<ItemInstance> itemInstance, std::shared_ptr<Mob> mob, std::shared_ptr<Mob> attacker)
{
	itemInstance->hurt(2, attacker);
	return true;
}

bool DiggerItem::mineBlock(std::shared_ptr<ItemInstance> itemInstance, Level *level, int tile, int x, int y, int z, std::shared_ptr<Mob> owner)
{
	// Don't damage tools if the tile can be destroyed in one hit.
	if (Tile::tiles[tile]->getDestroySpeed(level, x, y, z) != 0.0) itemInstance->hurt(1, owner);
	return true;
}

int DiggerItem::getAttackDamage(std::shared_ptr<Entity> entity)
{
	return attackDamage;
}

bool DiggerItem::isHandEquipped()
{
	return true;
}

int DiggerItem::getEnchantmentValue()
{
	return tier->getEnchantmentValue();
}

const Item::Tier *DiggerItem::getTier()
{
	return tier;
}

bool DiggerItem::isValidRepairItem(std::shared_ptr<ItemInstance> source, std::shared_ptr<ItemInstance> repairItem)
{
	if (tier->getTierItemId() == repairItem->id)
	{
		return true;
	}
	return Item::isValidRepairItem(source, repairItem);
}