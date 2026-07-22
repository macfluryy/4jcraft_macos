#pragma once

#include <memory>

#include "AbstractContainerMenu.h"

class CraftingContainer;
class Container;
class Inventory;
class Level;

class FireworksMenu : public AbstractContainerMenu {
    
public:
    static const int RESULT_SLOT = 0;
    static const int CRAFT_SLOT_START = 1;
    static const int CRAFT_SLOT_END = CRAFT_SLOT_START + 9;
    static const int INV_SLOT_START = CRAFT_SLOT_END;
    static const int INV_SLOT_END = INV_SLOT_START + (9 * 3);
    static const int USE_ROW_SLOT_START = INV_SLOT_END;
    static const int USE_ROW_SLOT_END = USE_ROW_SLOT_START + 9;

public:
    std::shared_ptr<CraftingContainer> craftSlots;
    std::shared_ptr<Container> resultSlots;

private:
    Level* level;
    int x, y, z;

    bool m_canMakeFireworks;
    bool m_canMakeCharge;
    bool m_canMakeFade;

public:
    FireworksMenu(std::shared_ptr<Inventory> inventory, Level* level, int xt,
                  int yt, int zt);

    virtual void
    slotsChanged();  
                     
    virtual void removed(std::shared_ptr<Player> player);
    virtual bool stillValid(std::shared_ptr<Player> player);
    virtual std::shared_ptr<ItemInstance> quickMoveStack(
        std::shared_ptr<Player> player, int slotIndex);
    virtual bool canTakeItemForPickAll(std::shared_ptr<ItemInstance> carried,
                                       Slot* target);

    
    virtual bool isValidIngredient(std::shared_ptr<ItemInstance> item,
                                   int slotId);
};