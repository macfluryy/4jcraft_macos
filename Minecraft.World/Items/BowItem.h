#pragma once

#include "Item.h"

class Player;
class Level;

class BowItem : public Item
{
public:
	static const wstring TEXTURE_PULL[];
	static const int MAX_DRAW_DURATION = 20 * 1;

private:
	static const int BOW_ICONS_COUNT = 3;
	Icon **icons;

public:
	BowItem(int id);

	virtual void releaseUsing(std::shared_ptr<ItemInstance> itemInstance, Level *level, std::shared_ptr<Player> player, int durationLeft);
	virtual std::shared_ptr<ItemInstance> useTimeDepleted(std::shared_ptr<ItemInstance> instance, Level *level, std::shared_ptr<Player> player);
	virtual int getUseDuration(std::shared_ptr<ItemInstance> itemInstance);
	virtual UseAnim getUseAnimation(std::shared_ptr<ItemInstance> itemInstance);
	virtual std::shared_ptr<ItemInstance> use(std::shared_ptr<ItemInstance> instance, Level *level, std::shared_ptr<Player> player);
	virtual int getEnchantmentValue();

	//@Override
	void registerIcons(IconRegister *iconRegister);
	Icon *getDrawnIcon(int amount);
};