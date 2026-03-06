#include "../Build/stdafx.h"
#include "../Headers/net.minecraft.commands.h"
#include "../Headers/net.minecraft.world.entity.player.h"
#include "../Headers/net.minecraft.world.damagesource.h"
#include "KillCommand.h"

EGameCommand KillCommand::getId()
{
	return eGameCommand_Kill;
}

void KillCommand::execute(std::shared_ptr<CommandSender> source, byteArray commandData)
{
	std::shared_ptr<Player> player = dynamic_pointer_cast<Player>(source);

	player->hurt(DamageSource::outOfWorld, 1000);

	source->sendMessage(L"Ouch. That look like it hurt.");
}