#include "../../Build/stdafx.h"
#include "../../Headers/net.minecraft.world.entity.npc.h"
#include "../../Headers/net.minecraft.world.entity.ai.control.h"
#include "../../Headers/net.minecraft.world.entity.ai.navigation.h"
#include "../../Headers/net.minecraft.world.inventory.h"
#include "TradeWithPlayerGoal.h"

TradeWithPlayerGoal::TradeWithPlayerGoal(Villager *mob)
{
	this->mob = mob;
	setRequiredControlFlags(Control::JumpControlFlag | Control::MoveControlFlag);
}

bool TradeWithPlayerGoal::canUse()
{
	if (!mob->isAlive()) return false;
	if (mob->isInWater()) return false;
	if (!mob->onGround) return false;
	if (mob->hurtMarked) return false;

	shared_ptr<Player> trader = mob->getTradingPlayer();
	if (trader == NULL)
	{
		// no interaction
		return false;
	}

	if (mob->distanceToSqr(trader) > (4 * 4))
	{
		// too far away
		return false;
	}

	if (!(trader->containerMenu == trader->inventoryMenu))
	{
		// closed container
		return false;
	}

	return true;
}

void TradeWithPlayerGoal::start()
{
	mob->getNavigation()->stop();
}

void TradeWithPlayerGoal::stop()
{
	mob->setTradingPlayer(nullptr);
}