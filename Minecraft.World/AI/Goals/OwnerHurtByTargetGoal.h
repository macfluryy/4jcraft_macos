#pragma once

#include "TargetGoal.h"

class TamableAnimal;

class OwnerHurtByTargetGoal : public TargetGoal
{
private:
	TamableAnimal *tameAnimal; // Owner of this goal
	std::<Mob> ownerLastHurtBy;

public:
	OwnerHurtByTargetGoal(TamableAnimal *tameAnimal);

	bool canUse();
	void start();
};