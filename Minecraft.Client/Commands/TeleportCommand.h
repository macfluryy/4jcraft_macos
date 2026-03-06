#pragma once

#include "../../Minecraft.World/Commands/Command.h"

class TeleportCommand : public Command
{
public:
	virtual EGameCommand getId();
	virtual void execute(std::shared_ptr<CommandSender> source, byteArray commandData);

	static std::shared_ptr<GameCommandPacket> preparePacket(PlayerUID subject, PlayerUID destination);
};