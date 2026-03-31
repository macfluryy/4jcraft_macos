#include <memory>

#include "SimpleFoiledItem.h"
#include "Minecraft.World/net/minecraft/world/item/Item.h"

SimpleFoiledItem::SimpleFoiledItem(int id) : Item(id) {}

bool SimpleFoiledItem::isFoil(std::shared_ptr<ItemInstance> itemInstance) {
    return true;
}
