#include "RandomStrollGoal.h"

#include <memory>
#include <optional>

#include "java/Random.h"
#include "minecraft/SharedConstants.h"
#include "minecraft/world/entity/PathfinderMob.h"
#include "minecraft/world/entity/ai/control/Control.h"
#include "minecraft/world/entity/ai/navigation/PathNavigation.h"
#include "minecraft/world/entity/ai/util/RandomPos.h"
#include "minecraft/world/phys/Vec3.h"

RandomStrollGoal::RandomStrollGoal(PathfinderMob* mob, double speedModifier) {
    this->mob = mob;
    this->speedModifier = speedModifier;
    setRequiredControlFlags(Control::MoveControlFlag |
                            Control::LookControlFlag);
}

bool RandomStrollGoal::canUse() {
    
    
    
    if (mob->getNoActionTime() < SharedConstants::TICKS_PER_SECOND * 5) {
        if (mob->getRandom()->nextInt(120) == 0) {
            auto pos =
                RandomPos::getPos(std::dynamic_pointer_cast<PathfinderMob>(
                                      mob->shared_from_this()),
                                  10, 7);
            if (!pos.has_value()) return false;
            wantedX = pos->x;
            wantedY = pos->y;
            wantedZ = pos->z;
            return true;
        }
    } else {
        
        
        
        
        

        if (mob->isExtraWanderingEnabled()) {
            auto pos =
                RandomPos::getPos(std::dynamic_pointer_cast<PathfinderMob>(
                                      mob->shared_from_this()),
                                  10, 7, mob->getWanderingQuadrant());
            if (!pos.has_value()) return false;
            wantedX = pos->x;
            wantedY = pos->y;
            wantedZ = pos->z;
            return true;
        }
    }
    return false;
}

bool RandomStrollGoal::canContinueToUse() {
    return !mob->getNavigation()->isDone();
}

void RandomStrollGoal::start() {
    mob->getNavigation()->moveTo(wantedX, wantedY, wantedZ, speedModifier);
}
