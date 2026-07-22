#pragma once

#include <string>

#include "minecraft/world/entity/ai/attributes/Attribute.h"















class HtmlString;

enum eMODIFIER_ID {
    eModifierId_ANONYMOUS = 0,

    eModifierId_ITEM_BASEDAMAGE,

    eModifierId_MOB_FLEEING,
    eModifierId_MOB_SPRINTING,

    eModifierId_MOB_ENDERMAN_ATTACKSPEED,
    eModifierId_MOB_PIG_ATTACKSPEED,
    eModifierId_MOB_WITCH_DRINKSPEED,
    eModifierId_MOB_ZOMBIE_BABYSPEED,

    eModifierId_POTION_DAMAGEBOOST,
    eModifierId_POTION_HEALTHBOOST,
    eModifierId_POTION_MOVESPEED,
    eModifierId_POTION_MOVESLOWDOWN,
    eModifierId_POTION_WEAKNESS,

    eModifierId_COUNT,
};

class AttributeModifier {
public:
    static const int OPERATION_ADDITION = 0;
    static const int OPERATION_MULTIPLY_BASE = 1;
    static const int OPERATION_MULTIPLY_TOTAL = 2;
    static const int TOTAL_OPERATIONS = 3;

private:
    double amount;
    int operation;
    std::wstring name;
    eMODIFIER_ID id;
    bool serialize;

    void _init(eMODIFIER_ID id, const std::wstring name, double amount,
               int operation);

public:
    AttributeModifier(double amount, int operation);
    AttributeModifier(eMODIFIER_ID id, double amount, int operation);

    eMODIFIER_ID getId();
    std::wstring getName();
    int getOperation();
    double getAmount();
    bool isSerializable();
    AttributeModifier* setSerialize(bool serialize);
    bool equals(AttributeModifier* modifier);
    std::wstring toString();
    HtmlString getHoverText(
        eATTRIBUTE_ID attribute);  
                                   
};