#include "BanCommand.h"

#include <memory>
#include <string>
#include <vector>

#include "java/InputOutputStream/ByteArrayInputStream.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "minecraft/commands/CommandSender.h"
#include "minecraft/commands/CommandsEnum.h"
#include "minecraft/network/Connection.h"
#include "minecraft/network/packet/DisconnectPacket.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/server/PlayerList.h"
#include "minecraft/server/level/ServerPlayer.h"
#include "minecraft/server/network/PlayerConnection.h"
#include "app/common/src/Network/Socket.h"

namespace {

std::wstring readUtfArg(std::vector<uint8_t>& commandData) {
    if (commandData.empty()) return L"";
    try {
        ByteArrayInputStream bais(commandData);
        DataInputStream dis(&bais);
        return dis.readUTF();
    } catch (...) {
        return L"";
    }
}

}  

EGameCommand BanCommand::getId() { return eGameCommand_Ban; }
int BanCommand::getPermissionLevel() { return LEVEL_ADMINS; }
void BanCommand::execute(std::shared_ptr<CommandSender> source,
                         std::vector<uint8_t>& commandData) {
    if (!requireOp(source)) return;
    std::wstring targetName = readUtfArg(commandData);
    if (targetName.empty()) {
        source->sendMessage(L"§cUsage: /ban <player>");
        return;
    }

    MinecraftServer* server = MinecraftServer::getInstance();
    if (server == nullptr) return;
    PlayerList* players = server->getPlayers();
    auto target = players->getPlayer(targetName);
    if (target == nullptr) {
        source->sendMessage(L"§cPlayer not found: " + targetName);
        return;
    }
    if (target == source) {
        source->sendMessage(L"§cCannot ban yourself");
        return;
    }

    PlayerUID xuid = target->getOnlineXuid();
    if (!players->banXuid(xuid)) {
        source->sendMessage(L"§ePlayer is already banned");
        return;
    }

    target->sendMessage(L"§cYou have been banned from the server");

    
    if (target->connection != nullptr) {
        target->connection->setWasKicked();
        if (target->connection->connection != nullptr &&
            target->connection->connection->getSocket() != nullptr) {
            std::uint8_t smallId =
                target->connection->connection->getSocket()->getSmallId();
            players->kickPlayerByShortId(smallId);
        } else {
            target->connection->disconnect(
                DisconnectPacket::eDisconnect_Banned);
        }
    }

    source->sendMessage(L"§aBanned " + targetName);
}

EGameCommand PardonCommand::getId() { return eGameCommand_Pardon; }
int PardonCommand::getPermissionLevel() { return LEVEL_ADMINS; }
void PardonCommand::execute(std::shared_ptr<CommandSender> source,
                            std::vector<uint8_t>& commandData) {
    if (!requireOp(source)) return;
    std::wstring targetName = readUtfArg(commandData);
    if (targetName.empty()) {
        source->sendMessage(L"§cUsage: /pardon <player>");
        return;
    }

    MinecraftServer* server = MinecraftServer::getInstance();
    if (server == nullptr) return;
    PlayerList* players = server->getPlayers();

    
    
    
    auto target = players->getPlayer(targetName);
    if (target == nullptr) {
        source->sendMessage(
            L"§cPlayer must be reachable to resolve their id (not online): " +
            targetName);
        return;
    }

    if (players->pardonXuid(target->getOnlineXuid())) {
        source->sendMessage(L"§aPardoned " + targetName);
    } else {
        source->sendMessage(L"§ePlayer was not banned");
    }
}
