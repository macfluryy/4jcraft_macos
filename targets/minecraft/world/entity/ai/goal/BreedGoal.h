#pragma once

#include <memory>

#include "Goal.h"

class Animal;
class Level;

class BreedGoal : public Goal {
private:
    Animal* animal;  
    Level* level;
    std::weak_ptr<Animal> partner;
    int loveTime;
    double speedModifier;

public:
    BreedGoal(Animal* animal, double speedModifier);

    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void stop();
    virtual void tick();

private:
    std::shared_ptr<Animal> getFreePartner();
    void breed();

public:
    
    
    virtual void setLevel(Level* level) { this->level = level; }
};