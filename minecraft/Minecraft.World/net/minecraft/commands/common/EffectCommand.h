#pragma once

#include "../Command.h"

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