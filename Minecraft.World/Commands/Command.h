#pragma once

// 4J Stu - Based loosely on the Java versions

#include "CommandsEnum.h"
#include "../Network/Packets/ChatPacket.h"

class AdminLogCommand;
class CommandSender;
class ServerPlayer;

class Command {
private:
    static AdminLogCommand* logger;

public:
    virtual EGameCommand getId() = 0;
    virtual void execute(std::shared_ptr<CommandSender> source,
                         byteArray commandData) = 0;
    virtual bool canExecute(std::shared_ptr<CommandSender> source);

    static void logAdminAction(std::shared_ptr<CommandSender> source,
                               ChatPacket::EChatPacketMessage messageType,
                               const std::wstring& message = L"",
                               int customData = -1,
                               const std::wstring& additionalMessage = L"");
    static void logAdminAction(std::shared_ptr<CommandSender> source, int type,
                               ChatPacket::EChatPacketMessage messageType,
                               const std::wstring& message = L"",
                               int customData = -1,
                               const std::wstring& additionalMessage = L"");
    static void setLogger(AdminLogCommand* logger);

protected:
    std::shared_ptr<ServerPlayer> getPlayer(PlayerUID playerId);
};