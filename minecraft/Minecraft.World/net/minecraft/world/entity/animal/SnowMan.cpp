#include "../../../../../Header Files/stdafx.h"
#include "../../level/net.minecraft.world.level.h"
#include "../ai/attributes/net.minecraft.world.entity.ai.attributes.h"
#include "../ai/goal/net.minecraft.world.entity.ai.goal.h"
#include "../ai/goal/target/net.minecraft.world.entity.ai.goal.target.h"
#include "../ai/navigation/net.minecraft.world.entity.ai.navigation.h"
#include "../monster/net.minecraft.world.entity.monster.h"
#include "../projectile/net.minecraft.world.entity.projectile.h"
#include "../../phys/net.minecraft.world.phys.h"
#include "../../level/tile/net.minecraft.world.level.tile.h"
#include "../../damageSource/net.minecraft.world.damagesource.h"
#include "../../level/biome/net.minecraft.world.level.biome.h"
#include "../../item/net.minecraft.world.item.h"
#include "../../../SharedConstants.h"
#include "Minecraft.Client/net/minecraft/client/renderer/Textures.h"
#include "SnowMan.h"
#include "../../../../../Header Files/SoundTypes.h"

SnowMan::SnowMan(Level* level) : Golem(level) {
    // 4J Stu - This function call had to be moved here from the Entity ctor to
    // ensure that the derived version of the function is called
    this->defineSynchedData();
    registerAttributes();
    setHealth(getMaxHealth());

    this->setSize(0.4f, 1.8f);

    getNavigation()->setAvoidWater(true);
    goalSelector.addGoal(
        1, new RangedAttackGoal(this, this, 1.25,
                                SharedConstants::TICKS_PER_SECOND * 1, 10));
    goalSelector.addGoal(2, new RandomStrollGoal(this, 1.0));
    goalSelector.addGoal(3, new LookAtPlayerGoal(this, typeid(Player), 6));
    goalSelector.addGoal(4, new RandomLookAroundGoal(this));

    targetSelector.addGoal(
        1, new NearestAttackableTargetGoal(this, typeid(Mob), 0, true, false,
                                           Enemy::ENEMY_SELECTOR));
}

bool SnowMan::useNewAi() { return true; }

void SnowMan::registerAttributes() {
    Golem::registerAttributes();

    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(4);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.2f);
}

void SnowMan::aiStep() {
    Golem::aiStep();

    if (isInWaterOrRain()) hurt(DamageSource::drown, 1);

    {
        int xx = GameMath::floor(x);
        int zz = GameMath::floor(z);
        if (level->getBiome(xx, zz)->getTemperature() > 1) {
            hurt(DamageSource::onFire, 1);
        }
    }

    for (int i = 0; i < 4; i++) {
        int xx = GameMath::floor(x + (i % 2 * 2 - 1) * 0.25f);
        int yy = GameMath::floor(y);
        int zz = GameMath::floor(z + ((i / 2) % 2 * 2 - 1) * 0.25f);
        if (level->getTile(xx, yy, zz) == 0) {
            if (level->getBiome(xx, zz)->getTemperature() < 0.8f) {
                if (Tile::topSnow->mayPlace(level, xx, yy, zz)) {
                    level->setTileAndUpdate(xx, yy, zz, Tile::topSnow_Id);
                }
            }
        }
    }
}

int SnowMan::getDeathLoot() { return Item::snowBall_Id; }

void SnowMan::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel) {
    // drop some feathers
    int count = random->nextInt(16);
    for (int i = 0; i < count; i++) {
        spawnAtLocation(Item::snowBall_Id, 1);
    }
}

void SnowMan::performRangedAttack(std::shared_ptr<LivingEntity> target,
                                  float power) {
    std::shared_ptr<Snowball> snowball = std::shared_ptr<Snowball>(new Snowball(
        level, std::dynamic_pointer_cast<LivingEntity>(shared_from_this())));
    double xd = target->x - x;
    double yd = (target->y + target->getHeadHeight() - 1.1f) - snowball->y;
    double zd = target->z - z;
    float yo = GameMath::sqrt(xd * xd + zd * zd) * 0.2f;
    snowball->shoot(xd, yd + yo, zd, 1.60f, 12);

    playSound(eSoundType_RANDOM_BOW, 1.0f,
              1 / (getRandom()->nextFloat() * 0.4f + 0.8f));
    level->addEntity(snowball);
}