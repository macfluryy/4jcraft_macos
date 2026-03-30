
#include "../../../../Header Files/stdafx.h"
#include "../../../../com/mojang/nbt/com.mojang.nbt.h"
#include "../level/tile/net.minecraft.world.level.tile.h"
#include "../phys/net.minecraft.world.phys.h"
#include "../level/net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "../entity/player/net.minecraft.world.entity.player.h"
#include "../entity/item/net.minecraft.world.entity.item.h"
#include "../entity/global/net.minecraft.world.entity.global.h"
#include "../entity/projectile/net.minecraft.world.entity.projectile.h"
#include "EggItem.h"
#include "../../../../Header Files/SoundTypes.h"

EggItem::EggItem(int id) : Item(id) { maxStackSize = 16; }

std::shared_ptr<ItemInstance> EggItem::use(
    std::shared_ptr<ItemInstance> instance, Level* level,
    std::shared_ptr<Player> player) {
    if (!player->abilities.instabuild) {
        instance->count--;
    }
    level->playEntitySound(player, eSoundType_RANDOM_BOW, 0.5f,
                           0.4f / (random->nextFloat() * 0.4f + 0.8f));
    if (!level->isClientSide)
        level->addEntity(
            std::shared_ptr<ThrownEgg>(new ThrownEgg(level, player)));
    return instance;
}
