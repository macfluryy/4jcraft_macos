#include "MsgCommand.h"

#include <memory>
#include <string>
#include <vector>

#include "java/InputOutputStream/ByteArrayInputStream.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "minecraft/commands/CommandSender.h"
#include "minecraft/commands/CommandsEnum.h"
#include "minecraft/network/packet/ChatPacket.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/server/PlayerList.h"
#include "minecraft/server/level/ServerPlayer.h"

EGameCommand MsgCommand::getId() { return eGameCommand_Msg; }

int MsgCommand::getPermissionLevel() { return LEVEL_ALL; }

void MsgCommand::execute(std::shared_ptr<CommandSender> source,
                         std::vector<uint8_t>& commandData) {
    if (commandData.empty()) {
        source->sendMessage(L"§cUsage: /msg <player> <message>");
        return;
    }

    try {
        ByteArrayInputStream bais(commandData);
        DataInputStream dis(&bais);

        std::wstring targetName = dis.readUTF();
        std::wstring message = dis.readUTF();

        if (targetName.empty() || message.empty()) {
            source->sendMessage(L"§cUsage: /msg <player> <message>");
            return;
        }

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

        std::wstring senderName = L"<unknown>";
        auto sourcePlayer =
            std::dynamic_pointer_cast<ServerPlayer>(source);
        if (sourcePlayer != nullptr) {
            senderName = sourcePlayer->getName();
        }

        
        target->sendMessage(L"§d[" + senderName + L" -> me] §r" + message);
        
        source->sendMessage(L"§d[me -> " + targetName + L"] §r" + message);

        target->m_lastReplyTo = senderName;
        if (sourcePlayer != nullptr) {
            sourcePlayer->m_lastReplyTo = targetName;
        }

    } catch (const std::exception&) {
        source->sendMessage(L"§cError executing msg command");
    }
}
