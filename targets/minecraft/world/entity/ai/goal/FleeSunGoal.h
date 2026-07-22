#pragma once

#include <optional>

#include "Goal.h"
#include "minecraft/world/phys/Vec3.h"

class Level;
class PathfinderMob;

class FleeSunGoal : public Goal {
private:
    PathfinderMob* mob;  
    double wantedX, wantedY, wantedZ;
    double speedModifier;
    Level* level;

public:
    FleeSunGoal(PathfinderMob* mob, double speedModifier);

    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();

private:
    std::optional<Vec3> getHidePos();

public:
    
    
    virtual void setLevel(Level* level) { this->level = level; }
};
