#include "../Platform/stdafx.h"
#include "../Headers/net.minecraft.world.entity.animal.h"
#include "../Headers/net.minecraft.world.item.h"
#include "../Headers/net.minecraft.world.entity.h"
#include "SaddleItem.h"

SaddleItem::SaddleItem(int id) : Item(id) { maxStackSize = 1; }

bool SaddleItem::interactEnemy(std::shared_ptr<ItemInstance> itemInstance,
                               std::shared_ptr<Player> player,
                               std::shared_ptr<LivingEntity> mob) {
    if ((mob != NULL) && mob->instanceof(eTYPE_PIG)) {
        std::shared_ptr<Pig> pig = std::dynamic_pointer_cast<Pig>(mob);
        if (!pig->hasSaddle() && !pig->isBaby()) {
            pig->setSaddle(true);
            itemInstance->count--;
        }
        return true;
    }
    return false;
}

bool SaddleItem::hurtEnemy(std::shared_ptr<ItemInstance> itemInstance,
                           std::shared_ptr<LivingEntity> mob,
                           std::shared_ptr<LivingEntity> attacker) {
    interactEnemy(itemInstance, nullptr, mob);
    return true;
}