#pragma once



#include <stdint.h>

#include <format>
#include <memory>
#include <string>
#include <vector>

#include "platform/PlatformTypes.h"
#include "CommandsEnum.h"
#include "minecraft/network/packet/ChatPacket.h"

class AdminLogCommand;
class CommandSender;
class ServerPlayer;

class Command {
public:
    
    static const int LEVEL_ALL = 0;
    
    static const int LEVEL_MODERATORS = 1;
    
    static const int LEVEL_GAMEMASTERS = 2;
    
    static const int LEVEL_ADMINS = 3;
    
    static const int LEVEL_OWNERS = 4;

private:
    static AdminLogCommand* logger;

public:
    virtual EGameCommand getId() = 0;
    virtual int getPermissionLevel();
    virtual void execute(std::shared_ptr<CommandSender> source,
                         std::vector<uint8_t>& commandData) = 0;
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

    
    
    static bool requireOp(std::shared_ptr<CommandSender> source);

protected:
    std::shared_ptr<ServerPlayer> getPlayer(PlayerUID playerId);
};