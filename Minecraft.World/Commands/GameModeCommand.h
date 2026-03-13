#pragma once

#include "Command.h"

class GameType;

class GameModeCommand : public Command {
public:
    virtual EGameCommand getId();
    virtual void execute(std::shared_ptr<CommandSender> source,
                         byteArray commandData);

protected:
    GameType* getModeForString(std::shared_ptr<CommandSender> source,
                               const std::wstring& name);
    std::shared_ptr<Player> getPlayer(PlayerUID playerId);
};