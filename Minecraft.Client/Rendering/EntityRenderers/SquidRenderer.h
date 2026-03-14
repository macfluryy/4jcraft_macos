#pragma once
#include "MobRenderer.h"

class SquidRenderer : public MobRenderer {
public:
    SquidRenderer(Model* model, float shadow);

public:
    virtual void render(std::shared_ptr<Entity> mob, double x, double y,
                        double z, float rot, float a);

protected:
    virtual void setupRotations(std::shared_ptr<Mob> _mob, float bob,
                                float bodyRot, float a);
    virtual float getBob(std::shared_ptr<Mob> _mob, float a);
};