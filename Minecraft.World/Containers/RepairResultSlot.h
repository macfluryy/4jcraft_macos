#pragma once

#include "Slot.h"

class RepairMenu;

class RepairResultSlot : public Slot
{
private:
	RepairMenu *m_menu;
	int xt, yt, zt;

public:
	RepairResultSlot(RepairMenu *menu, int xt, int yt, int zt, std::shared_ptr<Container> container, int slot, int x, int y);

	bool mayPlace(std::shared_ptr<ItemInstance> item);
	bool mayPickup(std::shared_ptr<Player> player);
	void onTake(std::shared_ptr<Player> player, std::shared_ptr<ItemInstance> carried);
	virtual bool mayCombine(std::shared_ptr<ItemInstance> item); // 4J Added
};