#pragma once
#include "MobRenderer.h"

class WolfRenderer : public MobRenderer
{
public:
	WolfRenderer(Model *model, Model *armor, float shadow);
protected:
    virtual float getBob(std::shared_ptr<Mob> _mob, float a);
	virtual int prepareArmor(std::shared_ptr<Mob> mob, int layer, float a);
};
