#include "../../../../../../Header Files/stdafx.h"
#include "../navigation/net.minecraft.world.entity.ai.navigation.h"
#include "../village/net.minecraft.world.entity.ai.village.h"
#include "../../net.minecraft.world.entity.h"
#include "../../../level/net.minecraft.world.level.h"
#include "RestrictOpenDoorGoal.h"

RestrictOpenDoorGoal::RestrictOpenDoorGoal(PathfinderMob* mob) {
    this->mob = mob;
}

bool RestrictOpenDoorGoal::canUse() {
    if (mob->level->isDay()) return false;
    std::shared_ptr<Village> village = mob->level->villages->getClosestVillage(
        Mth::floor(mob->x), Mth::floor(mob->y), Mth::floor(mob->z), 16);
    if (village == nullptr) return false;
    std::shared_ptr<DoorInfo> _doorInfo = village->getClosestDoorInfo(
        Mth::floor(mob->x), Mth::floor(mob->y), Mth::floor(mob->z));
    if (_doorInfo == nullptr) return false;
    doorInfo = _doorInfo;
    return _doorInfo->distanceToInsideSqr(Mth::floor(mob->x),
                                          Mth::floor(mob->y),
                                          Mth::floor(mob->z)) < 1.5 * 1.5;
}

bool RestrictOpenDoorGoal::canContinueToUse() {
    if (mob->level->isDay()) return false;
    std::shared_ptr<DoorInfo> _doorInfo = doorInfo.lock();
    if (_doorInfo == nullptr) return false;
    return !_doorInfo->removed &&
           _doorInfo->isInsideSide(Mth::floor(mob->x), Mth::floor(mob->z));
}

void RestrictOpenDoorGoal::start() {
    mob->getNavigation()->setCanOpenDoors(false);
    mob->getNavigation()->setCanPassDoors(false);
}

void RestrictOpenDoorGoal::stop() {
    mob->getNavigation()->setCanOpenDoors(true);
    mob->getNavigation()->setCanPassDoors(true);
    doorInfo = std::weak_ptr<DoorInfo>();
}

void RestrictOpenDoorGoal::tick() {
    std::shared_ptr<DoorInfo> _doorInfo = doorInfo.lock();
    if (_doorInfo) _doorInfo->incBookingCount();
}