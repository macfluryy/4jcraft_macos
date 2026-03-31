#include <memory>

#include "HealthBoostMobEffect.h"
#include "Minecraft.World/net/minecraft/world/effect/MobEffect.h"
#include "Minecraft.World/net/minecraft/world/entity/LivingEntity.h"

HealthBoostMobEffect::HealthBoostMobEffect(int id, bool isHarmful,
                                           eMinecraftColour color)
    : MobEffect(id, isHarmful, color) {}

void HealthBoostMobEffect::removeAttributeModifiers(
    std::shared_ptr<LivingEntity> entity, BaseAttributeMap* attributes,
    int amplifier) {
    MobEffect::removeAttributeModifiers(entity, attributes, amplifier);
    if (entity->getHealth() > entity->getMaxHealth()) {
        entity->setHealth(entity->getMaxHealth());
    }
}
