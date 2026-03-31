#include <string>

#include "LargeFireball.h"
#include "nbt/CompoundTag.h"
#include "Minecraft.World/net/minecraft/world/damageSource/DamageSource.h"
#include "Minecraft.World/net/minecraft/world/entity/Entity.h"
#include "Minecraft.World/net/minecraft/world/entity/LivingEntity.h"
#include "Minecraft.World/net/minecraft/world/entity/projectile/Fireball.h"
#include "Minecraft.World/net/minecraft/world/level/GameRules.h"
#include "Minecraft.World/net/minecraft/world/level/Level.h"
#include "Minecraft.World/net/minecraft/world/phys/HitResult.h"

LargeFireball::LargeFireball(Level* level) : Fireball(level) {
    explosionPower = 1;
}

LargeFireball::LargeFireball(Level* level, double x, double y, double z,
                             double xa, double ya, double za)
    : Fireball(level, x, y, z, xa, ya, za) {
    explosionPower = 1;
}

LargeFireball::LargeFireball(Level* level, std::shared_ptr<LivingEntity> mob,
                             double xa, double ya, double za)
    : Fireball(level, mob, xa, ya, za) {
    explosionPower = 1;
}

void LargeFireball::onHit(HitResult* res) {
    if (!level->isClientSide) {
        if (res->entity != nullptr) {
            DamageSource* damageSource = DamageSource::fireball(
                std::dynamic_pointer_cast<Fireball>(shared_from_this()), owner);
            res->entity->hurt(damageSource, 6);
            delete damageSource;
        }
        level->explode(
            nullptr, x, y, z, explosionPower, true,
            level->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING));
        remove();
    }
}

void LargeFireball::addAdditonalSaveData(CompoundTag* tag) {
    Fireball::addAdditonalSaveData(tag);
    tag->putInt(L"ExplosionPower", explosionPower);
}

void LargeFireball::readAdditionalSaveData(CompoundTag* tag) {
    Fireball::readAdditionalSaveData(tag);
    if (tag->contains(L"ExplosionPower"))
        explosionPower = tag->getInt(L"ExplosionPower");
}
