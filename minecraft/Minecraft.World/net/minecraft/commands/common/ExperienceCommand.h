#pragma once

#include <stdint.h>

#include "Minecraft.World/net/minecraft/commands/Command.h"
#include "Minecraft.World/net/minecraft/commands/CommandsEnum.h"

class CommandSender;

class ExperienceCommand : public Command {
public:
    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(std::shared_ptr<CommandSender> source,
                         std::vector<uint8_t>& commandData);
};