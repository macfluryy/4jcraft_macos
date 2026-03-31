#include <memory>
#include <string>

#include "FishingRodItem.h"
#include "SoundTypes.h"
#include "java/Random.h"
#include "Minecraft.World/net/minecraft/world/IconRegister.h"
#include "Minecraft.World/net/minecraft/world/entity/player/Player.h"
#include "Minecraft.World/net/minecraft/world/entity/projectile/FishingHook.h"
#include "Minecraft.World/net/minecraft/world/item/Item.h"
#include "Minecraft.World/net/minecraft/world/item/ItemInstance.h"
#include "Minecraft.World/net/minecraft/world/level/Level.h"

class Icon;

FishingRodItem::FishingRodItem(int id) : Item(id) {
    setMaxDamage(64);
    setMaxStackSize(1);
    emptyIcon = nullptr;
}

bool FishingRodItem::isHandEquipped() { return true; }

bool FishingRodItem::isMirroredArt() { return true; }

std::shared_ptr<ItemInstance> FishingRodItem::use(
    std::shared_ptr<ItemInstance> instance, Level* level,
    std::shared_ptr<Player> player) {
    if (player->fishing != nullptr) {
        int dmg = player->fishing->retrieve();
        instance->hurtAndBreak(dmg, player);
        player->swing();
    } else {
        level->playEntitySound(player, eSoundType_RANDOM_BOW, 0.5f,
                               0.4f / (random->nextFloat() * 0.4f + 0.8f));
        if (!level->isClientSide) {
            // 4J Stu - Move the player->fishing out of the ctor as we cannot
            // reference 'this'
            std::shared_ptr<FishingHook> hook =
                std::shared_ptr<FishingHook>(new FishingHook(level, player));
            player->fishing = hook;
            level->addEntity(std::shared_ptr<FishingHook>(hook));
        }
        player->swing();
    }
    return instance;
}

void FishingRodItem::registerIcons(IconRegister* iconRegister) {
    icon = iconRegister->registerIcon(getIconName() + L"_uncast");
    emptyIcon = iconRegister->registerIcon(getIconName() + L"_cast");
}

Icon* FishingRodItem::getEmptyIcon() { return emptyIcon; }
