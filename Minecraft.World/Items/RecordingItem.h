#pragma once


#include "Item.h"

class RecordingItem : public Item
{
public:
	const std::wstring recording;

public: // 4J Stu - Was protected in Java, but the can't access it where we need
	RecordingItem(int id, const std::wstring& recording);

	//@Override
	Icon *getIcon(int auxValue);
	virtual bool useOn(std::shared_ptr<ItemInstance> itemInstance, std::shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly=false);

	virtual void appendHoverText(std::shared_ptr<ItemInstance> itemInstance, std::shared_ptr<Player> player, std::vector<std::wstring> *lines, bool advanced, std::vector<std::wstring> &unformattedStrings);
	virtual const Rarity *getRarity(std::shared_ptr<ItemInstance> itemInstance);

	//@Override
	void registerIcons(IconRegister *iconRegister);
};