#include "Command.h"

#include "minecraft/commands/AdminLogCommand.h"
#include "minecraft/commands/CommandSender.h"
#include "minecraft/network/packet/ChatPacket.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/server/PlayerList.h"
#include "minecraft/server/level/ServerPlayer.h"

AdminLogCommand* Command::logger;

int Command::getPermissionLevel() { return LEVEL_OWNERS; }

bool Command::canExecute(std::shared_ptr<CommandSender> source) {
    
    return true; 
}

void Command::logAdminAction(std::shared_ptr<CommandSender> source,
                             ChatPacket::EChatPacketMessage messageType,
                             const std::wstring& message, int customData,
                             const std::wstring& additionalMessage) {
    logAdminAction(source, 0, messageType, message, customData,
                   additionalMessage);
}

void Command::logAdminAction(std::shared_ptr<CommandSender> source, int type,
                             ChatPacket::EChatPacketMessage messageType,
                             const std::wstring& message, int customData,
                             const std::wstring& additionalMessage) {
    if (logger != nullptr) {
        logger->logAdminCommand(source, type, messageType, message, customData,
                                additionalMessage);
    }
}

void Command::setLogger(AdminLogCommand* logger) { Command::logger = logger; }

bool Command::requireOp(std::shared_ptr<CommandSender> source) {
    auto serverPlayer = std::dynamic_pointer_cast<ServerPlayer>(source);
    if (serverPlayer == nullptr) {
        
        return true;
    }
    auto* server = MinecraftServer::getInstance();
    if (server == nullptr) return true;  
    if (server->getPlayers()->isOp(serverPlayer)) return true;

    serverPlayer->sendMessage(
        L"§cYou do not have permission to use this command");
    return false;
}

std::shared_ptr<ServerPlayer> Command::getPlayer(PlayerUID playerId) {
    std::shared_ptr<ServerPlayer> player =
        MinecraftServer::getInstance()->getPlayers()->getPlayer(playerId);

    if (player == nullptr) {
        return nullptr;
    } else {
        return player;
    }
}