#include "../../Platform/stdafx.h"
#include "../../Headers/net.minecraft.world.entity.ai.control.h"
#include "../../Headers/net.minecraft.world.entity.ai.navigation.h"
#include "../../Headers/net.minecraft.world.entity.ai.sensing.h"
#include "../../Headers/net.minecraft.world.entity.ai.util.h"
#include "../../Headers/net.minecraft.world.entity.h"
#include "../../Headers/net.minecraft.world.entity.animal.h"
#include "../../Headers/net.minecraft.world.level.h"
#include "../../Headers/net.minecraft.world.level.pathfinder.h"
#include "../../Headers/net.minecraft.world.phys.h"
#include "AvoidPlayerGoal.h"

AvoidPlayerGoalEntitySelector::AvoidPlayerGoalEntitySelector(
    AvoidPlayerGoal* parent) {
    m_parent = parent;
}

bool AvoidPlayerGoalEntitySelector::matches(
    std::shared_ptr<Entity> entity) const {
    return entity->isAlive() && m_parent->mob->getSensing()->canSee(entity);
}

AvoidPlayerGoal::AvoidPlayerGoal(PathfinderMob* mob,
                                 const std::type_info& avoidType, float maxDist,
                                 double walkSpeedModifier,
                                 double sprintSpeedModifier)
    : avoidType(avoidType) {
    this->mob = mob;
    // this->avoidType = avoidType;
    this->maxDist = maxDist;
    this->walkSpeedModifier = walkSpeedModifier;
    this->sprintSpeedModifier = sprintSpeedModifier;
    this->pathNav = mob->getNavigation();
    setRequiredControlFlags(Control::MoveControlFlag);

    entitySelector = new AvoidPlayerGoalEntitySelector(this);

    toAvoid = std::weak_ptr<Entity>();
    path = nullptr;
}

AvoidPlayerGoal::~AvoidPlayerGoal() {
    if (path != nullptr) delete path;
    delete entitySelector;
}

bool AvoidPlayerGoal::canUse() {
    if (avoidType == typeid(Player)) {
        std::shared_ptr<TamableAnimal> tamableAnimal =
            std::dynamic_pointer_cast<TamableAnimal>(mob->shared_from_this());
        if (tamableAnimal != nullptr && tamableAnimal->isTame()) return false;
        toAvoid = std::weak_ptr<Entity>(
            mob->level->getNearestPlayer(mob->shared_from_this(), maxDist));
        if (toAvoid.lock() == nullptr) return false;
    } else {
        AABB grown_bb = mob->bb.grow(maxDist, 3, maxDist);
        std::vector<std::shared_ptr<Entity> >* entities =
            mob->level->getEntitiesOfClass(avoidType, &grown_bb,
                                           entitySelector);
        if (entities->empty()) {
            delete entities;
            return false;
        }
        toAvoid = std::weak_ptr<Entity>(entities->at(0));
        delete entities;
    }

    Vec3 avoid_pos(toAvoid.lock()->x, toAvoid.lock()->y, toAvoid.lock()->z);
    auto pos = RandomPos::getPosAvoid(
        std::dynamic_pointer_cast<PathfinderMob>(mob->shared_from_this()), 16,
        7, &avoid_pos);
    if (!pos.has_value()) return false;
    if (toAvoid.lock()->distanceToSqr(pos->x, pos->y, pos->z) <
        toAvoid.lock()->distanceToSqr(mob->shared_from_this()))
        return false;
    delete path;
    path = pathNav->createPath(pos->x, pos->y, pos->z);
    if (path == nullptr) return false;
    if (!path->endsInXZ(&*pos)) return false;
    return true;
}

bool AvoidPlayerGoal::canContinueToUse() {
    return toAvoid.lock() != nullptr && !pathNav->isDone();
}

void AvoidPlayerGoal::start() {
    pathNav->moveTo(path, walkSpeedModifier);
    path = nullptr;
}

void AvoidPlayerGoal::stop() { toAvoid = std::weak_ptr<Entity>(); }

void AvoidPlayerGoal::tick() {
    if (mob->distanceToSqr(toAvoid.lock()) < 7 * 7)
        mob->getNavigation()->setSpeedModifier(sprintSpeedModifier);
    else
        mob->getNavigation()->setSpeedModifier(walkSpeedModifier);
}
