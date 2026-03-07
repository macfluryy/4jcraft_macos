#include "../Build/stdafx.h"
#include "../Headers/net.minecraft.world.h"
#include "../Headers/net.minecraft.world.phys.h"
#include "../Headers/net.minecraft.world.level.h"
#include "../Headers/net.minecraft.world.entity.player.h"
#include "../Headers/net.minecraft.world.damagesource.h"
#include "../Headers/net.minecraft.world.effect.h"
#include "../Headers/net.minecraft.world.item.enchantment.h"
#include "Monster.h"
#include "../../Minecraft.Client/Minecraft.h"



void Monster::_init()
{
	attackDamage = 2;
}

Monster::Monster(Level *level) : PathfinderMob( level )
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called

	// 4J Stu - Only the most derived classes should call this
	//this->defineSynchedData();

    _init();

	xpReward = Enemy::XP_REWARD_MEDIUM;
}

void Monster::aiStep() 
{
    float br = getBrightness(1);
    if (br > 0.5f)
	{
        noActionTime += 2;
    }

    PathfinderMob::aiStep();
}

void Monster::tick()
{
    PathfinderMob::tick();
    if (!level->isClientSide && (level->difficulty == Difficulty::PEACEFUL || Minecraft::GetInstance()->isTutorial() ) ) remove();
}

std::shared_ptr<Entity> Monster::findAttackTarget()
{
#ifndef _FINAL_BUILD
	if(app.GetMobsDontAttackEnabled())
	{
		return std::shared_ptr<Player>();
	}
#endif

    std::shared_ptr<Player> player = level->getNearestAttackablePlayer(shared_from_this(), 16);
	if (player != NULL && canSee(player) ) return player;
    return std::shared_ptr<Player>();
}

bool Monster::hurt(DamageSource *source, int dmg)
{
    if (PathfinderMob::hurt(source, dmg))
	{
		std::shared_ptr<Entity> sourceEntity = source->getEntity();
        if (rider.lock() == sourceEntity || riding == sourceEntity) return true;

        if (sourceEntity != shared_from_this()) 
		{
            this->attackTarget = sourceEntity;
        }
        return true;
    }
    return false;
}

/**
* Performs hurt action, returns if successful
* 
* @param target
* @return
*/
bool Monster::doHurtTarget(std::shared_ptr<Entity> target)
{
	int dmg = attackDamage;
	if (hasEffect(MobEffect::damageBoost))
	{
		dmg += (3 << getEffect(MobEffect::damageBoost)->getAmplifier());
	}
	if (hasEffect(MobEffect::weakness))
	{
		dmg -= (2 << getEffect(MobEffect::weakness)->getAmplifier());
	}

	DamageSource *damageSource = DamageSource::mobAttack(std::dynamic_pointer_cast<Mob>( shared_from_this() ) );
	bool didHurt = target->hurt(damageSource, dmg);
	delete damageSource;

	if (didHurt)
	{
		int fireAspect = EnchantmentHelper::getFireAspect(std::dynamic_pointer_cast<Mob>(shared_from_this()));
		if (fireAspect > 0)
		{
			target->setOnFire(fireAspect * 4);
		}

		std::shared_ptr<Mob> mob = std::dynamic_pointer_cast<Mob>(target);
		if (mob != NULL)
		{
			ThornsEnchantment::doThornsAfterAttack(shared_from_this(), mob, random);
		}
	}

	return didHurt;
}

void Monster::checkHurtTarget(std::shared_ptr<Entity> target, float distance)
{
    if (attackTime <= 0 && distance < 2.0f && target->bb->y1 > bb->y0 && target->bb->y0 < bb->y1)
	{
        attackTime = 20;
        doHurtTarget(target);
    }
}

float Monster::getWalkTargetValue(int x, int y, int z)
{
    return 0.5f - level->getBrightness(x, y, z);
}

bool Monster::isDarkEnoughToSpawn()
{
    int xt = Mth::floor(x);
    int yt = Mth::floor(bb->y0);
    int zt = Mth::floor(z);
    if (level->getBrightness(LightLayer::Sky, xt, yt, zt) > random->nextInt(32)) return false;

    int br = level->getRawBrightness(xt, yt, zt);

    if (level->isThundering()) 
	{
        int tmp = level->skyDarken;
        level->skyDarken = 10;
        br = level->getRawBrightness(xt, yt, zt);
        level->skyDarken = tmp;
    }

    return br <= random->nextInt(8);
}

bool Monster::canSpawn()
{
	// 4J Stu
	// Fix for #8265 - AI: Monsters will flash briefly on the screen around Monster Spawners when the game settings are set to Peaceful.
	return isDarkEnoughToSpawn() && PathfinderMob::canSpawn() && level->difficulty > Difficulty::PEACEFUL;
}
