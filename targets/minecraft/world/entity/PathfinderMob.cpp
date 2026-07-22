#include "PathfinderMob.h"

#include <cstdlib>

#include "java/Class.h"
#include "java/Random.h"
#include "minecraft/Pos.h"
#include "minecraft/SharedConstants.h"
#include "minecraft/util/Mth.h"
#include "minecraft/world/entity/Mob.h"
#include "minecraft/world/entity/TamableAnimal.h"
#include "minecraft/world/entity/ai/attributes/AttributeInstance.h"
#include "minecraft/world/entity/ai/attributes/AttributeModifier.h"
#include "minecraft/world/entity/ai/goal/Goal.h"
#include "minecraft/world/entity/ai/goal/GoalSelector.h"
#include "minecraft/world/entity/ai/goal/MoveTowardsRestrictionGoal.h"
#include "minecraft/world/entity/ai/navigation/PathNavigation.h"
#include "minecraft/world/entity/monster/SharedMonsterAttributes.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/pathfinder/Path.h"
#include "minecraft/world/phys/AABB.h"
#include "minecraft/world/phys/Vec3.h"

AttributeModifier* PathfinderMob::SPEED_MODIFIER_FLEEING =
    (new AttributeModifier(eModifierId_MOB_FLEEING, 2.0f,
                           AttributeModifier::OPERATION_MULTIPLY_TOTAL))
        ->setSerialize(false);

PathfinderMob::PathfinderMob(Level* level) : Mob(level) {
    path = nullptr;
    attackTarget = nullptr;
    holdGround = false;
    fleeTime = 0;

    restrictRadius = -1;
    restrictCenter = new Pos(0, 0, 0);
    addedLeashRestrictionGoal = false;
    leashRestrictionGoal = new MoveTowardsRestrictionGoal(this, 1.0f);
}

bool PathfinderMob::shouldHoldGround() { return false; }

PathfinderMob::~PathfinderMob() {
    delete path;
    delete restrictCenter;
    delete leashRestrictionGoal;
}

void PathfinderMob::serverAiStep() {
    if (fleeTime > 0) {
        if (--fleeTime == 0) {
            AttributeInstance* speed =
                getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);
            speed->removeModifier(SPEED_MODIFIER_FLEEING);
        }
    }
    holdGround = shouldHoldGround();
    float maxDist = 16;

    if (attackTarget == nullptr) {
        attackTarget = findAttackTarget();
        if (attackTarget != nullptr) {
            setPath(level->findPath(
                shared_from_this(), attackTarget, maxDist, true, false, false,
                true));  
        }
    } else {
        if (attackTarget->isAlive()) {
            float d = attackTarget->distanceTo(shared_from_this());
            if (canSee(attackTarget)) {
                checkHurtTarget(attackTarget, d);
            }
        } else {
            attackTarget = nullptr;
        }
    }

    



    
    
    
    
    
    
    
    
    

    if (!holdGround && (attackTarget != nullptr &&
                        (path == nullptr || random->nextInt(20) == 0))) {
        setPath(level->findPath(shared_from_this(), attackTarget, maxDist, true,
                                false, false,
                                true));  
    } else if (!holdGround &&
               ((path == nullptr && (random->nextInt(180) == 0) ||
                 fleeTime > 0) ||
                (random->nextInt(120) == 0 || fleeTime > 0))) {
        if (noActionTime < SharedConstants::TICKS_PER_SECOND * 5) {
            findRandomStrollLocation();
        }
    } else if (!holdGround && (path == nullptr)) {
        if ((noActionTime >= SharedConstants::TICKS_PER_SECOND * 5) &&
            isExtraWanderingEnabled()) {
            
            
            
            
            
            
            findRandomStrollLocation(getWanderingQuadrant());
        }
    }

    
    
    
    
    considerForExtraWandering(isDespawnProtected());

    int yFloor = Mth::floor(bb.y0 + 0.5f);

    bool inWater = isInWater();
    bool inLava = isInLava();
    xRot = 0;
    if (path == nullptr || random->nextInt(100) == 0) {
        this->Mob::serverAiStep();
        setPath(nullptr);  
        return;
    }

    Vec3 target = path->currentPos(shared_from_this());
    double r = bbWidth * 2;
    while (target.distanceToSqr(x, target.y, z) < r * r) {
        path->next();
        if (path->isDone()) {
            setPath(nullptr);  
            break;
        } else
            target = path->currentPos(shared_from_this());
    }

    jumping = false;
    
    



































    if (attackTarget != nullptr) {
        lookAt(attackTarget, 30, 30);
    }

    if (horizontalCollision && !isPathFinding()) jumping = true;
    if (random->nextFloat() < 0.8f && (inWater || inLava)) jumping = true;
}

