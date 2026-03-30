#include "../../Platform/stdafx.h"
#include "../../Headers/net.minecraft.world.entity.ai.control.h"
#include "../../Headers/net.minecraft.world.entity.npc.h"
#include "../../Headers/net.minecraft.world.entity.animal.h"
#include "../../Headers/net.minecraft.world.level.h"
#include "../../Headers/net.minecraft.world.phys.h"
#include "OfferFlowerGoal.h"

OfferFlowerGoal::OfferFlowerGoal(VillagerGolem* golem) {
    this->golem = golem;
    setRequiredControlFlags(Control::MoveControlFlag |
                            Control::LookControlFlag);
}

bool OfferFlowerGoal::canUse() {
    if (!golem->level->isDay()) return false;
    if (golem->getRandom()->nextInt(8000) != 0) return false;
    AABB golem_bb = golem->bb.grow(6, 2, 6);
    villager = std::weak_ptr<Villager>(std::dynamic_pointer_cast<Villager>(
        golem->level->getClosestEntityOfClass(typeid(Villager), &golem_bb,
                                              golem->shared_from_this())));
    return villager.lock() != NULL;
}

bool OfferFlowerGoal::canContinueToUse() {
    return _tick > 0 && villager.lock() != NULL;
}

void OfferFlowerGoal::start() {
    _tick = OFFER_TICKS;
    golem->offerFlower(true);
}

void OfferFlowerGoal::stop() {
    golem->offerFlower(false);
    villager = std::weak_ptr<Villager>();
}

void OfferFlowerGoal::tick() {
    golem->getLookControl()->setLookAt(villager.lock(), 30, 30);
    --_tick;
}
