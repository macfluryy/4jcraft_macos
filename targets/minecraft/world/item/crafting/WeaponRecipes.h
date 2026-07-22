




#pragma once

#include <format>
#include <string>
#include <vector>

class Object;
class Recipes;

#define MAX_WEAPON_RECIPES 2
class WeaponRecipies {
public:
    
    void _init();
    WeaponRecipies() { _init(); }

private:
    static std::wstring shapes[][4];
    std::vector<Object*>* map;

public:
    void addRecipes(Recipes* r);
};
