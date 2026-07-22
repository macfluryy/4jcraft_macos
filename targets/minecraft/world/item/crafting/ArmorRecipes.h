



#pragma once

#include <format>
#include <string>
#include <vector>

class Object;
class Recipes;

#define MAX_ARMOUR_RECIPES 5
class ArmorRecipes {
public:
    enum _eArmorType {
        eArmorType_None = 0,
        eArmorType_Helmet,
        eArmorType_Chestplate,
        eArmorType_Leggings,
        eArmorType_Boots,
    } eArmorType;

    
    void _init();
    ArmorRecipes() { _init(); }

private:
    
    static std::wstring shapes[][4];

private:
    std::vector<Object*>* map;

public:
    void addRecipes(Recipes* r);
    static _eArmorType GetArmorType(int iId);
};
