

#include "../Platform/stdafx.h"
#include "../Headers/com.mojang.nbt.h"
#include "../Headers/net.minecraft.world.level.tile.h"
#include "../Headers/net.minecraft.world.phys.h"
#include "../Headers/net.minecraft.world.level.h"
#include "../Headers/net.minecraft.world.item.h"
#include "../Headers/net.minecraft.world.entity.player.h"
#include "../Headers/net.minecraft.world.entity.item.h"
#include "../Headers/net.minecraft.world.entity.global.h"
#include "../Headers/net.minecraft.world.entity.projectile.h"
#include "EggItem.h"
#include "../Util/SoundTypes.h"

EggItem::EggItem(int id) : Item(id) { this->maxStackSize = 16; }

std::shared_ptr<ItemInstance> EggItem::use(
    std::shared_ptr<ItemInstance> instance, Level* level,
    std::shared_ptr<Player> player) {
    if (!player->abilities.instabuild) {
        instance->count--;
    }
    level->playSound(std::dynamic_pointer_cast<Entity>(player),
                     eSoundType_RANDOM_BOW, 0.5f,
                     0.4f / (random->nextFloat() * 0.4f + 0.8f));
    if (!level->isClientSide)
        level->addEntity(std::shared_ptr<ThrownEgg>(
            new ThrownEgg(level, std::dynamic_pointer_cast<Mob>(player))));
    return instance;
}
