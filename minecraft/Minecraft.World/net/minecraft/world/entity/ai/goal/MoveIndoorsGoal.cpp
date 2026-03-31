#include "../../../../../../Header Files/stdafx.h"
#include "../control/net.minecraft.world.entity.ai.control.h"
#include "../util/net.minecraft.world.entity.ai.util.h"
#include "../navigation/net.minecraft.world.entity.ai.navigation.h"
#include "../village/net.minecraft.world.entity.ai.village.h"
#include "../../net.minecraft.world.entity.h"
#include "../../../level/net.minecraft.world.level.h"
#include "../../../level/dimension/net.minecraft.world.level.dimension.h"
#include "MoveIndoorsGoal.h"

MoveIndoorsGoal::MoveIndoorsGoal(PathfinderMob* mob) {
    insideX = insideZ = -1;

    this->mob = mob;
    setRequiredControlFlags(Control::MoveControlFlag);
}

bool MoveIndoorsGoal::canUse() {
    if ((mob->level->isDay() && !mob->level->isRaining()) ||
        mob->level->dimension->hasCeiling)
        return false;
    if (mob->getRandom()->nextInt(50) != 0) return false;
    if (insideX != -1 && mob->distanceToSqr(insideX, mob->y, insideZ) < 2 * 2)
        return false;
    std::shared_ptr<Village> village = mob->level->villages->getClosestVillage(
        GameMath::floor(mob->x), GameMath::floor(mob->y), GameMath::floor(mob->z), 14);
    if (village == nullptr) return false;
    std::shared_ptr<DoorInfo> _doorInfo = village->getBestDoorInfo(
        GameMath::floor(mob->x), GameMath::floor(mob->y), GameMath::floor(mob->z));
    doorInfo = _doorInfo;
    return _doorInfo != nullptr;
}

bool MoveIndoorsGoal::canContinueToUse() {
    return !mob->getNavigation()->isDone();
}

void MoveIndoorsGoal::start() {
    insideX = -1;
    std::shared_ptr<DoorInfo> _doorInfo = doorInfo.lock();
    if (_doorInfo == nullptr) {
        doorInfo = std::weak_ptr<DoorInfo>();
        return;
    }
    if (mob->distanceToSqr(_doorInfo->getIndoorX(), _doorInfo->y,
                           _doorInfo->getIndoorZ()) > 16 * 16) {
        Vec3 towards(_doorInfo->getIndoorX() + 0.5, _doorInfo->getIndoorY(),
                     _doorInfo->getIndoorZ() + 0.5);
        auto pos = RandomPos::getPosTowards(
            std::dynamic_pointer_cast<PathfinderMob>(mob->shared_from_this()),
            14, 3, &towards);
        if (pos.has_value())
            mob->getNavigation()->moveTo(pos->x, pos->y, pos->z, 1.0f);
    } else
        mob->getNavigation()->moveTo(_doorInfo->getIndoorX() + 0.5,
                                     _doorInfo->getIndoorY(),
                                     _doorInfo->getIndoorZ() + 0.5, 1.0f);
}

void MoveIndoorsGoal::stop() {
    std::shared_ptr<DoorInfo> _doorInfo = doorInfo.lock();
    if (_doorInfo == nullptr) {
        doorInfo = std::weak_ptr<DoorInfo>();
        return;
    }

    insideX = _doorInfo->getIndoorX();
    insideZ = _doorInfo->getIndoorZ();
    doorInfo = std::weak_ptr<DoorInfo>();
}
