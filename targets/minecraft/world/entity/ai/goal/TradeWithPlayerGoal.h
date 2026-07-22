#pragma once

#include "Goal.h"

class Villager;

class TradeWithPlayerGoal : public Goal {
private:
    Villager* mob;  

public:
    TradeWithPlayerGoal(Villager* mob);

    bool canUse();
    void start();
    void stop();
};