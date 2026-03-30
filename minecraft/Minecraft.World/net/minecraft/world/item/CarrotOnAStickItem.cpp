#include "../../../../Header Files/stdafx.h"
#include "../entity/animal/net.minecraft.world.entity.animal.h"
#include "../entity/player/net.minecraft.world.entity.player.h"
#include "../entity/ai/goal/net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.item.h"
#include "CarrotOnAStickItem.h"

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