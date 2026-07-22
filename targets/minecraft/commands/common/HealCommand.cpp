#include "HealCommand.h"

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
#include "minecraft/world/food/FoodConstants.h"
#include "minecraft/world/food/FoodData.h"

EGameCommand HealCommand::getId() { return eGameCommand_Heal; }

int HealCommand::getPermissionLevel() { return LEVEL_GAMEMASTERS; }

void HealCommand::execute(std::shared_ptr<CommandSender> source,
                          std::vector<uint8_t>& commandData) {
    std::wstring targetName;
    if (!commandData.empty()) {
        try {
            ByteArrayInputStream bais(commandData);
            DataInputStream dis(&bais);
            targetName = dis.readUTF();
        } catch (...) {
            
        }
    }

    std::shared_ptr<Player> target;
    if (targetName.empty()) {
        target = std::dynamic_pointer_cast<Player>(source);
    } else {
        MinecraftServer* server = MinecraftServer::getInstance();
        if (server != nullptr) {
            target = server->getPlayers()->getPlayer(targetName);
        }
    }

    if (target == nullptr) {
        source->sendMessage(L"§cTarget player not found");
        return;
    }

    target->setHealth(target->getMaxHealth());
    
    FoodData* food = target->getFoodData();
    if (food != nullptr) {
        food->setFoodLevel(FoodConstants::MAX_FOOD);
        food->setSaturation(FoodConstants::MAX_SATURATION);
    }

    if (targetName.empty()) {
        source->sendMessage(L"§aHealed");
    } else {
        source->sendMessage(L"§aHealed " + target->getName());
        target->sendMessage(L"§aYou were healed");
    }
}
