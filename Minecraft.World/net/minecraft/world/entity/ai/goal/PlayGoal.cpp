#include "../../../../../../Header Files/stdafx.h"
#include "../control/net.minecraft.world.entity.ai.control.h"
#include "../util/net.minecraft.world.entity.ai.util.h"
#include "../navigation/net.minecraft.world.entity.ai.navigation.h"
#include "../../npc/net.minecraft.world.entity.npc.h"
#include "../../net.minecraft.world.entity.h"
#include "../../../level/net.minecraft.world.level.h"
#include "../../../phys/net.minecraft.world.phys.h"
#include "PlayGoal.h"
#include <limits>

PlayGoal::PlayGoal(Villager* mob, double speedModifier) {
    followFriend = std::weak_ptr<LivingEntity>();
    wantedX = wantedY = wantedZ = 0.0;
    playTime = 0;

    this->mob = mob;
    this->speedModifier = speedModifier;
    setRequiredControlFlags(Control::MoveControlFlag);
}

bool PlayGoal::canUse() {
    if (mob->getAge() >= 0) return false;
    if (mob->getRandom()->nextInt(400) != 0) return false;

    AABB mob_bb = mob->bb.grow(6, 3, 6);
    std::vector<std::shared_ptr<Entity> >* children =
        mob->level->getEntitiesOfClass(typeid(Villager), &mob_bb);
    double closestDistSqr = std::numeric_limits<double>::max();
    // for (Entity c : children)
    for (auto it = children->begin(); it != children->end(); ++it) {
        std::shared_ptr<Entity> c = *it;
        if (c.get() == mob) continue;
        std::shared_ptr<Villager> friendV =
            std::dynamic_pointer_cast<Villager>(c);
        if (friendV->isChasing()) continue;
        if (friendV->getAge() >= 0) continue;
        double distSqr = friendV->distanceToSqr(mob->shared_from_this());
        if (distSqr > closestDistSqr) continue;
        closestDistSqr = distSqr;
        followFriend = std::weak_ptr<LivingEntity>(friendV);
    }
    delete children;

    if (followFriend.lock() == nullptr) {
        auto pos = RandomPos::getPos(
            std::dynamic_pointer_cast<PathfinderMob>(mob->shared_from_this()),
            16, 3);
        if (!pos.has_value()) return false;
    }
    return true;
}

bool PlayGoal::canContinueToUse() {
    return playTime > 0 && followFriend.lock() != nullptr;
}

void PlayGoal::start() {
    if (followFriend.lock() != nullptr) mob->setChasing(true);
    playTime = 1000;
}

void PlayGoal::stop() {
    mob->setChasing(false);
    followFriend = std::weak_ptr<LivingEntity>();
}

void PlayGoal::tick() {
    --playTime;
    if (followFriend.lock() != nullptr) {
        if (mob->distanceToSqr(followFriend.lock()) > 2 * 2)
            mob->getNavigation()->moveTo(followFriend.lock(), speedModifier);
    } else {
        if (mob->getNavigation()->isDone()) {
            auto pos =
                RandomPos::getPos(std::dynamic_pointer_cast<PathfinderMob>(
                                      mob->shared_from_this()),
                                  16, 3);
            if (!pos.has_value()) return;
            mob->getNavigation()->moveTo(pos->x, pos->y, pos->z, speedModifier);
        }
    }
}
