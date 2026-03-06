#pragma once

#include "../Containers/SimpleContainer.h"

class EnderChestTileEntity;

class PlayerEnderChestContainer : public SimpleContainer
{
private:
	std::shared_ptr<EnderChestTileEntity> activeChest;

public:
	PlayerEnderChestContainer();

	void setActiveChest(std::shared_ptr<EnderChestTileEntity> activeChest);
	void setItemsByTag(ListTag<CompoundTag> *enderItemsList);
	ListTag<CompoundTag> *createTag();
	bool stillValid(std::shared_ptr<Player> player);
	void startOpen();
	void stopOpen();
};