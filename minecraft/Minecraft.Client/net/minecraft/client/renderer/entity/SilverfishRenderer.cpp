#include "Minecraft.World/Header Files/stdafx.h"
#include "SilverfishRenderer.h"
#include "Minecraft.World/net/minecraft/world/entity/monster/net.minecraft.world.entity.monster.h"
#include "../../model/SilverfishModel.h"

ResourceLocation SilverfishRenderer::SILVERFISH_LOCATION(TN_MOB_SILVERFISH);

SilverfishRenderer::SilverfishRenderer()
    : MobRenderer(new SilverfishModel(), 0.3f) {}

float SilverfishRenderer::getFlipDegrees(std::shared_ptr<LivingEntity> spider) {
    return 180;
}

void SilverfishRenderer::render(std::shared_ptr<Entity> _mob, double x,
                                double y, double z, float rot, float a) {
    MobRenderer::render(_mob, x, y, z, rot, a);
}

ResourceLocation* SilverfishRenderer::getTextureLocation(
    std::shared_ptr<Entity> mob) {
    return &SILVERFISH_LOCATION;
}

int SilverfishRenderer::prepareArmor(std::shared_ptr<LivingEntity> _silverfish,
                                     int layer, float a) {
    return -1;
}