#pragma once

#include "SimpleContainer.h"

class AnvilMenu;

class RepairContainer : public SimpleContainer,
                        public std::enable_shared_from_this<RepairContainer> {
private:
    AnvilMenu* m_menu;

public:
    RepairContainer(AnvilMenu* menu, int name, bool customName, int size);
    void setChanged();
    bool canPlaceItem(int slot, std::shared_ptr<ItemInstance> item);
};