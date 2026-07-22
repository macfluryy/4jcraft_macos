#pragma once

#include <memory>

#include "Behavior.h"

class ItemInstance;
class BlockSource;

class DispenseItemBehavior : public Behavior {
public:
    


    static DispenseItemBehavior* NOOP;

    virtual ~DispenseItemBehavior() = default;

    





    virtual std::shared_ptr<ItemInstance> dispense(
        BlockSource* source, std::shared_ptr<ItemInstance> dispensed) = 0;
};

class NoOpDispenseItemBehavior : public DispenseItemBehavior {
public:
    std::shared_ptr<ItemInstance> dispense(
        BlockSource* source, std::shared_ptr<ItemInstance> dispensed);
};
