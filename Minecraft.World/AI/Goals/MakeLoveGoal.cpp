#include "../../Platform/stdafx.h"
#include "../../Headers/net.minecraft.world.entity.ai.control.h"
#include "../../Headers/net.minecraft.world.entity.ai.village.h"
#include "../../Headers/net.minecraft.world.entity.ai.navigation.h"
#include "../../Headers/net.minecraft.world.entity.npc.h"
#include "../../Headers/net.minecraft.world.entity.h"
#include "../../Headers/net.minecraft.world.level.h"
#include "../../Headers/net.minecraft.world.phys.h"
#include "MakeLoveGoal.h"

MakeLoveGoal::MakeLoveGoal(Villager *villager)
{
	village = std::weak_ptr<Village>();
	partner = std::weak_ptr<Villager>();
	loveMakingTime = 0;

	this->villager = villager;
	level = villager->level;
	setRequiredControlFlags(Control::MoveControlFlag | Control::LookControlFlag);
}

bool MakeLoveGoal::canUse()
{

	if (villager->getAge() != 0) return false;
	if (villager->getRandom()->nextInt(500) != 0) return false;

	village = level->villages->getClosestVillage(Mth::floor(villager->x), Mth::floor(villager->y), Mth::floor(villager->z), 0);
	if (village.lock() == NULL) return false;
	if (!villageNeedsMoreVillagers()) return false;

	std::shared_ptr<Entity> mate = level->getClosestEntityOfClass(typeid(Villager), villager->bb->grow(8, 3, 8), villager->shared_from_this());
	if (mate == NULL) return false;

	partner = std::weak_ptr<Villager>(std::dynamic_pointer_cast<Villager>(mate));
	if (partner.lock()->getAge() != 0) return false;

	return true;
}

void MakeLoveGoal::start()
{
	loveMakingTime = 300;
	villager->setInLove(true);
}

void MakeLoveGoal::stop()
{
	village = std::weak_ptr<Village>();
	partner = std::weak_ptr<Villager>();
	villager->setInLove(false);
}

bool MakeLoveGoal::canContinueToUse()
{
	return partner.lock() != NULL && loveMakingTime >= 0 && villageNeedsMoreVillagers() && villager->getAge() == 0;
}

void MakeLoveGoal::tick()
{
	--loveMakingTime;
	villager->getLookControl()->setLookAt(partner.lock(), 10, 30);

	if (villager->distanceToSqr(partner.lock()) > 1.5 * 1.5)
	{
		villager->getNavigation()->moveTo(partner.lock(), 0.25f);
	}
	else
	{
		if (loveMakingTime == 0 && partner.lock()->isInLove()) breed();
	}

	if (villager->getRandom()->nextInt(35) == 0) level->broadcastEntityEvent(villager->shared_from_this(), EntityEvent::LOVE_HEARTS);
}

bool MakeLoveGoal::villageNeedsMoreVillagers()
{
	std::shared_ptr<Village> _village = village.lock();
	if( _village == NULL ) return false;

	int idealSize = (int) ((float) _village->getDoorCount() * 0.35);
	// System.out.println("idealSize: " + idealSize + " pop: " +
	// village.getPopulationSize());
	return _village->getPopulationSize() < idealSize;
}

void MakeLoveGoal::breed()
{
	// 4J Stu - This sets a timer that stops these villagers from trying to breed again
	// We should do this even if breeding fails due to vilalger count to stop them continually trying to breed
	partner.lock()->setAge(5 * 60 * 20);
	villager->setAge(5 * 60 * 20);
	// 4J - added limit to number of animals that can be bred
	if(level->canCreateMore( eTYPE_VILLAGER, Level::eSpawnType_Breed) )
	{
		std::shared_ptr<Villager> child = std::shared_ptr<Villager>( new Villager(level) );
		child->setAge(-20 * 60 * 20);
		child->setProfession(villager->getRandom()->nextInt(Villager::PROFESSION_MAX));
		child->moveTo(villager->x, villager->y, villager->z, 0, 0);
		level->addEntity(child);
		level->broadcastEntityEvent(child, EntityEvent::LOVE_HEARTS);
	}
}
