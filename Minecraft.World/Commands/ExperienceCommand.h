#pragma once

#include "Command.h"

class CommandSender;

class ExperienceCommand : public Command {
public:
    virtual EGameCommand getId();
    virtual void execute(std::shared_ptr<CommandSender> source,
                         byteArray commandData);

protected:
    std::shared_ptr<Player> getPlayer(PlayerUID playerId);
};