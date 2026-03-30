#include "../../../../../../../Header Files/stdafx.h"
#include "../../village/net.minecraft.world.entity.ai.village.h"
#include "../../../animal/net.minecraft.world.entity.animal.h"
#include "DefendVillageTargetGoal.h"

DefendVillageTargetGoal::DefendVillageTargetGoal(VillagerGolem* golem)
    : TargetGoal(golem, false, true) {
    this->golem = golem;
    setRequiredControlFlags(TargetGoal::TargetFlag);
}

bool DefendVillageTargetGoal::canUse() {
    std::shared_ptr<Village> village = golem->getVillage();
    if (village == nullptr) return false;
    potentialTarget = std::weak_ptr<LivingEntity>(village->getClosestAggressor(
        std::dynamic_pointer_cast<LivingEntity>(golem->shared_from_this())));
    std::shared_ptr<LivingEntity> potTarget = potentialTarget.lock();
    if (!canAttack(potTarget, false)) {
        // look for bad players
        if (mob->getRandom()->nextInt(20) == 0) {
            potentialTarget = village->getClosestBadStandingPlayer(
                std::dynamic_pointer_cast<LivingEntity>(
                    golem->shared_from_this()));
            return canAttack(potTarget, false);
        }
        return false;
    } else {
        return true;
    }
}

void DefendVillageTargetGoal::start() {
    golem->setTarget(potentialTarget.lock());
    TargetGoal::start();
}