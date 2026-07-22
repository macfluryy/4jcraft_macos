#pragma once

#include <memory>

#include "Goal.h"

class Villager;
class Village;
class Level;

class MakeLoveGoal : public Goal {
private:
    Villager* villager;  
    std::weak_ptr<Villager> partner;
    Level* level;
    int loveMakingTime;
    std::weak_ptr<Village> village;

public:
    MakeLoveGoal(Villager* villager);

    bool canUse();
    void start();
    void stop();
    bool canContinueToUse();
    void tick();

private:
    bool villageNeedsMoreVillagers();
    void breed();

public:
    
    
    virtual void setLevel(Level* level) { this->level = level; }
};