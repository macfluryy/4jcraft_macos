#include "AbstractProjectileDispenseBehavior.h"
#include "Minecraft.World/net/minecraft/world/level/tile/DispenserTile.h"
#include "Minecraft.World/net/minecraft/world/entity/projectile/Projectile.h"
#include "Minecraft.World/net/minecraft/world/level/Level.h"
#include "Minecraft.World/net/minecraft/world/level/tile/LevelEvent.h"
#include "java/Class.h"
#include "Minecraft.World/net/minecraft/core/BlockSource.h"
#include "Minecraft.World/net/minecraft/core/DefaultDispenseItemBehavior.h"
#include "Minecraft.World/net/minecraft/core/FacingEnum.h"
#include "Minecraft.World/net/minecraft/core/Position.h"
#include "Minecraft.World/net/minecraft/world/entity/Entity.h"
#include "Minecraft.World/net/minecraft/world/item/ItemInstance.h"

std::shared_ptr<ItemInstance> AbstractProjectileDispenseBehavior::execute(
    BlockSource* source, std::shared_ptr<ItemInstance> dispensed,
    eOUTCOME& outcome) {
    Level* world = source->getWorld();
    if (world->countInstanceOf(eTYPE_PROJECTILE, false) >=
        Level::MAX_DISPENSABLE_PROJECTILES) {
        return DefaultDispenseItemBehavior::execute(source, dispensed, outcome);
    }

    Position* position = DispenserTile::getDispensePosition(source);
    FacingEnum* facing = DispenserTile::getFacing(source->getData());

    std::shared_ptr<Projectile> projectile = getProjectile(world, position);

    delete position;

    projectile->shoot(facing->getStepX(), facing->getStepY() + .1f,
                      facing->getStepZ(), getPower(), getUncertainty());
    world->addEntity(std::dynamic_pointer_cast<Entity>(projectile));

    dispensed->remove(1);
    return dispensed;
}

void AbstractProjectileDispenseBehavior::playSound(BlockSource* source,
                                                   eOUTCOME outcome) {
    if (outcome != LEFT_ITEM) {
        source->getWorld()->levelEvent(LevelEvent::SOUND_LAUNCH,
                                       source->getBlockX(), source->getBlockY(),
                                       source->getBlockZ(), 0);
    }
}

float AbstractProjectileDispenseBehavior::getUncertainty() { return 6.0f; }

float AbstractProjectileDispenseBehavior::getPower() { return 1.1f; }
