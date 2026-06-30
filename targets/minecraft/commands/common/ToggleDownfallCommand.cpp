#include "ToggleDownfallCommand.h"

#include <string>
#include <vector>

#include "java/InputOutputStream/ByteArrayInputStream.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "minecraft/commands/CommandSender.h"
#include "minecraft/commands/CommandsEnum.h"
#include "minecraft/network/packet/ChatPacket.h"
#include "minecraft/network/packet/GameCommandPacket.h"
#include "minecraft/server/network/PlayerConnection.h"
#include "minecraft/network/packet/GameEventPacket.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/server/PlayerList.h"
#include "minecraft/server/level/ServerLevel.h"
#include "minecraft/server/level/ServerPlayer.h"
#include "minecraft/world/level/storage/LevelData.h"

EGameCommand ToggleDownfallCommand::getId() {
    return eGameCommand_ToggleDownfall;
}

int ToggleDownfallCommand::getPermissionLevel() { return LEVEL_GAMEMASTERS; }

void ToggleDownfallCommand::execute(std::shared_ptr<CommandSender> source,
                                    std::vector<uint8_t>& commandData) {
    // 4J - FIX: Handle weather command with proper type deserialization
    if (commandData.empty()) {
        source->sendMessage(L"§cUsage: /weather [clear|rain|thunder|thunderstorm]");
        return;
    }
    
    try {
        ByteArrayInputStream bais(commandData);
        DataInputStream dis(&bais);
        
        int weatherType = dis.readInt();
        
        doSetWeather(weatherType);
        
        std::wstring weatherNames[] = { L"Clear", L"Rain", L"Thunderstorm" };
        std::wstring weatherMsg = weatherNames[weatherType >= 0 && weatherType <= 2 ? weatherType : 0];
        source->sendMessage(L"§aWeather set to: " + weatherMsg);
        //logAdminAction(source, ChatPacket::e_ChatCustom, L"commands.weather.set");
        
    } catch (const std::exception& e) {
        source->sendMessage(L"§cError executing weather command");
    }
}

void ToggleDownfallCommand::doSetWeather(int weatherType) {
    MinecraftServer* server = MinecraftServer::getInstance();
    
    for (int i = 0; i < server->levels.size(); i++) {
        ServerLevel* level = server->levels[i];
        LevelData* levelData = level->getLevelData();
        
        if (weatherType == 0) {
            levelData->setRaining(false);
            levelData->setThundering(false);
            level->setRainLevel(0.0f);
            
            auto stopRainPacket = std::shared_ptr<GameEventPacket>(
                new GameEventPacket(GameEventPacket::STOP_RAINING, 0));
            for (auto& player : server->getPlayers()->players) {
                if (player && player->connection) {
                    player->connection->send(stopRainPacket);
                }
            }
        } else if (weatherType == 1) {
            levelData->setRaining(true);
            levelData->setThundering(false);
            level->setRainLevel(1.0f);
            
            auto startRainPacket = std::shared_ptr<GameEventPacket>(
                new GameEventPacket(GameEventPacket::START_RAINING, 0));
            for (auto& player : server->getPlayers()->players) {
                if (player && player->connection) {
                    player->connection->send(startRainPacket);
                }
            }
        } else if (weatherType == 2) {
            levelData->setRaining(true);
            levelData->setThundering(true);
            level->setRainLevel(1.0f);

            // 4J macOS - param=1 signals a thunderstorm so the client can
            // raise its thunderLevel (darker sky + lightning ambience),
            // not just plain rain. param=0 (used by the rain branch above)
            // stays a normal downpour.
            auto startRainPacket = std::shared_ptr<GameEventPacket>(
                new GameEventPacket(GameEventPacket::START_RAINING, 1));
            for (auto& player : server->getPlayers()->players) {
                if (player && player->connection) {
                    player->connection->send(startRainPacket);
                }
            }
        }
    }
}

std::shared_ptr<GameCommandPacket> ToggleDownfallCommand::preparePacket() {
    return std::shared_ptr<GameCommandPacket>(new GameCommandPacket(
        eGameCommand_ToggleDownfall, std::vector<uint8_t>()));
}