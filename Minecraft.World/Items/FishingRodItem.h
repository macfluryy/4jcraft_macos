#pragma once


#include "Item.h"

class Player;
class Level;

class FishingRodItem : public Item
{
public:
	static const std::wstring TEXTURE_EMPTY;

private:
	Icon *emptyIcon;

public:
	FishingRodItem(int id);

	virtual bool isHandEquipped();
	virtual bool isMirroredArt();
	virtual std::shared_ptr<ItemInstance> use(std::shared_ptr<ItemInstance> instance, Level *level, std::shared_ptr<Player> player);	

	//@Override
	void registerIcons(IconRegister *iconRegister);
	Icon *getEmptyIcon();
};
