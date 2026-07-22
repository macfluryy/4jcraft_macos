#include "TeleportCommand.h"

#include <string>

#include "java/Class.h"
#include "java/InputOutputStream/ByteArrayInputStream.h"
#include "java/InputOutputStream/ByteArrayOutputStream.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"
#include "minecraft/commands/CommandsEnum.h"
#include "minecraft/network/packet/ChatPacket.h"
#include "minecraft/network/packet/GameCommandPacket.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/server/PlayerList.h"
#include "minecraft/server/level/ServerPlayer.h"
#include "minecraft/server/network/PlayerConnection.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/dimension/Dimension.h"

EGameCommand TeleportCommand::getId() { return eGameCommand_Teleport; }

void TeleportCommand::execute(std::shared_ptr<CommandSender> source,
                              std::vector<uint8_t>& commandData) {
    if (commandData.empty()) {
        source->sendMessage(L"§cUsage: /tp <player> | <x> <y> <z> | <subject> <target>");
        return;
    }

    try {
        ByteArrayInputStream bais(commandData);
        DataInputStream dis(&bais);

        int mode = dis.readInt();

        MinecraftServer* server = MinecraftServer::getInstance();
        if (server == nullptr) return;
        PlayerList* players = server->getPlayerList();

        auto sourcePlayer = std::dynamic_pointer_cast<ServerPlayer>(source);

        if (mode == TP_MODE_TO_PLAYER) {
            
            std::wstring targetName = dis.readUTF();
            if (sourcePlayer == nullptr) {
                source->sendMessage(L"§cOnly players can use this form of /tp");
                return;
            }
            auto target = players->getPlayer(targetName);
            if (target == nullptr) {
                source->sendMessage(L"§cPlayer not found: " + targetName);
                return;
            }
            if (sourcePlayer->level->dimension->id !=
                target->level->dimension->id) {
                source->sendMessage(
                    L"§cTarget is in a different dimension");
                return;
            }
            sourcePlayer->ride(nullptr);
            sourcePlayer->connection->teleport(target->x, target->y, target->z,
                                               target->yRot, target->xRot);
            source->sendMessage(L"§aTeleported to " + targetName);

        } else if (mode == TP_MODE_TO_COORDS) {
            
            double x = dis.readDouble();
            double y = dis.readDouble();
            double z = dis.readDouble();
            if (sourcePlayer == nullptr) {
                source->sendMessage(L"§cOnly players can use this form of /tp");
                return;
            }
            sourcePlayer->ride(nullptr);
            sourcePlayer->connection->teleport(x, y, z, sourcePlayer->yRot,
                                               sourcePlayer->xRot);
            source->sendMessage(L"§aTeleported to " + std::to_wstring((int)x) +
                                L", " + std::to_wstring((int)y) + L", " +
                                std::to_wstring((int)z));

        } else if (mode == TP_MODE_PLAYER_TO_PLAYER) {
            
            std::wstring subjectName = dis.readUTF();
            std::wstring targetName = dis.readUTF();
            auto subject = players->getPlayer(subjectName);
            auto target = players->getPlayer(targetName);
            if (subject == nullptr) {
                source->sendMessage(L"§cPlayer not found: " + subjectName);
                return;
            }
            if (target == nullptr) {
                source->sendMessage(L"§cPlayer not found: " + targetName);
                return;
            }
            if (subject->level->dimension->id !=
                target->level->dimension->id) {
                source->sendMessage(
                    L"§cTarget is in a different dimension");
                return;
            }
            subject->ride(nullptr);
            subject->connection->teleport(target->x, target->y, target->z,
                                          target->yRot, target->xRot);
            source->sendMessage(L"§aTeleported " + subjectName + L" to " +
                                targetName);

        } else if (mode == TP_MODE_PLAYER_TO_COORDS) {
            
            std::wstring subjectName = dis.readUTF();
            double x = dis.readDouble();
            double y = dis.readDouble();
            double z = dis.readDouble();
            auto subject = players->getPlayer(subjectName);
            if (subject == nullptr) {
                source->sendMessage(L"§cPlayer not found: " + subjectName);
                return;
            }
            subject->ride(nullptr);
            subject->connection->teleport(x, y, z, subject->yRot,
                                          subject->xRot);
            source->sendMessage(L"§aTeleported " + subjectName + L" to " +
                                std::to_wstring((int)x) + L", " +
                                std::to_wstring((int)y) + L", " +
                                std::to_wstring((int)z));
        } else {
            source->sendMessage(L"§cUnknown teleport mode");
        }

    } catch (const std::exception&) {
        source->sendMessage(L"§cError executing teleport command");
    }
}

std::shared_ptr<GameCommandPacket> TeleportCommand::preparePacket(
    PlayerUID subject, PlayerUID destination) {
    ByteArrayOutputStream baos;
    DataOutputStream dos(&baos);

    dos.writePlayerUID(subject);
    dos.writePlayerUID(destination);

    return std::shared_ptr<GameCommandPacket>(
        new GameCommandPacket(eGameCommand_Teleport, baos.toByteArray()));
}
