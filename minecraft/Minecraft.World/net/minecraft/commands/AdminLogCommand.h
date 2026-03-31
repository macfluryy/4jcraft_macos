#pragma once

#include "Minecraft.World/net/minecraft/network/packet/ChatPacket.h"

class CommandSender;

class AdminLogCommand {
public:
    static const int LOGTYPE_DONT_SHOW_TO_SELF = 1;

    virtual void logAdminCommand(
        std::shared_ptr<CommandSender> source, int type,
        ChatPacket::EChatPacketMessage messageType,
        const std::wstring& message = L"", int customData = -1,
        const std::wstring& additionalMessage = L"") = 0;
};