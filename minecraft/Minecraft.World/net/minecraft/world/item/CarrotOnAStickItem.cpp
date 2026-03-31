#include <memory>

#include "CarrotOnAStickItem.h"
#include "Minecraft.World/net/minecraft/world/entity/Entity.h"
#include "Minecraft.World/net/minecraft/world/entity/ai/goal/ControlledByPlayerGoal.h"
#include "Minecraft.World/net/minecraft/world/entity/animal/Pig.h"
#include "Minecraft.World/net/minecraft/world/entity/player/Player.h"
#include "Minecraft.World/net/minecraft/world/item/FishingRodItem.h"
#include "Minecraft.World/net/minecraft/world/item/Item.h"
#include "Minecraft.World/net/minecraft/world/item/ItemInstance.h"

CarrotOnAStickItem::CarrotOnAStickItem(int id) : Item(id) {
    setMaxStackSize(1);
    setMaxDamage(25);
}

bool CarrotOnAStickItem::isHandEquipped() { return true; }

bool CarrotOnAStickItem::isMirroredArt() { return true; }

std::shared_ptr<ItemInstance> CarrotOnAStickItem::use(
    std::shared_ptr<ItemInstance> itemInstance, Level* level,
    std::shared_ptr<Player> player) {
    if (player->isRiding()) {
        std::shared_ptr<Pig> pig =
            std::dynamic_pointer_cast<Pig>(player->riding);
        if (pig) {
            if (pig->getControlGoal()->canBoost() &&
                itemInstance->getMaxDamage() - itemInstance->getAuxValue() >=
                    7) {
                pig->getControlGoal()->boost();
                itemInstance->hurtAndBreak(7, player);

                if (itemInstance->count == 0) {
                    std::shared_ptr<ItemInstance> replacement =
                        std::shared_ptr<ItemInstance>(
                            new ItemInstance(Item::fishingRod));
                    replacement->setTag(itemInstance->tag);
                    return replacement;
                }
            }
        }
    }

    return itemInstance;
}