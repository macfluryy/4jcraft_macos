#pragma once

#include <stdint.h>

#include <memory>
#include <string>

#include "minecraft/commands/Command.h"
#include "minecraft/commands/CommandsEnum.h"

class CommandSender;

class SummonCommand : public Command {
public:
    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(std::shared_ptr<CommandSender> source,
                         std::vector<uint8_t>& commandData);

    // Resolve a mob name (case-insensitive) to its EntityIO numeric id.
    // Returns -1 if not found.
    static int resolveEntityIdByName(const std::wstring& name);
};
