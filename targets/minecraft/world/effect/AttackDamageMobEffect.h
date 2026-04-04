#pragma once

#include "app/common/App_enums.h"
#include "MobEffect.h"

class AttributeModifier;

class AttackDamageMobEffect : public MobEffect {
public:
    AttackDamageMobEffect(int id, bool isHarmful, eMinecraftColour color);

    double getAttributeModifierValue(int amplifier,
                                     AttributeModifier* original);
};