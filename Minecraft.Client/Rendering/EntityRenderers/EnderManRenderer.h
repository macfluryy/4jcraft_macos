#pragma once

#include "MobRenderer.h"

class EnderMan;
class EndermanModel;

class EndermanRenderer : public MobRenderer
{
private:
	EndermanModel *model;
	Random random;

public:
	EndermanRenderer();

	virtual void render(std::shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a);
	virtual void additionalRendering(std::shared_ptr<Mob> _mob, float a);

protected:
	virtual int prepareArmor(std::shared_ptr<Mob> _mob, int layer, float a);
};