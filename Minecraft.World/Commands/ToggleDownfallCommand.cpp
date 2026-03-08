#include "../Build/stdafx.h"
#include "../../Minecraft.Client/MinecraftServer.h"
#include "../../Minecraft.Client/Level/ServerLevel.h"
#include "../Headers/net.minecraft.commands.h"
#include "../Headers/net.minecraft.world.level.h"
#include "../Headers/net.minecraft.world.level.storage.h"
#include "../Headers/net.minecraft.network.packet.h"
#include "ToggleDownfallCommand.h"

EGameCommand ToggleDownfallCommand::getId()
{
	return eGameCommand_ToggleDownfall;
}

void ToggleDownfallCommand::execute(std::shared_ptr<CommandSender> source, byteArray commandData)
{
	doToggleDownfall();
	logAdminAction(source, ChatPacket::e_ChatCustom, L"commands.downfall.success");
}

void ToggleDownfallCommand::doToggleDownfall()
{
	MinecraftServer::getInstance()->levels[0]->toggleDownfall();
	MinecraftServer::getInstance()->levels[0]->getLevelData()->setThundering(true);
}

std::shared_ptr<GameCommandPacket> ToggleDownfallCommand::preparePacket()
{
	return std::shared_ptr<GameCommandPacket>( new GameCommandPacket(eGameCommand_ToggleDownfall, byteArray() ));
}
