#include "../../Build/stdafx.h"
#include "../../Headers/net.minecraft.world.entity.ai.control.h"
#include "../../Headers/net.minecraft.world.entity.ai.navigation.h"
#include "../../Headers/net.minecraft.world.entity.ai.sensing.h"
#include "../../Headers/net.minecraft.world.entity.projectile.h"
#include "../../Headers/net.minecraft.world.entity.h"
#include "../../Headers/net.minecraft.world.level.h"
#include "../../Headers/net.minecraft.world.phys.h"
#include "../../Util/SoundTypes.h"
#include "ArrowAttackGoal.h"

ArrowAttackGoal::ArrowAttackGoal(Mob *mob, float speed, int projectileType, int attackInterval)
{
	// 4J Init
	target = weak_ptr<Mob>();
	attackTime = 0;
	seeTime = 0;

	this->mob = mob;
	this->level = mob->level;
	this->speed = speed;
	this->projectileType = projectileType;
	this->attackInterval = attackInterval;
	setRequiredControlFlags(Control::MoveControlFlag | Control::LookControlFlag);
}

bool ArrowAttackGoal::canUse()
{
	std::shared_ptr<Mob> bestTarget = mob->getTarget();
	if (bestTarget == NULL) return false;
	target = weak_ptr<Mob>(bestTarget);
	return true;
}

bool ArrowAttackGoal::canContinueToUse()
{
	return target.lock() != NULL && (canUse() || !mob->getNavigation()->isDone());
}

void ArrowAttackGoal::stop()
{
	target = weak_ptr<Mob>();
}

void ArrowAttackGoal::tick()
{
	double attackRadiusSqr = 10 * 10;
	std::shared_ptr<Mob> tar = target.lock();
	double targetDistSqr = mob->distanceToSqr(tar->x, tar->bb->y0, tar->z);
	bool canSee = mob->getSensing()->canSee(tar);

	if (canSee)
	{
		++seeTime;
	}
	else seeTime = 0;

	if (targetDistSqr > attackRadiusSqr || seeTime < 20) mob->getNavigation()->moveTo(tar, speed);
	else mob->getNavigation()->stop();

	mob->getLookControl()->setLookAt(tar, 30, 30);

	attackTime = max(attackTime - 1, 0);
	if (attackTime > 0) return;
	if (targetDistSqr > attackRadiusSqr || !canSee) return;
	fireAtTarget();
	attackTime = attackInterval;
}

void ArrowAttackGoal::fireAtTarget()
{
	std::shared_ptr<Mob> tar = target.lock();
	if (projectileType == ArrowType)
	{
		std::shared_ptr<Arrow> arrow = std::shared_ptr<Arrow>( new Arrow(level, dynamic_pointer_cast<Mob>(mob->shared_from_this()), tar, 1.60f, 12) );
		level->playSound(mob->shared_from_this(), eSoundType_RANDOM_BOW, 1.0f, 1 / (mob->getRandom()->nextFloat() * 0.4f + 0.8f));
		level->addEntity(arrow);
	}
	else if (projectileType == SnowballType)
	{
		std::shared_ptr<Snowball> snowball = std::shared_ptr<Snowball>( new Snowball(level, dynamic_pointer_cast<Mob>(mob->shared_from_this())) );
		double xd = tar->x - mob->x;
		double yd = (tar->y + tar->getHeadHeight() - 1.1f) - snowball->y;
		double zd = tar->z - mob->z;
		float yo = sqrt(xd * xd + zd * zd) * 0.2f;
		snowball->shoot(xd, yd + yo, zd, 1.60f, 12);

		level->playSound(mob->shared_from_this(), eSoundType_RANDOM_BOW, 1.0f, 1 / (mob->getRandom()->nextFloat() * 0.4f + 0.8f));
		level->addEntity(snowball);
	}
}