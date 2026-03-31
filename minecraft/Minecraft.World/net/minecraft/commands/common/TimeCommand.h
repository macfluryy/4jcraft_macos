#pragma once

#include <stdint.h>
#include <memory>

#include "Minecraft.World/net/minecraft/commands/Command.h"
#include "Minecraft.World/net/minecraft/commands/CommandsEnum.h"
#include "Minecraft.World/net/minecraft/network/packet/GameCommandPacket.h"

class CommandSender;

class TimeCommand : public Command {
public:
    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(std::shared_ptr<CommandSender> source,
                         std::vector<uint8_t>& commandData);

protected:
    void doSetTime(std::shared_ptr<CommandSender> source, int value);
    void doAddTime(std::shared_ptr<CommandSender> source, int value);

public:
    static std::shared_ptr<GameCommandPacket> preparePacket(bool night);
};