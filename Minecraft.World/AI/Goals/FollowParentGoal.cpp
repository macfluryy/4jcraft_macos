#include "../../Platform/stdafx.h"
#include "../../Headers/net.minecraft.world.entity.animal.h"
#include "../../Headers/net.minecraft.world.entity.ai.navigation.h"
#include "../../Headers/net.minecraft.world.level.h"
#include "../../Headers/net.minecraft.world.phys.h"
#include "../../Util/BasicTypeContainers.h"
#include "FollowParentGoal.h"

FollowParentGoal::FollowParentGoal(Animal* animal, float speed) {
    timeToRecalcPath = 0;

    this->animal = animal;
    this->speed = speed;
}

bool FollowParentGoal::canUse() {
    if (animal->getAge() >= 0) return false;

    std::vector<std::shared_ptr<Entity> >* parents =
        animal->level->getEntitiesOfClass(typeid(*animal),
                                          animal->bb->grow(8, 4, 8));

    std::shared_ptr<Animal> closest = nullptr;
    double closestDistSqr = Double::MAX_VALUE;
    for (AUTO_VAR(it, parents->begin()); it != parents->end(); ++it) {
        std::shared_ptr<Animal> parent = std::dynamic_pointer_cast<Animal>(*it);
        if (parent->getAge() < 0) continue;
        double distSqr = animal->distanceToSqr(parent);
        if (distSqr > closestDistSqr) continue;
        closestDistSqr = distSqr;
        closest = parent;
    }
    delete parents;

    if (closest == NULL) return false;
    if (closestDistSqr < 3 * 3) return false;
    parent = std::weak_ptr<Animal>(closest);
    return true;
}

bool FollowParentGoal::canContinueToUse() {
    if (parent.lock() == NULL || !parent.lock()->isAlive()) return false;
    double distSqr = animal->distanceToSqr(parent.lock());
    if (distSqr < 3 * 3 || distSqr > 16 * 16) return false;
    return true;
}

void FollowParentGoal::start() { timeToRecalcPath = 0; }

void FollowParentGoal::stop() { parent = std::weak_ptr<Animal>(); }

void FollowParentGoal::tick() {
    if (--timeToRecalcPath > 0) return;
    timeToRecalcPath = 10;
    animal->getNavigation()->moveTo(parent.lock(), speed);
}