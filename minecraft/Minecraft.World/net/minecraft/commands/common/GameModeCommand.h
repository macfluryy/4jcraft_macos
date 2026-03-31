#pragma once

#include <stdint.h>
#include <memory>
#include <string>

#include "Minecraft.World/net/minecraft/commands/Command.h"
#include "Minecraft.World/net/minecraft/commands/CommandsEnum.h"

class GameType;
class CommandSender;

class GameModeCommand : public Command {
public:
    virtual EGameCommand getId();
    int getPermissionLevel();
    virtual void execute(std::shared_ptr<CommandSender> source,
                         std::vector<uint8_t>& commandData);

protected:
    GameType* getModeForString(std::shared_ptr<CommandSender> source,
                               const std::wstring& name);
};