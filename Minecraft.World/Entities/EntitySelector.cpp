#include "../Platform/stdafx.h"
#include "../Containers/Container.h"
#include "EntitySelector.h"

const EntitySelector* EntitySelector::ENTITY_STILL_ALIVE =
    new AliveEntitySelector();
const EntitySelector* EntitySelector::CONTAINER_ENTITY_SELECTOR =
    new ContainerEntitySelector();

bool AliveEntitySelector::matches(std::shared_ptr<Entity> entity) const {
    return entity->isAlive();
}

bool ContainerEntitySelector::matches(std::shared_ptr<Entity> entity) const {
    return (std::dynamic_pointer_cast<Container>(entity) != NULL) &&
           entity->isAlive();
}

MobCanWearArmourEntitySelector::MobCanWearArmourEntitySelector(
    std::shared_ptr<ItemInstance> item) {
    this->item = item;
}

bool MobCanWearArmourEntitySelector::matches(
    std::shared_ptr<Entity> entity) const {
    if (!entity->isAlive()) return false;
    if (!entity->instanceof(eTYPE_LIVINGENTITY)) return false;

    std::shared_ptr<LivingEntity> mob =
        std::dynamic_pointer_cast<LivingEntity>(entity);

    if (mob->getCarried(Mob::getEquipmentSlotForItem(item)) != NULL)
        return false;

    if (mob->instanceof(eTYPE_MOB)) {
        return std::dynamic_pointer_cast<Mob>(mob)->canPickUpLoot();
    } else if (mob->instanceof(eTYPE_PLAYER)) {
        return true;
    }

    return false;
}