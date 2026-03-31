#pragma once

#include "MobEffect.h"
#include "Minecraft.Client/Common/App_enums.h"

class LivingEntity;
class BaseAttributeMap;

class HealthBoostMobEffect : public MobEffect {
public:
    HealthBoostMobEffect(int id, bool isHarmful, eMinecraftColour color);

    void removeAttributeModifiers(std::shared_ptr<LivingEntity> entity,
                                  BaseAttributeMap* attributes, int amplifier);
};