void PathfinderMob::findRandomStrollLocation(
    int quadrant )  
{
    bool hasBest = false;
    int xBest = -1;
    int yBest = -1;
    int zBest = -1;
    float best = -99999;
    for (int i = 0; i < 10; i++) {
        
        
        
        int xt, zt;
        int yt = Mth::floor(y + random->nextInt(7) - 3);
        if (quadrant == -1) {
            xt = Mth::floor(x + random->nextInt(13) - 6);
            zt = Mth::floor(z + random->nextInt(13) - 6);
        } else {
            int sx = ((quadrant & 1) ? -1 : 1);
            int sz = ((quadrant & 2) ? -1 : 1);
            xt = Mth::floor(x + random->nextInt(7) * sx);
            zt = Mth::floor(z + random->nextInt(7) * sz);
        }
        float value = getWalkTargetValue(xt, yt, zt);
        if (value > best) {
            best = value;
            xBest = xt;
            yBest = yt;
            zBest = zt;
            hasBest = true;
        }
    }
    if (hasBest) {
        setPath(level->findPath(shared_from_this(), xBest, yBest, zBest, 10,
                                true, false, false,
                                true));  
    }
}

void PathfinderMob::checkHurtTarget(std::shared_ptr<Entity> target, float d) {}

float PathfinderMob::getWalkTargetValue(int x, int y, int z) { return 0; }

std::shared_ptr<Entity> PathfinderMob::findAttackTarget() {
    return std::shared_ptr<Entity>();
}

bool PathfinderMob::canSpawn() {
    int xt = Mth::floor(x);
    int yt = Mth::floor(bb.y0);
    int zt = Mth::floor(z);
    return this->Mob::canSpawn() && getWalkTargetValue(xt, yt, zt) >= 0;
}

bool PathfinderMob::isPathFinding() { return path != nullptr; }

void PathfinderMob::setPath(Path* path) {
    delete this->path;
    this->path = path;
}

std::shared_ptr<Entity> PathfinderMob::getAttackTarget() {
    return attackTarget;
}

void PathfinderMob::setAttackTarget(std::shared_ptr<Entity> attacker) {
    attackTarget = attacker;
}


bool PathfinderMob::isWithinRestriction() {
    return isWithinRestriction(Mth::floor(x), Mth::floor(y), Mth::floor(z));
}

bool PathfinderMob::isWithinRestriction(int x, int y, int z) {
    if (restrictRadius == -1) return true;
    return restrictCenter->distSqr(x, y, z) < restrictRadius * restrictRadius;
}

void PathfinderMob::restrictTo(int x, int y, int z, int radius) {
    restrictCenter->set(x, y, z);
    restrictRadius = radius;
}

Pos* PathfinderMob::getRestrictCenter() { return restrictCenter; }

float PathfinderMob::getRestrictRadius() { return restrictRadius; }

void PathfinderMob::clearRestriction() { restrictRadius = -1; }

bool PathfinderMob::hasRestriction() { return restrictRadius != -1; }

void PathfinderMob::tickLeash() {
    Mob::tickLeash();

    if (isLeashed() && getLeashHolder() != nullptr &&
        getLeashHolder()->level == this->level) {
        
        std::shared_ptr<Entity> leashHolder = getLeashHolder();
        restrictTo((int)leashHolder->x, (int)leashHolder->y,
                   (int)leashHolder->z, 5);

        float _distanceTo = distanceTo(leashHolder);

        std::shared_ptr<TamableAnimal> tamabaleAnimal =
            shared_from_this()->instanceof(eTYPE_TAMABLE_ANIMAL)
                ? std::dynamic_pointer_cast<TamableAnimal>(shared_from_this())
                : nullptr;
        if ((tamabaleAnimal != nullptr) && tamabaleAnimal->isSitting()) {
            if (_distanceTo > 10) {
                dropLeash(true, true);
            }
            return;
        }

        if (!addedLeashRestrictionGoal) {
            goalSelector.addGoal(2, leashRestrictionGoal, false);
            getNavigation()->setAvoidWater(false);
            addedLeashRestrictionGoal = true;
        }

        onLeashDistance(_distanceTo);

        if (_distanceTo > 4) {
            
            getNavigation()->moveTo(leashHolder, 1.0);
        }
        if (_distanceTo > 6) {
            
            double dx = (leashHolder->x - x) / _distanceTo;
            double dy = (leashHolder->y - y) / _distanceTo;
            double dz = (leashHolder->z - z) / _distanceTo;

            xd += dx * std::abs(dx) * .4;
            yd += dy * std::abs(dy) * .4;
            zd += dz * std::abs(dz) * .4;
        }
        if (_distanceTo > 10) {
            dropLeash(true, true);
        }

    } else if (!isLeashed() && addedLeashRestrictionGoal) {
        addedLeashRestrictionGoal = false;
        goalSelector.removeGoal(leashRestrictionGoal);
        getNavigation()->setAvoidWater(true);
        clearRestriction();
    }
}

void PathfinderMob::onLeashDistance(float distanceToLeashHolder) {}

bool PathfinderMob::couldWander() {
    return (noActionTime < SharedConstants::TICKS_PER_SECOND * 5) ||
           (isExtraWanderingEnabled());
}
