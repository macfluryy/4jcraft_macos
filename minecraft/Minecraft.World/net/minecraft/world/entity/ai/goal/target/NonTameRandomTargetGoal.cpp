#include "NonTameRandomTargetGoal.h"
#include "Minecraft.World/net/minecraft/world/entity/TamableAnimal.h"
#include "Minecraft.World/net/minecraft/world/entity/ai/goal/target/NearestAttackableTargetGoal.h"

NonTameRandomTargetGoal::NonTameRandomTargetGoal(
    TamableAnimal* mob, const std::type_info& targetType, int randomInterval,
    bool mustSee)
    : NearestAttackableTargetGoal(mob, targetType, randomInterval, mustSee) {
    tamableMob = mob;
}

bool NonTameRandomTargetGoal::canUse() {
    return !tamableMob->isTame() && NearestAttackableTargetGoal::canUse();
}
