#include "../../Build/stdafx.h"
#include "../../Headers/net.minecraft.world.entity.player.h"
#include "../../Headers/net.minecraft.world.entity.npc.h"
#include "LookAtTradingPlayerGoal.h"

LookAtTradingPlayerGoal::LookAtTradingPlayerGoal(Villager *villager) : LookAtPlayerGoal((Mob *)villager, typeid(Player), 8)
{
	this->villager = villager;
}

bool LookAtTradingPlayerGoal::canUse()
{
	if (villager->isTrading())
	{
		lookAt = std::<Entity>(dynamic_pointer_cast<Entity>(villager->getTradingPlayer()));
		return true;
	}
	return false;
}