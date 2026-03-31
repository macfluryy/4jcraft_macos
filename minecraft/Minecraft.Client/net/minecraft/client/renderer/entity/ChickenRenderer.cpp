#include "Minecraft.World/Header Files/stdafx.h"
#include "Minecraft.World/net/minecraft/util/GameMath.h"
#include "ChickenRenderer.h"
#include "Minecraft.World/net/minecraft/world/entity/animal/net.minecraft.world.entity.animal.h"

ResourceLocation ChickenRenderer::CHICKEN_LOCATION =
    ResourceLocation(TN_MOB_CHICKEN);

ChickenRenderer::ChickenRenderer(Model* model, float shadow)
    : MobRenderer(model, shadow) {}

void ChickenRenderer::render(std::shared_ptr<Entity> _mob, double x, double y,
                             double z, float rot, float a) {
    MobRenderer::render(_mob, x, y, z, rot, a);
}

float ChickenRenderer::getBob(std::shared_ptr<LivingEntity> _mob, float a) {
    // 4J - dynamic cast required because we aren't using templates/generics in
    // our version
    std::shared_ptr<Chicken> mob = std::dynamic_pointer_cast<Chicken>(_mob);

    float flap = mob->oFlap + (mob->flap - mob->oFlap) * a;
    float flapSpeed = mob->oFlapSpeed + (mob->flapSpeed - mob->oFlapSpeed) * a;

    return (sinf(flap) + 1) * flapSpeed;
}

ResourceLocation* ChickenRenderer::getTextureLocation(
    std::shared_ptr<Entity> mob) {
    return &CHICKEN_LOCATION;
}