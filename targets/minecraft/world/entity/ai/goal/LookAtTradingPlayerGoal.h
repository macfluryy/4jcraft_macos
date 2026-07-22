#pragma once

#include "LookAtPlayerGoal.h"

class Villager;

class LookAtTradingPlayerGoal : public LookAtPlayerGoal {
private:
    Villager* villager;  

public:
    LookAtTradingPlayerGoal(Villager* villager);

    virtual bool canUse();
};