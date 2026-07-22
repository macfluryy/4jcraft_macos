#pragma once
#include <memory>

#include "LivingEntityRenderer.h"

class Mob;
class Model;









class MobRenderer : public LivingEntityRenderer {
public:
    MobRenderer(Model* model, float shadow);
    virtual void render(std::shared_ptr<Entity> mob, double x, double y,
                        double z, float rot, float a);

protected:
    virtual bool shouldShowName(std::shared_ptr<LivingEntity> mob);
    virtual void renderLeash(std::shared_ptr<Mob> entity, double x, double y,
                             double z, float rot, float a);

private:
    double lerp(double prev, double next, double a);
};
