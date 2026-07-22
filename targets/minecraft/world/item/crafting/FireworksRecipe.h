#pragma once

#include <memory>

#include "Recipy.h"

class CraftingContainer;
class ItemInstance;

class FireworksRecipe : public Recipy {
private:
    

    
    
    class ThreadStorage {
    public:
        std::shared_ptr<ItemInstance> resultItem;
        ThreadStorage();
    };
    static thread_local ThreadStorage* m_tlsStorage;
    static ThreadStorage* m_defaultThreadStorage;

    void setResultItem(std::shared_ptr<ItemInstance> item);

public:
    
    
    
    static void CreateNewThreadStorage();
    static void UseDefaultThreadStorage();
    static void ReleaseThreadStorage();

public:
    FireworksRecipe();

    bool matches(std::shared_ptr<CraftingContainer> craftSlots, Level* level);
    std::shared_ptr<ItemInstance> assemble(
        std::shared_ptr<CraftingContainer> craftSlots);
    int size();
    const ItemInstance* getResultItem();

    virtual const int getGroup() { return 0; }

    
    virtual bool requiresRecipe(int iRecipe) { return false; };
    virtual void collectRequirements(INGREDIENTS_REQUIRED* pIngReq) {};

    
    static void updatePossibleRecipes(
        std::shared_ptr<CraftingContainer> craftSlots, bool* firework,
        bool* charge, bool* fade);
    static bool isValidIngredient(std::shared_ptr<ItemInstance> item,
                                  bool firework, bool charge, bool fade);
};