#include "OpCommand.h"

#include <memory>
#include <string>
#include <vector>

#include "java/InputOutputStream/ByteArrayInputStream.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "minecraft/commands/CommandSender.h"
#include "minecraft/commands/CommandsEnum.h"
#include "minecraft/network/packet/PlayerInfoPacket.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/server/PlayerList.h"
#include "minecraft/server/level/ServerPlayer.h"
#include "minecraft/server/network/PlayerConnection.h"
#include "minecraft/world/entity/player/Player.h"
#include "app/common/src/Network/NetworkPlayerInterface.h"

namespace {

void applyOp(std::shared_ptr<CommandSender> source,
             const std::wstring& targetName, bool grant) {
    MinecraftServer* server = MinecraftServer::getInstance();
    if (server == nullptr) {
        source->sendMessage(L"§cServer not running");
        return;
    }
    if (targetName.empty()) {
        source->sendMessage(grant ? L"§cUsage: /op <player>"
                                  : L"§cUsage: /deop <player>");
        return;
    }
    auto target = server->getPlayers()->getPlayer(targetName);
    if (target == nullptr) {
        source->sendMessage(L"§cPlayer not found: " + targetName);
        return;
    }

    target->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_Op,
                                   grant ? 1u : 0u);

    
    if (target->connection != nullptr &&
        target->connection->getNetworkPlayer() != nullptr) {
        server->getPlayers()->broadcastAll(std::make_shared<PlayerInfoPacket>(
            target->connection->getNetworkPlayer()->GetSmallId(), -1,
            target->getAllPlayerGamePrivileges()));
    }

    if (grant) {
        source->sendMessage(L"§aOp'd " + targetName);
        target->sendMessage(L"§aYou are now an operator");
    } else {
        source->sendMessage(L"§aDe-op'd " + targetName);
        target->sendMessage(L"§eYou are no longer an operator");
    }
}

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

EGameCommand OpCommand::getId() { return eGameCommand_Op; }
int OpCommand::getPermissionLevel() { return LEVEL_OWNERS; }
void OpCommand::execute(std::shared_ptr<CommandSender> source,
                        std::vector<uint8_t>& commandData) {
    if (!requireOp(source)) return;
    applyOp(source, readUtfArg(commandData), true);
}

EGameCommand DeOpCommand::getId() { return eGameCommand_DeOp; }
int DeOpCommand::getPermissionLevel() { return LEVEL_OWNERS; }
void DeOpCommand::execute(std::shared_ptr<CommandSender> source,
                          std::vector<uint8_t>& commandData) {
    if (!requireOp(source)) return;
    applyOp(source, readUtfArg(commandData), false);
}
