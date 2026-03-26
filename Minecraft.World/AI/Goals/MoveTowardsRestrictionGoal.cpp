#include "../../Platform/stdafx.h"
#include "../../Headers/net.minecraft.world.entity.ai.control.h"
#include "../../Headers/net.minecraft.world.entity.ai.util.h"
#include "../../Headers/net.minecraft.world.entity.ai.navigation.h"
#include "../../Headers/net.minecraft.world.entity.h"
#include "../../Headers/net.minecraft.world.level.h"
#include "MoveTowardsRestrictionGoal.h"

MoveTowardsRestrictionGoal::MoveTowardsRestrictionGoal(PathfinderMob* mob,
                                                       double speedModifier) {
    wantedX = wantedY = wantedZ = 0.0;

    this->mob = mob;
    this->speedModifier = speedModifier;
    setRequiredControlFlags(Control::MoveControlFlag);
}

bool MoveTowardsRestrictionGoal::canUse() {
    if (mob->isWithinRestriction()) return false;
    Pos* toward = mob->getRestrictCenter();
    Vec3 towards(toward->x, toward->y, toward->z);
    Vec3* pos = RandomPos::getPosTowards(
        std::dynamic_pointer_cast<PathfinderMob>(mob->shared_from_this()), 16,
        7, &towards);
    if (pos == NULL) return false;
    wantedX = pos->x;
    wantedY = pos->y;
    wantedZ = pos->z;
    return true;
}

bool MoveTowardsRestrictionGoal::canContinueToUse() {
    return !mob->getNavigation()->isDone();
}

void MoveTowardsRestrictionGoal::start() {
    mob->getNavigation()->moveTo(wantedX, wantedY, wantedZ, speedModifier);
}
