#include "Slime.h"

#include <math.h>
#include <stdint.h>

#include <numbers>
#include <string>

#include "java/Random.h"
#include "minecraft/core/particles/ParticleTypes.h"
#include "minecraft/sounds/SoundTypes.h"
#include "minecraft/util/Mth.h"
#include "minecraft/world/Difficulty.h"
#include "minecraft/world/damageSource/DamageSource.h"
#include "minecraft/world/entity/Mob.h"
#include "minecraft/world/entity/SyncedEntityData.h"
#include "minecraft/world/entity/ai/attributes/AttributeInstance.h"
#include "minecraft/world/entity/monster/SharedMonsterAttributes.h"
#include "minecraft/world/entity/player/Player.h"
#include "minecraft/world/item/Item.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/LevelType.h"
#include "minecraft/world/level/biome/Biome.h"
#include "minecraft/world/level/chunk/LevelChunk.h"
#include "minecraft/world/level/storage/LevelData.h"
#include "minecraft/world/phys/AABB.h"
#include "nbt/CompoundTag.h"

void Slime::_init() {
    jumpDelay = 0;

    targetSquish = 0;
    squish = 0;
    oSquish = 0;
}

Slime::Slime(Level* level) : Mob(level) {
    
    
    this->defineSynchedData();
    registerAttributes();
    setHealth(getMaxHealth());

    _init();

    int size = 1 << (random->nextInt(3));
    heightOffset = 0;
    jumpDelay = random->nextInt(20) + 10;
    setSize(size);
}

void Slime::defineSynchedData() {
    Mob::defineSynchedData();

    entityData->define(ID_SIZE, (uint8_t)1);
}

void Slime::setSize(int size) {
    entityData->set(ID_SIZE, (uint8_t)size);
    setSize(0.6f * size, 0.6f * size);
    setPos(x, y, z);
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)
        ->setBaseValue(size * size);
    setHealth(getMaxHealth());
    xpReward = size;
}

int Slime::getSize() { return entityData->getByte(ID_SIZE); }

void Slime::addAdditonalSaveData(CompoundTag* tag) {
    Mob::addAdditonalSaveData(tag);
    tag->putInt(L"Size", getSize() - 1);
}

void Slime::readAdditionalSaveData(CompoundTag* tag) {
    Mob::readAdditionalSaveData(tag);
    setSize(tag->getInt(L"Size") + 1);
}

ePARTICLE_TYPE Slime::getParticleName() { return eParticleType_slime; }

int Slime::getSquishSound() {
    return getSize() > 1 ? eSoundType_MOB_SLIME_BIG : eSoundType_MOB_SLIME;
}

