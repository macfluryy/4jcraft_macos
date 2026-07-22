#include "Giant.h"

#include "minecraft/world/entity/ai/attributes/AttributeInstance.h"
#include "minecraft/world/entity/monster/Monster.h"
#include "minecraft/world/entity/monster/SharedMonsterAttributes.h"
#include "minecraft/world/level/Level.h"

Giant::Giant(Level* level) : Monster(level) {
    
    
    this->defineSynchedData();
    registerAttributes();

    heightOffset *= 6;
    setSize(bbWidth * 6, bbHeight * 6);
}

void Giant::registerAttributes() {
    Monster::registerAttributes();

    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(100);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.5f);
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(50);
}

float Giant::getWalkTargetValue(int x, int y, int z) {
    return level->getBrightness(x, y, z) - 0.5f;
}