#pragma once




#include "Slot.h"

class Container;

class EnchantmentSlot : public Slot {
public:
    EnchantmentSlot(std::shared_ptr<Container> container, int id, int x, int y)
        : Slot(container, id, x, y) {}
    virtual bool mayPlace(std::shared_ptr<ItemInstance> item) { return true; }
    virtual bool mayCombine(std::shared_ptr<ItemInstance> item) {
        return false;
    }  
};