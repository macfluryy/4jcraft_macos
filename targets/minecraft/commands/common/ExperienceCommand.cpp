#include "ExperienceCommand.h"

#include <memory>
#include <string>
#include <vector>

#include "java/InputOutputStream/ByteArrayInputStream.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "minecraft/commands/CommandSender.h"
#include "minecraft/commands/CommandsEnum.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/server/PlayerList.h"
#include "minecraft/server/level/ServerPlayer.h"
#include "minecraft/world/entity/player/Player.h"

EGameCommand ExperienceCommand::getId() { return eGameCommand_Experience; }

int ExperienceCommand::getPermissionLevel() { return LEVEL_GAMEMASTERS; }

void ExperienceCommand::execute(std::shared_ptr<CommandSender> source,
                                std::vector<uint8_t>& commandData) {
    if (commandData.empty()) {
        source->sendMessage(L"§cUsage: /xp <amount>[L|l] [player]");
        return;
    }

    try {
        ByteArrayInputStream bais(commandData);
        DataInputStream dis(&bais);

        int amount = dis.readInt();
        bool levels = dis.readBoolean();
        std::wstring targetName = dis.readUTF();

        std::shared_ptr<Player> targetPlayer;

        if (targetName.empty()) {
            targetPlayer = std::dynamic_pointer_cast<Player>(source);
        } else {
            auto server = MinecraftServer::getInstance();
            if (server != nullptr) {
                targetPlayer = server->getPlayers()->getPlayer(targetName);
            }
        }

        if (targetPlayer == nullptr) {
            source->sendMessage(L"§cTarget player not found");
            return;
        }

        if (levels) {
            targetPlayer->giveExperienceLevels(amount);
            if (amount >= 0) {
                source->sendMessage(L"§aGave " + std::to_wstring(amount) +
                                    L" experience level(s) to " +
                                    targetPlayer->getName());
            } else {
                source->sendMessage(L"§aTook " + std::to_wstring(-amount) +
                                    L" experience level(s) from " +
                                    targetPlayer->getName());
            }
        } else {
            if (amount < 0) {
                source->sendMessage(
                    L"§cCannot remove raw XP, use levels instead (e.g. /xp -5L)");
                return;
            }
            targetPlayer->increaseXp(amount);
            source->sendMessage(L"§aGave " + std::to_wstring(amount) +
                                L" experience to " +
                                targetPlayer->getName());
        }

    } catch (const std::exception&) {
        source->sendMessage(L"§cError executing xp command");
    }
}