void Slime::tick() {
    if (!level->isClientSide && level->difficulty == Difficulty::PEACEFUL &&
        getSize() > 0) {
        removed = true;
    }

    squish = squish + (targetSquish - squish) * .5f;

    oSquish = squish;
    bool wasOnGround = onGround;
    Mob::tick();
    if (onGround && !wasOnGround) {
        int size = getSize();
        for (int i = 0; i < size * 8; i++) {
            float dir = random->nextFloat() * std::numbers::pi * 2;
            float d = random->nextFloat() * 0.5f + 0.5f;
            float xd = sinf(dir) * size * 0.5f * d;
            float zd = cosf(dir) * size * 0.5f * d;
            level->addParticle(getParticleName(), x + xd, bb.y0, z + zd, 0, 0,
                               0);
        }

        if (doPlayLandSound()) {
            playSound(
                getSquishSound(), getSoundVolume(),
                ((random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f) /
                    0.8f);
        }
        targetSquish = -0.5f;
    }
    
    else if (!onGround && wasOnGround) {
        targetSquish = 1;
    }
    decreaseSquish();

    if (level->isClientSide) {
        int size = getSize();
        setSize(0.6f * size, 0.6f * size);
    }
}

void Slime::serverAiStep() {
    checkDespawn();
    std::shared_ptr<Player> player =
        level->getNearestAttackablePlayer(shared_from_this(), 16);
    if (player != nullptr) {
        lookAt(player, 10, 20);
    }
    if (onGround && jumpDelay-- <= 0) {
        jumpDelay = getJumpDelay();
        if (player != nullptr) {
            jumpDelay /= 3;
        }
        jumping = true;
        if (doPlayJumpSound()) {
            playSound(
                getSquishSound(), getSoundVolume(),
                ((random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f) *
                    0.8f);
        }

        
        
        xxa = 1 - random->nextFloat() * 2;
        yya = (float)1 * getSize();
    } else {
        jumping = false;
        if (onGround) {
            xxa = yya = 0;
        }
    }
}

void Slime::decreaseSquish() { targetSquish = targetSquish * 0.6f; }

int Slime::getJumpDelay() { return random->nextInt(20) + 10; }

std::shared_ptr<Slime> Slime::createChild() {
    return std::make_shared<Slime>(level);
}

void Slime::remove() {
    int size = getSize();
    if (!level->isClientSide && size > 1 && getHealth() <= 0) {
        int count = 2 + random->nextInt(3);
        for (int i = 0; i < count; i++) {
            
            
            
            
            
            if (i == 0 || level->countInstanceOf(eTYPE_SLIME, true) < 35) {
                float xd = (i % 2 - 0.5f) * size / 4.0f;
                float zd = (i / 2 - 0.5f) * size / 4.0f;
                std::shared_ptr<Slime> slime = createChild();
                slime->setSize(size / 2);
                slime->moveTo(x + xd, y + 0.5, z + zd,
                              random->nextFloat() * 360, 0);
                level->addEntity(slime);
            }
        }
    }
    Mob::remove();
}

void Slime::playerTouch(std::shared_ptr<Player> player) {
    if (isDealsDamage()) {
        int size = getSize();
        if (canSee(player) &&
            distanceToSqr(player) < (0.6 * size) * (0.6 * size)) {
            DamageSource* damageSource = DamageSource::mobAttack(
                std::dynamic_pointer_cast<Mob>(shared_from_this()));
            if (player->hurt(damageSource, getAttackDamage())) {
                playSound(
                    eSoundType_MOB_SLIME_ATTACK, 1,
                    (random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f);
            }
            delete damageSource;
        }
    }
}

bool Slime::isDealsDamage() { return getSize() > 1; }

int Slime::getAttackDamage() { return getSize(); }

int Slime::getHurtSound() {
    return getSize() > 1 ? eSoundType_MOB_SLIME_BIG : eSoundType_MOB_SLIME;
}

int Slime::getDeathSound() {
    return getSize() > 1 ? eSoundType_MOB_SLIME_BIG : eSoundType_MOB_SLIME;
}

int Slime::getDeathLoot() {
    if (getSize() == 1) return Item::slimeBall->id;
    return 0;
}

bool Slime::canSpawn() {
    LevelChunk* lc = level->getChunkAt(Mth::floor(x), Mth::floor(z));
    if (level->getLevelData()->getGenerator() == LevelType::lvl_flat &&
        random->nextInt(4) != 1) {
        return false;
    }
    Random* lcr =
        lc->getRandom(987234911l);  
    if ((getSize() == 1 || level->difficulty > Difficulty::PEACEFUL)) {
        
        Biome* biome = level->getBiome(Mth::floor(x), Mth::floor(z));

        if (biome == Biome::swampland && y > 50 && y < 70 &&
            random->nextFloat() < 0.5f) {
            if (random->nextFloat() < level->getMoonBrightness() &&
                level->getRawBrightness(Mth::floor(x), Mth::floor(y),
                                        Mth::floor(z)) <= random->nextInt(8)) {
                return Mob::canSpawn();
            }
        }
        if (random->nextInt(10) == 0 && lcr->nextInt(10) == 0 && y < 40) {
            return Mob::canSpawn();
        }
    }

    delete lcr;
    return false;
}

float Slime::getSoundVolume() { return 0.4f * getSize(); }

int Slime::getMaxHeadXRot() { return 0; }

bool Slime::doPlayJumpSound() { return getSize() > 0; }

bool Slime::doPlayLandSound() { return getSize() > 2; }
