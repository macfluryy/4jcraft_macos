#pragma once

#include "MobEffect.h"
#include "Minecraft.Client/Common/App_enums.h"

class InstantenousMobEffect : public MobEffect {
public:
    InstantenousMobEffect(int id, bool isHarmful, eMinecraftColour color);
    bool isInstantenous();
    bool isDurationEffectTick(int remainingDuration, int amplification);
};