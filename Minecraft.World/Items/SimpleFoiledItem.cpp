#include "../Platform/stdafx.h"

#include "SimpleFoiledItem.h"

SimpleFoiledItem::SimpleFoiledItem(int id) : Item(id) {}

bool SimpleFoiledItem::isFoil(std::shared_ptr<ItemInstance> itemInstance) {
    return true;
}
