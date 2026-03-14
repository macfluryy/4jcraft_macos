#pragma once
#include "MobRenderer.h"

class ChickenRenderer : public MobRenderer {
public:
    ChickenRenderer(Model* model, float shadow);
    virtual void render(std::shared_ptr<Entity> _mob, double x, double y,
                        double z, float rot, float a);

protected:
    virtual float getBob(std::shared_ptr<Mob> _mob, float a);
};