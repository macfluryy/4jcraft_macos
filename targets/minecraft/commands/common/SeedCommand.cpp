#include "SeedCommand.h"

#include <memory>
#include <string>
#include <vector>

#include "minecraft/commands/CommandSender.h"
#include "minecraft/commands/CommandsEnum.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/server/level/ServerLevel.h"
#include "minecraft/world/level/storage/LevelData.h"

EGameCommand SeedCommand::getId() { return eGameCommand_Seed; }

int SeedCommand::getPermissionLevel() { return LEVEL_GAMEMASTERS; }

void SeedCommand::execute(std::shared_ptr<CommandSender> source,
                          std::vector<uint8_t>& commandData) {
    MinecraftServer* server = MinecraftServer::getInstance();
    if (server == nullptr || server->levels.empty()) {
        source->sendMessage(L"§cServer not running");
        return;
    }
    ServerLevel* level = server->levels[0];
    if (level == nullptr || level->getLevelData() == nullptr) {
        source->sendMessage(L"§cWorld data unavailable");
        return;
    }

    int64_t seed = level->getLevelData()->getSeed();
    source->sendMessage(L"§eSeed: §f" + std::to_wstring(seed));
}
