#pragma once

#include "../Command.h"

class GameType;

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