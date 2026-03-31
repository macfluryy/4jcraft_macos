#include <memory>

#include "EmptyMapItem.h"
#include "Minecraft.World/net/minecraft/world/entity/player/Inventory.h"
#include "Minecraft.World/net/minecraft/world/entity/player/Player.h"
#include "Minecraft.World/net/minecraft/world/item/ComplexItem.h"
#include "Minecraft.World/net/minecraft/world/item/Item.h"
#include "Minecraft.World/net/minecraft/world/item/ItemInstance.h"
#include "Minecraft.World/net/minecraft/world/item/MapItem.h"

EmptyMapItem::EmptyMapItem(int id) : ComplexItem(id) {}

std::shared_ptr<ItemInstance> EmptyMapItem::use(
    std::shared_ptr<ItemInstance> itemInstance, Level* level,
    std::shared_ptr<Player> player) {
    // shared_ptr<ItemInstance> map = shared_ptr<ItemInstance>( new
    // ItemInstance(Item::map, 1, level->getFreeAuxValueFor(L"map")) );

    // String id = "map_" + map.getAuxValue();
    // MapItemSavedData data = new MapItemSavedData(id);
    // level.setSavedData(id, data);

    // data.scale = 0;
    // int scale = MapItemSavedData.MAP_SIZE * 2 * (1 << data.scale);
    // data.x = (int) (Math.round(player.x / scale) * scale);
    // data.z = (int) (Math.round(player.z / scale) * scale);
    // data.dimension = (byte) level.dimension.id;

    // data.setDirty();

    std::shared_ptr<ItemInstance> map =
        std::shared_ptr<ItemInstance>(new ItemInstance(Item::map, 1, -1));
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