#pragma once

#include "Item.h"

class Mob;

class DiggerItem : public Item
{
private:
	TileArray *tiles;
protected:
	float speed;
private:
	int attackDamage;

protected:
	const Tier *tier;

	DiggerItem(int id, int attackDamage, const Tier *tier, TileArray *tiles);

public:
	virtual float getDestroySpeed(std::shared_ptr<ItemInstance> itemInstance, Tile *tile);
	virtual bool hurtEnemy(std::shared_ptr<ItemInstance> itemInstance, std::shared_ptr<Mob> mob, std::shared_ptr<Mob> attacker);
	virtual bool mineBlock(std::shared_ptr<ItemInstance> itemInstance, Level *level, int tile, int x, int y, int z, std::shared_ptr<Mob> owner);
	virtual int getAttackDamage(std::shared_ptr<Entity> entity);
	virtual bool isHandEquipped();
	virtual int getEnchantmentValue();

	const Tier *getTier();
	bool isValidRepairItem(std::shared_ptr<ItemInstance> source, std::shared_ptr<ItemInstance> repairItem);
};