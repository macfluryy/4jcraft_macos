#include "KickPlayerCommand.h"

#include <memory>
#include <string>
#include <vector>

#include "java/InputOutputStream/ByteArrayInputStream.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "minecraft/commands/CommandSender.h"
#include "minecraft/commands/CommandsEnum.h"
#include "minecraft/network/Connection.h"
#include "minecraft/network/packet/ChatPacket.h"
#include "minecraft/network/packet/DisconnectPacket.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/server/PlayerList.h"
#include "minecraft/server/level/ServerPlayer.h"
#include "minecraft/server/network/PlayerConnection.h"
#include "app/common/src/Network/Socket.h"

EGameCommand KickPlayerCommand::getId() { return eGameCommand_Kick; }

int KickPlayerCommand::getPermissionLevel() { return LEVEL_ADMINS; }

void KickPlayerCommand::execute(std::shared_ptr<CommandSender> source,
                                std::vector<uint8_t>& commandData) {
    if (!requireOp(source)) return;
    if (commandData.empty()) {
        source->sendMessage(L"§cUsage: /kick <player> [reason]");
        return;
    }

    try {
        ByteArrayInputStream bais(commandData);
        DataInputStream dis(&bais);

        std::wstring targetName = dis.readUTF();
        std::wstring reason = dis.readUTF();

        MinecraftServer* server = MinecraftServer::getInstance();
        if (server == nullptr) {
            source->sendMessage(L"§cServer not running");
            return;
        }

        auto target = server->getPlayers()->getPlayer(targetName);
        if (target == nullptr) {
            source->sendMessage(L"§cPlayer not found: " + targetName);
            return;
        }

        // Don't allow kicking the host (source) accidentally
        if (target == source) {
            source->sendMessage(L"§cCannot kick yourself");
            return;
        }

        // Notify target before kick
        if (!reason.empty()) {
            target->sendMessage(L"§cKicked: " + reason);
        } else {
            target->sendMessage(L"§cYou were kicked from the server");
        }

        // Use the existing kick infrastructure if available; otherwise
        // disconnect the connection directly.
        if (target->connection != nullptr) {
            target->connection->setWasKicked();
            if (target->connection->connection != nullptr &&
                target->connection->connection->getSocket() != nullptr) {
                std::uint8_t smallId =
                    target->connection->connection->getSocket()->getSmallId();
                server->getPlayers()->kickPlayerByShortId(smallId);
            } else {
                target->connection->disconnect(
                    DisconnectPacket::eDisconnect_Kicked);
            }
        }

        source->sendMessage(L"§aKicked " + targetName +
                            (reason.empty() ? L"" : L" (" + reason + L")"));

    } catch (const std::exception&) {
        source->sendMessage(L"§cError executing kick command");
    }
}
