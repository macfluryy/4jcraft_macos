#include "GameModeCommand.h"

#include "java/InputOutputStream/ByteArrayInputStream.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "minecraft/commands/CommandSender.h"
#include "minecraft/commands/CommandsEnum.h"
#include "minecraft/server/level/ServerPlayer.h"
#include "minecraft/server/level/ServerPlayerGameMode.h"
#include "minecraft/world/level/LevelSettings.h"
#include "minecraft/client/Minecraft.h"

class CommandSender;

EGameCommand GameModeCommand::getId() { return eGameCommand_GameMode; }

int GameModeCommand::getPermissionLevel() { return LEVEL_GAMEMASTERS; }

void GameModeCommand::execute(std::shared_ptr<CommandSender> source,
                              std::vector<uint8_t>& commandData) {
    if (commandData.empty()) {
        source->sendMessage(L"§cUsage: /gamemode [survival|creative|adventure|spectator]");
        return;
    }
    
    try {
        ByteArrayInputStream bais(commandData);
        DataInputStream dis(&bais);
        
        int gameModeId = dis.readInt();
        
        auto player = std::dynamic_pointer_cast<ServerPlayer>(source);
        if (player == nullptr) {
            source->sendMessage(L"§cOnly players can change gamemode");
            return;
        }
        
        if (gameModeId < 0 || gameModeId > 3) {
            source->sendMessage(L"§cInvalid gamemode ID: " + std::to_wstring(gameModeId));
            return;
        }
        
        GameType* gameType = GameType::byId(gameModeId);
        if (gameType != nullptr) {
            player->setGameMode(gameType);
        } else {
            source->sendMessage(L"§cInvalid gamemode");
            return;
        }
        
        std::wstring modes[] = { L"Survival", L"Creative", L"Adventure", L"Spectator" };
        source->sendMessage(L"§aGamemode changed to: " + modes[gameModeId]);
        
    } catch (const std::exception& e) {
        source->sendMessage(L"§cError executing gamemode command");
    }
}