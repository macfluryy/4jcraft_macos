#pragma once

#include "TargetGoal.h"

class TamableAnimal;

class OwnerHurtTargetGoal : public TargetGoal
{
private:
	TamableAnimal *tameAnimal; // Owner of this goal
	std::<Mob> ownerLastHurt;

public:
	OwnerHurtTargetGoal(TamableAnimal *tameAnimal);

	bool canUse();
	void start();
};