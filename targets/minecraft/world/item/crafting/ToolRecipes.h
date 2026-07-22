



#pragma once

#include <format>
#include <string>
#include <vector>

#define MAX_TOOL_RECIPES 5

class Recipes;
class Object;

class ToolRecipies {
public:
    
    void _init();
    ToolRecipies() { _init(); }

private:
    static std::wstring shapes[][4];
    std::vector<Object*>* map;

public:
    void addRecipes(Recipes* r);
};
