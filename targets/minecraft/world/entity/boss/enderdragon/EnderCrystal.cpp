#include "EnderCrystal.h"

#include <memory>
#include <vector>

#include "java/Random.h"
#include "minecraft/util/Mth.h"
#include "minecraft/world/damageSource/DamageSource.h"
#include "minecraft/world/entity/Entity.h"
#include "minecraft/world/entity/SyncedEntityData.h"
#include "minecraft/world/entity/boss/enderdragon/EnderDragon.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/tile/Tile.h"

void EnderCrystal::_init(Level* level) {
    
    
    this->defineSynchedData();

    blocksBuilding = true;
    setSize(2.0f, 2.0f);
    heightOffset = bbHeight / 2.0f;
    life = MAX_LIFE;

    time = random->nextInt(100000);
}

EnderCrystal::EnderCrystal(Level* level) : Entity(level) { _init(level); }

EnderCrystal::EnderCrystal(Level* level, double x, double y, double z)
    : Entity(level) {
    _init(level);
    setPos(x, y, z);
}

bool EnderCrystal::makeStepSound() { return false; }

void EnderCrystal::defineSynchedData() {
    entityData->define(DATA_REMAINING_LIFE, life);
}

void EnderCrystal::tick() {
    xo = x;
    yo = y;
    zo = z;
    time++;

    entityData->set(DATA_REMAINING_LIFE, life);

    
    
    
    
    if (!level->isClientSide) {
        int xt = Mth::floor(x);
        int yt = Mth::floor(y);
        int zt = Mth::floor(z);
        if (level->getTile(xt, yt, zt) != Tile::fire_Id) {
            level->setTileAndUpdate(xt, yt, zt, Tile::fire_Id);
        }
    }
}

void EnderCrystal::addAdditonalSaveData(CompoundTag* tag) {}

void EnderCrystal::readAdditionalSaveData(CompoundTag* tag) {}

float EnderCrystal::getShadowHeightOffs() { return 0; }

bool EnderCrystal::isPickable() { return true; }

bool EnderCrystal::hurt(DamageSource* source, float damage) {
    if (isInvulnerable()) return false;

    
    
    if (source->getEntity() != nullptr &&
        source->getEntity()->instanceof(eTYPE_ENDERDRAGON)) {
        return false;
    }

    if (!removed && !level->isClientSide) {
        life = 0;
        if (life <= 0) {
            remove();
            if (!level->isClientSide) {
                level->explode(nullptr, x, y, z, 6, true);

                std::vector<std::shared_ptr<Entity> > entities =
                    level->getAllEntities();
                std::shared_ptr<EnderDragon> dragon = nullptr;
                auto itEnd = entities.end();
                for (auto it = entities.begin(); it != itEnd; it++) {
                    std::shared_ptr<Entity> e = *it;  
                    dragon = std::dynamic_pointer_cast<EnderDragon>(e);
                    if (dragon != nullptr) {
                        dragon->handleCrystalDestroyed(source);
                        break;
                    }
                }
            }
        }
    }
    return true;
}