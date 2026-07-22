#pragma once




#include "minecraft/world/SimpleContainer.h"

class EnchantmentMenu;

class EnchantmentContainer : public SimpleContainer {
private:
    EnchantmentMenu* m_menu;

public:
    EnchantmentContainer(EnchantmentMenu* menu);
    virtual int getMaxStackSize();
    virtual void setChanged();
    virtual bool canPlaceItem(int slot, std::shared_ptr<ItemInstance> item);
};