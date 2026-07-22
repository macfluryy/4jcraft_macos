#pragma once

#include "Goal.h"
#include "minecraft/SharedConstants.h"

class Level;
class Mob;



class EatTileGoal : public Goal {
private:
    static const int EAT_ANIMATION_TICKS =
        SharedConstants::TICKS_PER_SECOND * 2;

    Mob* mob;  
    Level* level;
    int eatAnimationTick;

public:
    EatTileGoal(Mob* mob);

    virtual bool canUse();
    virtual void start();
    virtual void stop();
    virtual bool canContinueToUse();
    virtual int getEatAnimationTick();
    virtual void tick();

    
    
    virtual void setLevel(Level* level) { this->level = level; }
};