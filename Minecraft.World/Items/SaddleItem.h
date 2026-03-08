#pragma once


#include "Item.h"

class SaddleItem : public Item
{
public:
	SaddleItem(int id);

	virtual bool interactEnemy(std::shared_ptr<ItemInstance> itemInstance, std::shared_ptr<Mob> mob);
	virtual bool hurtEnemy(std::shared_ptr<ItemInstance> itemInstance, std::shared_ptr<Mob> mob, std::shared_ptr<Mob> attacker);
};