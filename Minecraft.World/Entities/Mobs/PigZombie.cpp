#include "../../Platform/stdafx.h"
#include "../../Headers/com.mojang.nbt.h"
#include "../../Headers/net.minecraft.world.h"
#include "../../Headers/net.minecraft.world.level.h"
#include "../../Headers/net.minecraft.world.phys.h"
#include "../../Headers/net.minecraft.world.entity.player.h"
#include "../../Headers/net.minecraft.world.item.h"
#include "../../Headers/net.minecraft.world.item.enchantment.h"
#include "../../Headers/net.minecraft.world.entity.item.h"
#include "../../Headers/net.minecraft.world.damagesource.h"
#include "PigZombie.h"
#include "../../../Minecraft.Client/Textures/Textures.h"
#include "../../Util/SoundTypes.h"

std::shared_ptr<ItemInstance> PigZombie::sword;

void PigZombie::staticCtor() {
    PigZombie::sword =
        std::shared_ptr<ItemInstance>(new ItemInstance(Item::sword_gold, 1));
}

void PigZombie::_init() {
    angerTime = 0;
    playAngrySoundIn = 0;
}

PigZombie::PigZombie(Level* level) : Zombie(level) {
    // 4J Stu - This function call had to be moved here from the Entity ctor to
    // ensure that the derived version of the function is called

    // 4J Stu - Zombie has already called this, and we don't override it so the
    // Zombie one is the most derived version anyway
    // this->defineSynchedData();

    // 4J Stu - This function call had to be moved here from the Entity ctor to
    // ensure that the derived version of the function is called
    health = getMaxHealth();

    _init();

    this->textureIdx = TN_MOB_PIGZOMBIE;  // 4J was L"/mob/pigzombie.png";
    runSpeed = 0.5f;
    attackDamage = 5;
    this->fireImmune = true;
}

bool PigZombie::useNewAi() { return false; }

int PigZombie::getTexture() { return textureIdx; }

void PigZombie::tick() {
    runSpeed = attackTarget != NULL ? 0.95f : 0.5f;
    if (playAngrySoundIn > 0) {
        if (--playAngrySoundIn == 0) {
            level->playSound(
                shared_from_this(), eSoundType_MOB_ZOMBIEPIG_ZPIGANGRY,
                getSoundVolume() * 2,
                ((random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f) *
                    1.8f);
        }
    }
    Zombie::tick();
}

bool PigZombie::canSpawn() {
    return level->difficulty > Difficulty::PEACEFUL &&
           level->isUnobstructed(bb) &&
           level->getCubes(shared_from_this(), bb)->empty() &&
           !level->containsAnyLiquid(bb);
}

void PigZombie::addAdditonalSaveData(CompoundTag* tag) {
    Zombie::addAdditonalSaveData(tag);
    tag->putShort(L"Anger", (short)angerTime);
}

void PigZombie::readAdditionalSaveData(CompoundTag* tag) {
    Zombie::readAdditionalSaveData(tag);
    angerTime = tag->getShort(L"Anger");
}

std::shared_ptr<Entity> PigZombie::findAttackTarget() {
#ifndef _FINAL_BUILD
#ifdef _DEBUG_MENUS_ENABLED
    if (app.GetMobsDontAttackEnabled()) {
        return std::shared_ptr<Player>();
    }
#endif
#endif

    if (angerTime == 0) return nullptr;
    return Zombie::findAttackTarget();
}

bool PigZombie::hurt(DamageSource* source, int dmg) {
    std::shared_ptr<Entity> sourceEntity = source->getEntity();
    if (std::dynamic_pointer_cast<Player>(sourceEntity) != NULL) {
        std::vector<std::shared_ptr<Entity> >* nearby =
            level->getEntities(shared_from_this(), bb->grow(32, 32, 32));
        AUTO_VAR(itEnd, nearby->end());
        for (AUTO_VAR(it, nearby->begin()); it != itEnd; it++) {
            std::shared_ptr<Entity> e = *it;  // nearby->at(i);
            if (std::dynamic_pointer_cast<PigZombie>(e) != NULL) {
                std::shared_ptr<PigZombie> pigZombie =
                    std::dynamic_pointer_cast<PigZombie>(e);
                pigZombie->alert(sourceEntity);
            }
        }
        alert(sourceEntity);
    }
    return Zombie::hurt(source, dmg);
}

void PigZombie::alert(std::shared_ptr<Entity> target) {
    this->attackTarget = target;
    angerTime = 20 * 20 + random->nextInt(20 * 20);
    playAngrySoundIn = random->nextInt(20 * 2);
}

int PigZombie::getAmbientSound() { return eSoundType_MOB_ZOMBIEPIG_AMBIENT; }

int PigZombie::getHurtSound() { return eSoundType_MOB_ZOMBIEPIG_HURT; }

int PigZombie::getDeathSound() { return eSoundType_MOB_ZOMBIEPIG_DEATH; }

void PigZombie::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel) {
    int count = random->nextInt(2 + playerBonusLevel);
    for (int i = 0; i < count; i++) {
        spawnAtLocation(Item::rotten_flesh_Id, 1);
    }
    count = random->nextInt(2 + playerBonusLevel);
    for (int i = 0; i < count; i++) {
        spawnAtLocation(Item::goldNugget_Id, 1);
    }
}

void PigZombie::dropRareDeathLoot(int rareLootLevel) {
    if (rareLootLevel > 0) {
        std::shared_ptr<ItemInstance> sword =
            std::shared_ptr<ItemInstance>(new ItemInstance(Item::sword_gold));
        EnchantmentHelper::enchantItem(random, sword, 5);
        spawnAtLocation(sword, 0);
    } else {
        int select = random->nextInt(3);
        if (select == 0) {
            spawnAtLocation(Item::goldIngot_Id, 1);
        } else if (select == 1) {
            spawnAtLocation(Item::sword_gold_Id, 1);
        } else if (select == 2) {
            spawnAtLocation(Item::helmet_gold_Id, 1);
        }
    }
}

int PigZombie::getDeathLoot() { return Item::rotten_flesh_Id; }

void PigZombie::finalizeMobSpawn() {
    Zombie::finalizeMobSpawn();
    setVillager(false);
}

std::shared_ptr<ItemInstance> PigZombie::getCarriedItem() {
    // TODO 4J - could be of const std::shared_ptr<ItemInstance>  type.
    return (std::shared_ptr<ItemInstance>)sword;
}
