#include "../Platform/stdafx.h"
#include "../Headers/net.minecraft.world.entity.h"
#include "NameTagItem.h"

NameTagItem::NameTagItem(int id) : Item(id) {}

bool NameTagItem::interactEnemy(std::shared_ptr<ItemInstance> itemInstance,
                                std::shared_ptr<Player> player,
                                std::shared_ptr<LivingEntity> target) {
    if (!itemInstance->hasCustomHoverName()) return false;

    if ((target != nullptr) && target->instanceof(eTYPE_MOB)) {
        std::shared_ptr<Mob> mob = std::dynamic_pointer_cast<Mob>(target);
        mob->setCustomName(itemInstance->getHoverName());
        mob->setPersistenceRequired();
        itemInstance->count--;
        return true;
    }

    return Item::interactEnemy(itemInstance, player, target);
}