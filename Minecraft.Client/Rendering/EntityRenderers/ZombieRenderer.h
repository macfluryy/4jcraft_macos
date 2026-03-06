#pragma once

#include "HumanoidMobRenderer.h"

class VillagerZombieModel;
class Zombie;

class ZombieRenderer : public HumanoidMobRenderer
{
private:
	HumanoidModel *defaultModel;
	VillagerZombieModel *villagerModel;

protected:
	HumanoidModel *defaultArmorParts1;
	HumanoidModel *defaultArmorParts2;
	HumanoidModel *villagerArmorParts1;
	HumanoidModel *villagerArmorParts2;

private:
	int modelVersion;

public:
	ZombieRenderer();

protected:
	void createArmorParts();
	int prepareArmor(std::shared_ptr<Mob> _mob, int layer, float a);

public:
	void render(std::shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a);

protected:
	void additionalRendering(std::shared_ptr<Mob> _mob, float a);

private:
	void swapArmor(std::shared_ptr<Zombie> mob);

protected:
	void setupRotations(std::shared_ptr<Mob> _mob, float bob, float bodyRot, float a);
};