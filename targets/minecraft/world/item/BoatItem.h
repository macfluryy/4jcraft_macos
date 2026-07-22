#pragma once

#include "Item.h"

class Player;
class Level;

class BoatItem : public Item {
public:
    BoatItem(int id);

    virtual bool TestUse(std::shared_ptr<ItemInstance> itemInstance,
                         Level* level, std::shared_ptr<Player> player);
    virtual std::shared_ptr<ItemInstance> use(
        std::shared_ptr<ItemInstance> itemInstance, Level* level,
        std::shared_ptr<Player> player);

    






};