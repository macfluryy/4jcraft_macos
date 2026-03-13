#pragma once

#include "Item.h"

class ItemInstance;

class CoalItem : public Item {
public:
    static const int STONE_COAL = 0;
    static const int CHAR_COAL = 1;

    CoalItem(int id);

    virtual unsigned int getDescriptionId(
        std::shared_ptr<ItemInstance> instance);
};