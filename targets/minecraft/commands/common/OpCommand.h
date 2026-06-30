#pragma once

#include <stdint.h>

#include "minecraft/commands/Command.h"
#include "minecraft/commands/CommandsEnum.h"

// Shared implementation for /op and /deop. The first byte of commandData
// determines whether to grant (1) or revoke (0) operator privileges.
class OpCommand : public Command {
public:
    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(std::shared_ptr<CommandSender> source,
                         std::vector<uint8_t>& commandData);
};

class DeOpCommand : public Command {
public:
    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(std::shared_ptr<CommandSender> source,
                         std::vector<uint8_t>& commandData);
};
