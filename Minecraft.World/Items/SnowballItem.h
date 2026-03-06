#pragma once
using namespace std;

#include "Item.h"

class SnowballItem : public Item
{
public:
	SnowballItem(int id);

	virtual std::shared_ptr<ItemInstance> use(std::shared_ptr<ItemInstance> instance, Level *level, std::shared_ptr<Player> player);
};