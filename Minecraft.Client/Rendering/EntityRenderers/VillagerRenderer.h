#pragma once

#include "MobRenderer.h"

class VillagerModel;

class VillagerRenderer : public MobRenderer {
protected:
    VillagerModel* villagerModel;

public:
    VillagerRenderer();

protected:
    virtual int prepareArmor(std::shared_ptr<Mob> villager, int layer, float a);

public:
    virtual void render(std::shared_ptr<Entity> mob, double x, double y,
                        double z, float rot, float a);

protected:
    virtual void renderName(std::shared_ptr<Mob> mob, double x, double y,
                            double z);
    virtual void additionalRendering(std::shared_ptr<Mob> mob, float a);
    virtual void scale(std::shared_ptr<Mob> player, float a);
};