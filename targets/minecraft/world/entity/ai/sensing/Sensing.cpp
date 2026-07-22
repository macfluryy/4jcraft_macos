#include "Sensing.h"

#include "minecraft/world/entity/Mob.h"

class Entity;

Sensing::Sensing(Mob* mob) { this->mob = mob; }

void Sensing::tick() {
    seen.clear();
    unseen.clear();
}

bool Sensing::canSee(std::shared_ptr<Entity> target) {
    
    
    
    for (auto it = seen.begin(); it != seen.end(); ++it) {
        if (target == (*it).lock()) return true;
    }
    for (auto it = unseen.begin(); it != unseen.end(); ++it) {
        if (target == (*it).lock()) return false;
    }

    
    bool canSee = mob->canSee(target);
    
    if (canSee)
        seen.push_back(std::weak_ptr<Entity>(target));
    else
        unseen.push_back(std::weak_ptr<Entity>(target));
    return canSee;
}