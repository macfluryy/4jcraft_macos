#pragma once

#include "MobRenderer.h"

class VillagerGolemModel;

class VillagerGolemRenderer : public MobRenderer
{
private:
	VillagerGolemModel *golemModel;

public:
	VillagerGolemRenderer();

protected:
	int prepareArmor(VillagerGolemModel *villagerGolem, int layer, float a);

public:
	void render(std::shared_ptr<Entity> mob, double x, double y, double z, float rot, float a);

protected:
	void setupRotations(std::shared_ptr<Mob> _mob, float bob, float bodyRot, float a);
	void additionalRendering(std::shared_ptr<Mob> mob, float a);
};