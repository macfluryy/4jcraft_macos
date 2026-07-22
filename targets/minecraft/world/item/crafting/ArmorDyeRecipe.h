#pragma once

#include <memory>

#include "Recipy.h"
#include "minecraft/world/item/ItemInstance.h"

class CraftingContainer;

class ArmorDyeRecipe : public Recipy {
public:
    bool matches(std::shared_ptr<CraftingContainer> craftSlots, Level* level);

    
    
    static std::shared_ptr<ItemInstance> assembleDyedArmor(
        std::shared_ptr<CraftingContainer> craftSlots);
    std::shared_ptr<ItemInstance> assemble(
        std::shared_ptr<CraftingContainer> craftSlots);

    int size();
    const ItemInstance* getResultItem();

    virtual const int getGroup();

    
    virtual bool requiresRecipe(int iRecipe);
    virtual void collectRequirements(INGREDIENTS_REQUIRED* pIngReq);
};