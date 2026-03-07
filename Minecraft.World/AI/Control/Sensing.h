#pragma once

class Sensing
{
private:
	Mob *mob;
	std::vector<std::<Entity> > seen;
	std::vector<std::<Entity> > unseen;

public:
	Sensing(Mob *mob);

	void tick();
	bool canSee(std::shared_ptr<Entity> target);
};