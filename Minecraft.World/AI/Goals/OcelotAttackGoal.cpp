#include "../../Platform/stdafx.h"
#include "../../Headers/net.minecraft.world.entity.ai.control.h"
#include "../../Headers/net.minecraft.world.entity.ai.navigation.h"
#include "../../Headers/net.minecraft.world.entity.h"
#include "../../Headers/net.minecraft.world.entity.animal.h"
#include "../../Headers/net.minecraft.world.phys.h"
#include "OcelotAttackGoal.h"

OzelotAttackGoal::OzelotAttackGoal(Mob *mob)
{
	target = std::weak_ptr<Mob>();
	attackTime = 0;
	speed = 0;
	trackTarget = false;

	this->mob = mob;
	this->level = mob->level;
	setRequiredControlFlags(Control::MoveControlFlag | Control::LookControlFlag);
}

bool OzelotAttackGoal::canUse()
{
	std::shared_ptr<Mob> bestTarget = mob->getTarget();
	if (bestTarget == NULL) return false;
	target = std::weak_ptr<Mob>(bestTarget);
	return true;
}

bool OzelotAttackGoal::canContinueToUse()
{
	if (target.lock() == NULL || !target.lock()->isAlive()) return false;
	if (mob->distanceToSqr(target.lock()) > 15 * 15) return false;
	return !mob->getNavigation()->isDone() || canUse();
}

void OzelotAttackGoal::stop()
{
	target = std::weak_ptr<Mob>();
	mob->getNavigation()->stop();
}

void OzelotAttackGoal::tick()
{
	mob->getLookControl()->setLookAt(target.lock(), 30, 30);

	double meleeRadiusSqr = (mob->bbWidth * 2) * (mob->bbWidth * 2);
	double distSqr = mob->distanceToSqr(target.lock()->x, target.lock()->bb->y0, target.lock()->z);

	float speed = Ozelot::WALK_SPEED;
	if (distSqr > meleeRadiusSqr && distSqr < 4 * 4) speed = Ozelot::SPRINT_SPEED;
	else if (distSqr < 15 * 15) speed = Ozelot::SNEAK_SPEED;

	mob->getNavigation()->moveTo(target.lock(), speed);

	attackTime = std::max(attackTime - 1, 0);

	if (distSqr > meleeRadiusSqr) return;
	if (attackTime > 0) return;
	attackTime = 20;
	mob->doHurtTarget(target.lock());
}