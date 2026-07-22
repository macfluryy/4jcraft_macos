




#pragma once

#include "platform/PlatformTypes.h"
#include "minecraft/world/inventory/CraftingContainer.h"

#define RECIPE_TYPE_2x2 0
#define RECIPE_TYPE_3x3 1

class Recipy {
public:
    enum _eGroupType {
        eGroupType_First = 0,
        eGroupType_Structure = 0,
        eGroupType_Tool,
        eGroupType_Food,
        eGroupType_Armour,
        eGroupType_Mechanism,
        eGroupType_Transport,
        eGroupType_Decoration,
        eGroupType_Max
    } eGroupType;  

    
    
    typedef struct {
        int iIngC;
        int iType;  
                    
        int* iIngIDA;
        int* iIngValA;
        int* iIngAuxValA;
        Recipy* pRecipy;
        bool bCanMake[XUSER_MAX_COUNT];
        unsigned int*
            uiGridA;  
        unsigned short usBitmaskMissingGridIngredients
            [XUSER_MAX_COUNT];  
                                
    } INGREDIENTS_REQUIRED;
    ~Recipy() {}
    virtual bool matches(std::shared_ptr<CraftingContainer> craftSlots,
                         Level* level) = 0;
    virtual std::shared_ptr<ItemInstance> assemble(
        std::shared_ptr<CraftingContainer> craftSlots) = 0;
    virtual int size() = 0;
    virtual const ItemInstance* getResultItem() = 0;
    virtual const int getGroup() = 0;

    
    virtual bool requiresRecipe(int iRecipe) = 0;
    virtual void collectRequirements(INGREDIENTS_REQUIRED* pIngReq) = 0;
};
