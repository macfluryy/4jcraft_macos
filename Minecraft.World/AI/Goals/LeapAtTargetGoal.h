#pragma once

#include "Goal.h"

class LeapAtTargetGoal : public Goal
{
private:
    Mob *mob; // Owner of this goal
    std::<Mob> target;
    float yd;

public:
	LeapAtTargetGoal(Mob *mob, float yd);

    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
};