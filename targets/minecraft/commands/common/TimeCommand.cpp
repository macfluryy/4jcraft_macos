#include "TimeCommand.h"

#include <string>
#include <vector>

#include "java/InputOutputStream/ByteArrayInputStream.h"
#include "java/InputOutputStream/ByteArrayOutputStream.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"
#include "minecraft/commands/CommandsEnum.h"
#include "minecraft/network/packet/ChatPacket.h"
#include "minecraft/network/packet/GameCommandPacket.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/server/level/ServerLevel.h"

class CommandSender;

EGameCommand TimeCommand::getId() { return eGameCommand_Time; }

int TimeCommand::getPermissionLevel() { return LEVEL_GAMEMASTERS; }

void TimeCommand::execute(std::shared_ptr<CommandSender> source,
                          std::vector<uint8_t>& commandData) {
    // 4J - FIX: Properly deserialize time command data
    if (commandData.empty()) {
        source->sendMessage(L"§cUsage: /time set [sunrise | day | noon | sunset | night | midnight | N ]");
        return;
    }
    
    try {
        ByteArrayInputStream bais(commandData);
        DataInputStream dis(&bais);

        int timeValue = dis.readInt();
        
        if (timeValue < 0 || timeValue > 23999) {
            source->sendMessage(L"§cInvalid time value: " + std::to_wstring(timeValue));
            source->sendMessage(L"§cTime must be between 0 and 23999 ticks");
            source->sendMessage(L"§c0=Sunrise, 1000=Day, 6000=Noon, 12000=Sunset, 13000=Night, 18000=Midnight");
            return;
        }

        doSetTime(source, timeValue);
        
        std::wstring timeDesc;
        if (timeValue == 0) timeDesc = L"Sunrise";
        else if (timeValue == 1000) timeDesc = L"Day (Morning)";
        else if (timeValue == 6000) timeDesc = L"Noon";
        else if (timeValue == 12000) timeDesc = L"Sunset";
        else if (timeValue == 13000) timeDesc = L"Night";
        else if (timeValue == 18000) timeDesc = L"Midnight";
        else timeDesc = L"Tick " + std::to_wstring(timeValue);
        
        source->sendMessage(L"§aTime set to: " + timeDesc + L" (" + std::to_wstring(timeValue) + L")");
        //logAdminAction(source, ChatPacket::e_ChatCustom, L"commands.time.set");
        
    } catch (const std::exception& e) {
        source->sendMessage(L"§cError executing time command");
    }
}

void TimeCommand::doSetTime(std::shared_ptr<CommandSender> source, int value) {
    for (int i = 0; i < MinecraftServer::getInstance()->levels.size(); i++) {
        MinecraftServer::getInstance()->levels[i]->setDayTime(value);
    }
}

void TimeCommand::doAddTime(std::shared_ptr<CommandSender> source, int value) {
    for (int i = 0; i < MinecraftServer::getInstance()->levels.size(); i++) {
        ServerLevel* level = MinecraftServer::getInstance()->levels[i];
        level->setDayTime(level->getDayTime() + value);
    }
}

std::shared_ptr<GameCommandPacket> TimeCommand::preparePacket(bool night) {
    ByteArrayOutputStream baos;
    DataOutputStream dos(&baos);

    dos.writeBoolean(night);

    return std::shared_ptr<GameCommandPacket>(
        new GameCommandPacket(eGameCommand_Time, baos.toByteArray()));
}