#include "../../Platform/stdafx.h"
#include "../../Headers/net.minecraft.world.entity.ai.control.h"
#include "../../Headers/net.minecraft.world.entity.ai.navigation.h"
#include "../../Headers/net.minecraft.world.entity.ai.util.h"
#include "../../Headers/net.minecraft.world.entity.h"
#include "../../Headers/net.minecraft.world.phys.h"
#include "MoveTowardsTargetGoal.h"

MoveTowardsTargetGoal::MoveTowardsTargetGoal(PathfinderMob* mob,
                                             double speedModifier,
                                             float within) {
    this->mob = mob;
    this->speedModifier = speedModifier;
    this->within = within;
    setRequiredControlFlags(Control::MoveControlFlag);
}

bool MoveTowardsTargetGoal::canUse() {
    target = std::weak_ptr<LivingEntity>(mob->getTarget());
    if (target.lock() == NULL) return false;
    if (target.lock()->distanceToSqr(mob->shared_from_this()) > within * within)
        return false;
    Vec3 towards(target.lock()->x, target.lock()->y, target.lock()->z);
    auto pos = RandomPos::getPosTowards(
        std::dynamic_pointer_cast<PathfinderMob>(mob->shared_from_this()), 16,
        7, &towards);
    if (!pos.has_value()) return false;
    wantedX = pos->x;
    wantedY = pos->y;
    wantedZ = pos->z;
    return true;
}

bool MoveTowardsTargetGoal::canContinueToUse() {
    return target.lock() != NULL && !mob->getNavigation()->isDone() &&
           target.lock()->isAlive() &&
           target.lock()->distanceToSqr(mob->shared_from_this()) <
               within * within;
}

void MoveTowardsTargetGoal::stop() { target = std::weak_ptr<Mob>(); }

void MoveTowardsTargetGoal::start() {
    mob->getNavigation()->moveTo(wantedX, wantedY, wantedZ, speedModifier);
}
