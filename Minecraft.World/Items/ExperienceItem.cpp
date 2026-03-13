#include "../Platform/stdafx.h"
#include "../Headers/net.minecraft.world.level.h"
#include "../Headers/net.minecraft.world.entity.projectile.h"
#include "../Headers/net.minecraft.world.entity.player.h"
#include "../Headers/net.minecraft.world.item.h"
#include "../Util/SoundTypes.h"
#include "ExperienceItem.h"

ExperienceItem::ExperienceItem(int id) : Item(id) {}

bool ExperienceItem::isFoil(std::shared_ptr<ItemInstance> itemInstance) {
    return true;
}

bool ExperienceItem::TestUse(Level* level, std::shared_ptr<Player> player) {
    return true;
}

std::shared_ptr<ItemInstance> ExperienceItem::use(
    std::shared_ptr<ItemInstance> itemInstance, Level* level,
    std::shared_ptr<Player> player) {
    if (!player->abilities.instabuild) {
        itemInstance->count--;
    }
    level->playSound(player, eSoundType_RANDOM_BOW, 0.5f,
                     0.4f / (random->nextFloat() * 0.4f + 0.8f));
    if (!level->isClientSide)
        level->addEntity(std::shared_ptr<ThrownExpBottle>(
            new ThrownExpBottle(level, player)));
    return itemInstance;
}