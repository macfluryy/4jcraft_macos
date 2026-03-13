#include "../../Platform/stdafx.h"
#include "SilverfishRenderer.h"
#include "../../../Minecraft.World/Headers/net.minecraft.world.entity.monster.h"
#include "../Models/SilverfishModel.h"

SilverfishRenderer::SilverfishRenderer()
    : MobRenderer(new SilverfishModel(), 0.3f) {}

float SilverfishRenderer::getFlipDegrees(std::shared_ptr<Silverfish> spider) {
    return 180;
}

void SilverfishRenderer::render(std::shared_ptr<Entity> _mob, double x,
                                double y, double z, float rot, float a) {
    MobRenderer::render(_mob, x, y, z, rot, a);
}

int SilverfishRenderer::prepareArmor(std::shared_ptr<Mob> _silverfish,
                                     int layer, float a) {
    return -1;
}