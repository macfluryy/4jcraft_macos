#pragma once
using namespace std;

#include "Container.h"

class Player;

class CompoundContainer : public Container
{
private:
	int name;
	std::shared_ptr<Container> c1, c2;

public:
	CompoundContainer(int name, std::shared_ptr<Container> c1, std::shared_ptr<Container> c2);

	unsigned int getContainerSize();

	int getName();

	std::shared_ptr<ItemInstance> getItem(unsigned int slot);

	std::shared_ptr<ItemInstance> removeItem(unsigned int slot, int i);
	std::shared_ptr<ItemInstance> removeItemNoUpdate(int slot);

	void setItem(unsigned int slot, std::shared_ptr<ItemInstance> item);

	int getMaxStackSize();

	void setChanged();

	bool stillValid(std::shared_ptr<Player> player);

	virtual void startOpen();
	virtual void stopOpen();
};