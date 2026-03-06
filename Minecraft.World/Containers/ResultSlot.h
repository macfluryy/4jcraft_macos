#pragma once

#include "Slot.h"

class ResultSlot : public Slot
{
private:
	std::shared_ptr<Container> craftSlots;
	Player *player; // This can't be a std::shared_ptr, as we create a result slot in the inventorymenu in the Player ctor
	int removeCount;

public:
	ResultSlot(Player *player, std::shared_ptr<Container> craftSlots, std::shared_ptr<Container> container, int id, int x, int y);
	virtual ~ResultSlot() {}

	virtual bool mayPlace(std::shared_ptr<ItemInstance> item);
	virtual std::shared_ptr<ItemInstance> remove(int c);

protected:
	virtual void onQuickCraft(std::shared_ptr<ItemInstance> picked, int count);
	virtual void checkTakeAchievements(std::shared_ptr<ItemInstance> carried);

public:
	virtual void onTake(std::shared_ptr<Player> player, std::shared_ptr<ItemInstance> carried);
	virtual bool mayCombine(std::shared_ptr<ItemInstance> item); // 4J Added
};