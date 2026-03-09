#include "../Platform/stdafx.h"
#include "../Headers/net.minecraft.world.item.h"
#include "../Headers/net.minecraft.world.entity.h"
#include "../Headers/net.minecraft.world.entity.player.h"
#include "../Headers/net.minecraft.world.level.tile.h"
#include "WeaponItem.h"

WeaponItem::WeaponItem(int id, const Tier *tier) : Item(id), tier( tier )
{
    maxStackSize = 1;
    setMaxDamage(tier->getUses());

    damage = 4 + tier->getAttackDamageBonus();
}

float WeaponItem::getDestroySpeed(std::shared_ptr<ItemInstance> itemInstance, Tile *tile)
{
    if (tile->id == Tile::web_Id)
	{
        // swords can quickly cut web
        return 15;
    }
    return 1.5f;
}

bool WeaponItem::hurtEnemy(std::shared_ptr<ItemInstance> itemInstance, std::shared_ptr<Mob> mob, std::shared_ptr<Mob> attacker) 
{
	itemInstance->hurt(1, attacker);
    return true;
}

bool WeaponItem::mineBlock(std::shared_ptr<ItemInstance> itemInstance, Level *level, int tile, int x, int y, int z, std::shared_ptr<Mob> owner)
{
	// Don't damage weapons if the tile can be destroyed in one hit.
	if (Tile::tiles[tile]->getDestroySpeed(level, x, y, z) != 0.0) itemInstance->hurt(2, owner);
	return true;
}

int WeaponItem::getAttackDamage(std::shared_ptr<Entity> entity)
{
    return damage;
}

bool WeaponItem::isHandEquipped()
{
    return true;
}

UseAnim WeaponItem::getUseAnimation(std::shared_ptr<ItemInstance> itemInstance)
{
	return UseAnim_block;
}    

int WeaponItem::getUseDuration(std::shared_ptr<ItemInstance> itemInstance)
{
	return 20 * 60 * 60; // Block for a maximum of one hour!
}

std::shared_ptr<ItemInstance> WeaponItem::use(std::shared_ptr<ItemInstance> instance, Level *level, std::shared_ptr<Player> player)
{
	player->startUsingItem(instance, getUseDuration(instance));
	return instance;
}

bool WeaponItem::canDestroySpecial(Tile *tile)
{
    return tile->id == Tile::web_Id;
}

int WeaponItem::getEnchantmentValue()
{
	return tier->getEnchantmentValue();
}

const Item::Tier *WeaponItem::getTier()
{
	return tier;
}

bool WeaponItem::isValidRepairItem(std::shared_ptr<ItemInstance> source, std::shared_ptr<ItemInstance> repairItem)
{
	if (tier->getTierItemId() == repairItem->id)
	{
		return true;
	}
	return Item::isValidRepairItem(source, repairItem);
}