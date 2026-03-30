#include "../Platform/stdafx.h"
#include "../Headers/net.minecraft.world.entity.ai.goal.h"
#include "../Headers/net.minecraft.world.entity.h"
#include "../Headers/net.minecraft.world.level.h"
#include "SyncedEntityData.h"
#include "../Util/ParticleTypes.h"
#include "TamableAnimal.h"

TamableAnimal::TamableAnimal(Level* level) : Animal(level) {
    sitGoal = new SitGoal(this);
}

TamableAnimal::~TamableAnimal() {
    if (sitGoal != NULL) delete sitGoal;
}

void TamableAnimal::defineSynchedData() {
    Animal::defineSynchedData();
    entityData->define(DATA_FLAGS_ID, (uint8_t)0);
    entityData->define(DATA_OWNERUUID_ID, L"");
}

void TamableAnimal::addAdditonalSaveData(CompoundTag* tag) {
    Animal::addAdditonalSaveData(tag);
    if (getOwnerUUID().empty()) {
        tag->putString(L"Owner", L"");
    } else {
        tag->putString(L"Owner", getOwnerUUID());
    }
    tag->putBoolean(L"Sitting", isSitting());
}

void TamableAnimal::readAdditionalSaveData(CompoundTag* tag) {
    Animal::readAdditionalSaveData(tag);
    std::wstring owner = tag->getString(L"Owner");
    if (owner.length() > 0) {
        setOwnerUUID(owner);
        setTame(true);
    }
    sitGoal->wantToSit(tag->getBoolean(L"Sitting"));
    setSitting(tag->getBoolean(L"Sitting"));
}

void TamableAnimal::spawnTamingParticles(bool success) {
    ePARTICLE_TYPE particle = eParticleType_heart;
    if (!success) {
        particle = eParticleType_smoke;
    }
    for (int i = 0; i < 7; i++) {
        double xa = random->nextGaussian() * 0.02;
        double ya = random->nextGaussian() * 0.02;
        double za = random->nextGaussian() * 0.02;
        level->addParticle(
            particle, x + random->nextFloat() * bbWidth * 2 - bbWidth,
            y + .5f + random->nextFloat() * bbHeight,
            z + random->nextFloat() * bbWidth * 2 - bbWidth, xa, ya, za);
    }
}

void TamableAnimal::handleEntityEvent(uint8_t id) {
    if (id == EntityEvent::TAMING_SUCCEEDED) {
        spawnTamingParticles(true);
    } else if (id == EntityEvent::TAMING_FAILED) {
        spawnTamingParticles(false);
    } else {
        Animal::handleEntityEvent(id);
    }
}

bool TamableAnimal::isTame() {
    return (entityData->getByte(DATA_FLAGS_ID) & 0x04) != 0;
}

void TamableAnimal::setTame(bool value) {
    uint8_t current = entityData->getByte(DATA_FLAGS_ID);
    if (value) {
        entityData->set(DATA_FLAGS_ID, (uint8_t)(current | 0x04));
    } else {
        entityData->set(DATA_FLAGS_ID, (uint8_t)(current & ~0x04));
    }
}

bool TamableAnimal::isSitting() {
    return (entityData->getByte(DATA_FLAGS_ID) & 0x01) != 0;
}

void TamableAnimal::setSitting(bool value) {
    uint8_t current = entityData->getByte(DATA_FLAGS_ID);
    if (value) {
        entityData->set(DATA_FLAGS_ID, (uint8_t)(current | 0x01));
    } else {
        entityData->set(DATA_FLAGS_ID, (uint8_t)(current & ~0x01));
    }
}

std::wstring TamableAnimal::getOwnerUUID() {
    return entityData->getString(DATA_OWNERUUID_ID);
}

void TamableAnimal::setOwnerUUID(const std::wstring& name) {
    entityData->set(DATA_OWNERUUID_ID, name);
}

std::shared_ptr<Entity> TamableAnimal::getOwner() {
    return level->getPlayerByUUID(getOwnerUUID());
}

SitGoal* TamableAnimal::getSitGoal() { return sitGoal; }

bool TamableAnimal::wantsToAttack(std::shared_ptr<LivingEntity> target,
                                  std::shared_ptr<LivingEntity> owner) {
    return true;
}

Team* TamableAnimal::getTeam() {
    if (isTame()) {
        std::shared_ptr<LivingEntity> owner =
            std::dynamic_pointer_cast<LivingEntity>(getOwner());
        if (owner != NULL) {
            return owner->getTeam();
        }
    }
    return Animal::getTeam();
}

bool TamableAnimal::isAlliedTo(std::shared_ptr<LivingEntity> other) {
    if (isTame()) {
        std::shared_ptr<LivingEntity> owner =
            std::dynamic_pointer_cast<LivingEntity>(getOwner());
        if (other == owner) {
            return true;
        }
        if (owner != NULL) {
            return owner->isAlliedTo(other);
        }
    }
    return Animal::isAlliedTo(other);
}