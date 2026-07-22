#pragma once

#include <memory>

#include "Goal.h"
#include "java/Class.h"

class Level;
class PathfinderMob;
class Path;

class MeleeAttackGoal : public Goal {
private:
    Level* level;
    PathfinderMob* mob;  
    int attackTime;
    double speedModifier;
    bool trackTarget;
    std::unique_ptr<Path> path;
    eINSTANCEOF attackType;
    int timeToRecalcPath;

    void _init(PathfinderMob* mob, double speedModifier, bool trackTarget);

public:
    MeleeAttackGoal(PathfinderMob* mob, eINSTANCEOF attackType,
                    double speedModifier, bool trackTarget);
    MeleeAttackGoal(PathfinderMob* mob, double speedModifier, bool trackTarget);
    ~MeleeAttackGoal();

    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual void tick();

    
    
    virtual void setLevel(Level* level) { this->level = level; }
};
