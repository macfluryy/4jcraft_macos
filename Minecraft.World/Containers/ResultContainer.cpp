#include "../Build/stdafx.h"
#include "../Headers/net.minecraft.world.entity.player.h"
#include "ResultContainer.h"

ResultContainer::ResultContainer() : Container()
{
	items = new ItemInstanceArray(1);
}

unsigned int ResultContainer::getContainerSize()
{
	return 1;
}

std::shared_ptr<ItemInstance> ResultContainer::getItem(unsigned int slot)
{
	return (*items)[0];
}

int ResultContainer::getName()
{
	return 0;
}

std::shared_ptr<ItemInstance> ResultContainer::removeItem(unsigned int slot, int count)
{
	if ((*items)[0] != NULL)
	{
		std::shared_ptr<ItemInstance> item = (*items)[0];
		(*items)[0] = nullptr;
		return item;
	}
	return nullptr;
}

std::shared_ptr<ItemInstance> ResultContainer::removeItemNoUpdate(int slot)
{
	if ((*items)[0] != NULL)
	{
		std::shared_ptr<ItemInstance> item = (*items)[0];
		(*items)[0] = nullptr;
		return item;
	}
	return nullptr;
}

void ResultContainer::setItem(unsigned int slot, std::shared_ptr<ItemInstance> item)
{
	(*items)[0] = item;
}

int ResultContainer::getMaxStackSize()
{
	return Container::LARGE_MAX_STACK_SIZE;
}

void ResultContainer::setChanged()
{
}

bool ResultContainer::stillValid(std::shared_ptr<Player> player)
{
	return true;
}