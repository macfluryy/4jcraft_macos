#pragma once
#include "MobRenderer.h"

class CreeperRenderer: public MobRenderer
{
private:
	Model *armorModel;

public:
	CreeperRenderer();
protected:
	virtual void scale(std::shared_ptr<Mob> _mob, float a);
    virtual int getOverlayColor(std::shared_ptr<Mob> mob, float br, float a);
    virtual int prepareArmor(std::shared_ptr<Mob> mob, int layer, float a);
    virtual int prepareArmorOverlay(std::shared_ptr<Mob> _mob, int layer, float a);
};