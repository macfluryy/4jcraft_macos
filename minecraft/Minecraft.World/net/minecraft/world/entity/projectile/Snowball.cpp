#include "Snowball.h"
#include "Minecraft.World/Header Files/ParticleTypes.h"
#include "Minecraft.World/net/minecraft/world/damageSource/DamageSource.h"
#include "Minecraft.World/net/minecraft/world/entity/Entity.h"
#include "Minecraft.World/net/minecraft/world/entity/projectile/Throwable.h"
#include "Minecraft.World/net/minecraft/world/level/Level.h"
#include "Minecraft.World/net/minecraft/world/phys/HitResult.h"

class LivingEntity;

void Snowball::_init() {
    // 4J Stu - This function call had to be moved here from the Entity ctor to
    // ensure that the derived version of the function is called
    this->defineSynchedData();
}

Snowball::Snowball(Level* level) : Throwable(level) { _init(); }

Snowball::Snowball(Level* level, std::shared_ptr<LivingEntity> mob)
    : Throwable(level, mob) {
    _init();
}

Snowball::Snowball(Level* level, double x, double y, double z)
    : Throwable(level, x, y, z) {
    _init();
}

void Snowball::onHit(HitResult* res) {
    if (res->entity != nullptr) {
        int damage = 0;
        if (res->entity->instanceof(eTYPE_BLAZE)) {
            damage = 3;
        }

        DamageSource* damageSource =
            DamageSource::thrown(shared_from_this(), getOwner());
        res->entity->hurt(damageSource, damage);
        delete damageSource;
    }
    for (int i = 0; i < 8; i++)
        level->addParticle(eParticleType_snowballpoof, x, y, z, 0, 0, 0);
    if (!level->isClientSide) {
        remove();
    }
}