#include "../../Build/stdafx.h"
#include "../../Headers/net.minecraft.world.entity.h"
#include "../../Headers/net.minecraft.world.entity.animal.h"
#include "OwnerHurtTargetGoal.h"

OwnerHurtTargetGoal::OwnerHurtTargetGoal(TamableAnimal *tameAnimal) : TargetGoal(tameAnimal, 32, false)
{
	this->tameAnimal = tameAnimal;
	setRequiredControlFlags(TargetGoal::TargetFlag);
}

bool OwnerHurtTargetGoal::canUse()
{
	if (!tameAnimal->isTame()) return false;
	std::shared_ptr<Mob> owner = tameAnimal->getOwner();
	if (owner == NULL) return false;
	ownerLastHurt = weak_ptr<Mob>(owner->getLastHurtMob());
	return canAttack(ownerLastHurt.lock(), false);
}

void OwnerHurtTargetGoal::start()
{
	mob->setTarget(ownerLastHurt.lock());
	TargetGoal::start();
}