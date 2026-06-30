#include "ListPlayersCommand.h"

#include <memory>
#include <string>
#include <vector>

#include "minecraft/commands/CommandSender.h"
#include "minecraft/commands/CommandsEnum.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/server/PlayerList.h"
#include "minecraft/server/level/ServerPlayer.h"

EGameCommand ListPlayersCommand::getId() { return eGameCommand_List; }

int ListPlayersCommand::getPermissionLevel() { return LEVEL_ALL; }

void ListPlayersCommand::execute(std::shared_ptr<CommandSender> source,
                                 std::vector<uint8_t>& commandData) {
    MinecraftServer* server = MinecraftServer::getInstance();
    if (server == nullptr) {
        source->sendMessage(L"§cServer not running");
        return;
    }
    PlayerList* players = server->getPlayers();
    int total = (int)players->players.size();
    int max = players->getMaxPlayers();

    source->sendMessage(L"§eThere are " + std::to_wstring(total) + L"/" +
                        std::to_wstring(max) + L" players online:");

    std::wstring line;
    for (size_t i = 0; i < players->players.size(); ++i) {
        auto& p = players->players[i];
        if (p == nullptr) continue;
        if (!line.empty()) line += L", ";
        line += L"§a" + p->getName() + L"§7";
    }
    if (!line.empty()) source->sendMessage(line);
}
