#pragma once
#include "MobRenderer.h"

class GhastRenderer : public MobRenderer {
public:
    GhastRenderer();

protected:
    virtual void scale(std::shared_ptr<Mob> mob, float a);
};