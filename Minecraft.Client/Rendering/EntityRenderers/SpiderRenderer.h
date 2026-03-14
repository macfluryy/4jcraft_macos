#pragma once
#include "MobRenderer.h"

class SpiderRenderer : public MobRenderer {
public:
    SpiderRenderer();

protected:
    virtual float getFlipDegrees(std::shared_ptr<Mob> spider);
    virtual int prepareArmor(std::shared_ptr<Mob> _spider, int layer, float a);
    void scale(std::shared_ptr<Mob> _mob, float a);
};