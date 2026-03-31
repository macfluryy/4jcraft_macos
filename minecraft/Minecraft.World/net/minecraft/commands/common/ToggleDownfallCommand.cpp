#include <string>
#include <vector>

#include "Minecraft.Client/net/minecraft/server/MinecraftServer.h"
#include "Minecraft.Client/net/minecraft/server/level/ServerLevel.h"
#include "ToggleDownfallCommand.h"
#include "Minecraft.World/net/minecraft/network/packet/ChatPacket.h"
#include "Minecraft.World/net/minecraft/network/packet/GameCommandPacket.h"
#include "Minecraft.World/net/minecraft/world/level/storage/LevelData.h"

EGameCommand ToggleDownfallCommand::getId() {
    return eGameCommand_ToggleDownfall;
}

int ToggleDownfallCommand::getPermissionLevel() { return LEVEL_GAMEMASTERS; }

void ToggleDownfallCommand::execute(std::shared_ptr<CommandSender> source,
                                    std::vector<uint8_t>& commandData) {
    doToggleDownfall();
    logAdminAction(source, ChatPacket::e_ChatCustom,
                   L"commands.downfall.success");
}

void ToggleDownfallCommand::doToggleDownfall() {
    MinecraftServer::getInstance()->levels[0]->toggleDownfall();
    MinecraftServer::getInstance()->levels[0]->getLevelData()->setThundering(
        true);
}

std::shared_ptr<GameCommandPacket> ToggleDownfallCommand::preparePacket() {
    return std::shared_ptr<GameCommandPacket>(
        new GameCommandPacket(eGameCommand_ToggleDownfall, std::vector<uint8_t>()));
}