#include "../../Platform/stdafx.h"
#include "../../Headers/net.minecraft.world.entity.ai.control.h"
#include "../../Headers/net.minecraft.world.entity.ai.navigation.h"
#include "../../Headers/net.minecraft.world.entity.ai.util.h"
#include "../../Headers/net.minecraft.world.entity.ai.village.h"
#include "../../Headers/net.minecraft.world.entity.h"
#include "../../Headers/net.minecraft.world.level.h"
#include "MoveThroughVillageGoal.h"
#include <limits>
#include "../Navigation/Path.h"

MoveThroughVillageGoal::MoveThroughVillageGoal(PathfinderMob* mob,
                                               double speedModifier,
                                               bool onlyAtNight) {
    path = NULL;
    doorInfo = std::weak_ptr<DoorInfo>();

    this->mob = mob;
    this->speedModifier = speedModifier;
    this->onlyAtNight = onlyAtNight;
    setRequiredControlFlags(Control::MoveControlFlag);
}

MoveThroughVillageGoal::~MoveThroughVillageGoal() {
    if (path != NULL) delete path;
}

bool MoveThroughVillageGoal::canUse() {
    updateVisited();

    if (onlyAtNight && mob->level->isDay()) return false;

    std::shared_ptr<Village> village = mob->level->villages->getClosestVillage(
        Mth::floor(mob->x), Mth::floor(mob->y), Mth::floor(mob->z), 0);
    if (village == NULL) return false;

    std::shared_ptr<DoorInfo> _doorInfo = getNextDoorInfo(village);
    if (_doorInfo == NULL) return false;
    doorInfo = _doorInfo;

    bool oldCanOpenDoors = mob->getNavigation()->canOpenDoors();
    mob->getNavigation()->setCanOpenDoors(false);
    delete path;

    path = mob->getNavigation()->createPath(_doorInfo->x, _doorInfo->y,
                                            _doorInfo->z);
    mob->getNavigation()->setCanOpenDoors(oldCanOpenDoors);
    if (path != NULL) return true;

    Vec3 towards(_doorInfo->x, _doorInfo->y, _doorInfo->z);
    auto pos = RandomPos::getPosTowards(
        std::dynamic_pointer_cast<PathfinderMob>(mob->shared_from_this()), 10,
        7, &towards);
    if (!pos.has_value()) return false;
    mob->getNavigation()->setCanOpenDoors(false);
    delete path;
    path = mob->getNavigation()->createPath(pos->x, pos->y, pos->z);
    mob->getNavigation()->setCanOpenDoors(oldCanOpenDoors);
    return path != NULL;
}

bool MoveThroughVillageGoal::canContinueToUse() {
    if (mob->getNavigation()->isDone()) return false;
    float dist = mob->bbWidth + 4.f;
    std::shared_ptr<DoorInfo> _doorInfo = doorInfo.lock();
    if (_doorInfo == NULL) return false;

    return mob->distanceToSqr(_doorInfo->x, _doorInfo->y, _doorInfo->z) >
           dist * dist;
}

void MoveThroughVillageGoal::start() {
    mob->getNavigation()->moveTo(path, speedModifier);
    path = NULL;
}

void MoveThroughVillageGoal::stop() {
    std::shared_ptr<DoorInfo> _doorInfo = doorInfo.lock();
    if (_doorInfo == NULL) return;

    if (mob->getNavigation()->isDone() ||
        mob->distanceToSqr(_doorInfo->x, _doorInfo->y, _doorInfo->z) < 4 * 4) {
        visited.push_back(doorInfo);
    }
}

std::shared_ptr<DoorInfo> MoveThroughVillageGoal::getNextDoorInfo(
    std::shared_ptr<Village> village) {
    std::shared_ptr<DoorInfo> closest = nullptr;
    int closestDistSqr = std::numeric_limits<int>::max();
    std::vector<std::shared_ptr<DoorInfo> >* doorInfos =
        village->getDoorInfos();
    // for (DoorInfo di : doorInfos)
    for (auto it = doorInfos->begin(); it != doorInfos->end(); ++it) {
        std::shared_ptr<DoorInfo> di = *it;
        int distSqr = di->distanceToSqr(Mth::floor(mob->x), Mth::floor(mob->y),
                                        Mth::floor(mob->z));
        if (distSqr < closestDistSqr) {
            if (hasVisited(di)) continue;
            closest = di;
            closestDistSqr = distSqr;
        }
    }
    return closest;
}

bool MoveThroughVillageGoal::hasVisited(std::shared_ptr<DoorInfo> di) {
    // for (DoorInfo di2 : visited)
    for (auto it = visited.begin(); it != visited.end();) {
        std::shared_ptr<DoorInfo> di2 = (*it).lock();
        if (di2 == NULL) {
            it = visited.erase(it);
        } else {
            if (di->x == di2->x && di->y == di2->y && di->z == di2->z)
                return true;
            ++it;
        }
    }
    return false;
}

void MoveThroughVillageGoal::updateVisited() {
    if (visited.size() > 15) visited.erase(visited.begin());
}
