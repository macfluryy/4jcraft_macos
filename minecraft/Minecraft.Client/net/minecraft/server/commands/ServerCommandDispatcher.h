#pragma once

#include <string>

#include "Minecraft.World/net/minecraft/commands/CommandDispatcher.h"
#include "Minecraft.World/net/minecraft/commands/AdminLogCommand.h"
#include "Minecraft.World/net/minecraft/network/packet/ChatPacket.h"

class ServerCommandDispatcher : public CommandDispatcher,
                                public AdminLogCommand {
public:
    ServerCommandDispatcher();
    void logAdminCommand(std::shared_ptr<CommandSender> source, int type,
                         ChatPacket::EChatPacketMessage messageType,
                         const std::wstring& message = L"", int customData = -1,
                         const std::wstring& additionalMessage = L"");
};