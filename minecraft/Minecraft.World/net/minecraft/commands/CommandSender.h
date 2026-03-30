#pragma once

#include "CommandsEnum.h"
#include "../network/packet/ChatPacket.h"

class CommandSender {
public:
    // virtual int getUID() = 0;
    virtual void sendMessage(
        const std::wstring& message,
        ChatPacket::EChatPacketMessage type = ChatPacket::e_ChatCustom,
        int customData = -1, const std::wstring& additionalMessage = L"") = 0;
    virtual bool hasPermission(EGameCommand command) = 0;
};