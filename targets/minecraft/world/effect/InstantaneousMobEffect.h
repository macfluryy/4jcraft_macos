#pragma once

#include "app/common/App_enums.h"
#include "MobEffect.h"

class InstantenousMobEffect : public MobEffect {
public:
    InstantenousMobEffect(int id, bool isHarmful, eMinecraftColour color);
    bool isInstantenous();
    bool isDurationEffectTick(int remainingDuration, int amplification);
};