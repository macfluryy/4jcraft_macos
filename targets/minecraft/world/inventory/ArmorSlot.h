#pragma once




#include <memory>

#include "Slot.h"

class Container;

class ArmorSlot : public Slot {
private:
    const int slotNum;

public:
    ArmorSlot(int slotNum, std::shared_ptr<Container> container, int id, int x,
              int y);
    virtual ~ArmorSlot() {}

    virtual int getMaxStackSize();
    virtual bool mayPlace(std::shared_ptr<ItemInstance> item);
    Icon* getNoItemIcon();
    
    
    
};