#pragma once

#include <stdint.h>
#include <string>

#include "minecraft/commands/Command.h"
#include "minecraft/commands/CommandsEnum.h"

class CommandSender;

class EffectCommand : public Command {
public:
    EGameCommand getId();
    int getPermissionLevel();
    std::wstring getUsage(CommandSender* source);
    void execute(std::shared_ptr<CommandSender> source, std::vector<uint8_t>& commandData);

protected:
    std::wstring getPlayerNames();

public:
    bool isValidWildcardPlayerArgument(std::wstring args, int argumentIndex);
};