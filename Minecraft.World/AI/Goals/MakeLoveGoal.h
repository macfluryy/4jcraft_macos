#pragma once

#include "Goal.h"

class Villager;
class Village;

class MakeLoveGoal : public Goal
{
private:
	Villager *villager; // Owner of this goal
	std::<Villager> partner;
	Level *level;
	int loveMakingTime;
	std::<Village> village;

public:
	MakeLoveGoal(Villager *villager);

	bool canUse();
	void start();
	void stop();
	bool canContinueToUse();
	void tick();

private:
	bool villageNeedsMoreVillagers();
	void breed();

public:	
	// 4J Added override to update ai elements when loading entity from schematics
	virtual void setLevel(Level *level) { this->level = level; }
};