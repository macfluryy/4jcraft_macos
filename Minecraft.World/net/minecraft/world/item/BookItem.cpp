#include "../../../../Header Files/stdafx.h"

#include "BookItem.h"

BookItem::BookItem(int id) : Item(id) {}

bool BookItem::isEnchantable(std::shared_ptr<ItemInstance> itemInstance) {
    return itemInstance->count == 1;
}

int BookItem::getEnchantmentValue() { return 1; }