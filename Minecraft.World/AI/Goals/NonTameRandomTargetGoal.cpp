#include "../../Platform/stdafx.h"
#include "../../Headers/net.minecraft.world.entity.animal.h"
#include "NonTameRandomTargetGoal.h"

NonTameRandomTargetGoal::NonTameRandomTargetGoal(TamableAnimal *mob, const std::type_info& targetType, float within, int randomInterval, bool mustSee) : NearestAttackableTargetGoal(mob, targetType, within, randomInterval, mustSee)
{
	this->tamableMob = mob;
}

bool NonTameRandomTargetGoal::canUse()
{
	if (tamableMob->isTame()) return false;
	return NearestAttackableTargetGoal::canUse();
}
