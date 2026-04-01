#include <memory>

#include "SnowballItem.h"
#include "Minecraft.World/net/minecraft/sounds/SoundTypes.h"
#include "java/Random.h"
#include "Minecraft.World/net/minecraft/world/entity/player/Abilities.h"
#include "Minecraft.World/net/minecraft/world/entity/player/Player.h"
#include "Minecraft.World/net/minecraft/world/entity/projectile/Snowball.h"
#include "Minecraft.World/net/minecraft/world/item/Item.h"
#include "Minecraft.World/net/minecraft/world/item/ItemInstance.h"
#include "Minecraft.World/net/minecraft/world/level/Level.h"

class Entity;

SnowballItem::SnowballItem(int id) : Item(id) { this->maxStackSize = 16; }

std::shared_ptr<ItemInstance> SnowballItem::use(
    std::shared_ptr<ItemInstance> instance, Level* level,
    std::shared_ptr<Player> player) {
    if (!player->abilities.instabuild) {
        instance->count--;
    }
    level->playEntitySound((std::shared_ptr<Entity>)player,
                           eSoundType_RANDOM_BOW, 0.5f,
                           0.4f / (random->nextFloat() * 0.4f + 0.8f));
    if (!level->isClientSide)
        level->addEntity(
            std::make_shared<Snowball>(level, player));
    return instance;
}