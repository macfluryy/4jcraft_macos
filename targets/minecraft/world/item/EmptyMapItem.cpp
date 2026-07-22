#include "EmptyMapItem.h"

#include <memory>

#include "minecraft/world/entity/player/Inventory.h"
#include "minecraft/world/entity/player/Player.h"
#include "minecraft/world/item/ComplexItem.h"
#include "minecraft/world/item/Item.h"
#include "minecraft/world/item/ItemInstance.h"
#include "minecraft/world/item/MapItem.h"

EmptyMapItem::EmptyMapItem(int id) : ComplexItem(id) {}

std::shared_ptr<ItemInstance> EmptyMapItem::use(
    std::shared_ptr<ItemInstance> itemInstance, Level* level,
    std::shared_ptr<Player> player) {
    
    

    
    
    

    
    
    
    
    

    

    std::shared_ptr<ItemInstance> map =
        std::make_shared<ItemInstance>(Item::map, 1, -1);
    Item::map->onCraftedBy(map, level, player);

    itemInstance->count--;
    if (itemInstance->count <= 0) {
        return map;
    } else {
        if (!player->inventory->add(map->copy())) {
            player->drop(map);
        }
    }

    return itemInstance;
}