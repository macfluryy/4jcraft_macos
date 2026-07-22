#include "WeaponRecipes.h"

#include <vector>

#include "Recipes.h"
#include "java/Class.h"
#include "minecraft/world/item/Item.h"
#include "minecraft/world/item/ItemInstance.h"
#include "minecraft/world/level/tile/Tile.h"


std::wstring WeaponRecipies::shapes[][4] = {
    {L"X",        
     L"X",        
     L"#", L""},  
};

void WeaponRecipies::_init() {
    map = new std::vector<Object*>[MAX_WEAPON_RECIPES];

    ADD_OBJECT(map[0], Tile::wood);
    ADD_OBJECT(map[0], Tile::cobblestone);
    ADD_OBJECT(map[0], Item::ironIngot);
    ADD_OBJECT(map[0], Item::diamond);
    ADD_OBJECT(map[0], Item::goldIngot);

    ADD_OBJECT(map[1], Item::sword_wood);
    ADD_OBJECT(map[1], Item::sword_stone);
    ADD_OBJECT(map[1], Item::sword_iron);
    ADD_OBJECT(map[1], Item::sword_diamond);
    ADD_OBJECT(map[1], Item::sword_gold);
}

void WeaponRecipies::addRecipes(Recipes* r) {
    wchar_t wchTypes[7];
    wchTypes[6] = 0;

    for (unsigned int m = 0; m < map[0].size(); m++) {
        Object* pObjMaterial = map[0].at(m);

        for (int t = 0; t < MAX_WEAPON_RECIPES - 1; t++) {
            Item* target = map[t + 1].at(m)->item;

            wchTypes[0] = L'w';
            wchTypes[1] = L'c';
            wchTypes[2] = L'i';
            wchTypes[3] = L'c';
            wchTypes[5] = L'g';
            if (pObjMaterial->GetType() == eType_TILE) {
                wchTypes[4] = L't';
                r->addShapedRecipy(
                    new ItemInstance(target), wchTypes, shapes[t],

                    L'#', Item::stick, L'X', pObjMaterial->tile, L'T');
            } else {
                
                wchTypes[4] = L'i';
                r->addShapedRecipy(
                    new ItemInstance(target), wchTypes, shapes[t],

                    L'#', Item::stick, L'X', pObjMaterial->item, L'T');
            }
        }
    }

    





















}