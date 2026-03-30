#include "../../../../../../../Header Files/stdafx.h"
#include "../../../animal/net.minecraft.world.entity.animal.h"
#include "NonTameRandomTargetGoal.h"

NonTameRandomTargetGoal::NonTameRandomTargetGoal(
    TamableAnimal* mob, const std::type_info& targetType, int randomInterval,
    bool mustSee)
    : NearestAttackableTargetGoal(mob, targetType, randomInterval, mustSee) {
    tamableMob = mob;
}

bool NonTameRandomTargetGoal::canUse() {
    return !tamableMob->isTame() && NearestAttackableTargetGoal::canUse();
}
