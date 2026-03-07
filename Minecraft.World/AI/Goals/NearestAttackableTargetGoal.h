#pragma once

#include "TargetGoal.h"

class NearestAttackableTargetGoal : public TargetGoal
{
public:
	class DistComp
	{
	private:
		Entity *source;

	public:
		DistComp(Entity *source);

		bool operator() (std::shared_ptr<Entity> e1, std::shared_ptr<Entity> e2);
	};

private:
	std::<Mob> target;
	const std::type_info& targetType;
	int randomInterval;
	DistComp *distComp;

public:
	//public NearestAttackableTargetGoal(Mob mob, const std::type_info& targetType, float within, int randomInterval, bool mustSee)
	//{
	//	this(mob, targetType, within, randomInterval, mustSee, false);
	//}

	NearestAttackableTargetGoal(Mob *mob, const std::type_info& targetType, float within, int randomInterval, bool mustSee, bool mustReach = false);
	virtual ~NearestAttackableTargetGoal();

	virtual bool canUse();
	void start();
};