#pragma once

#include <stdint.h>

#include "minecraft/commands/Command.h"
#include "minecraft/commands/CommandsEnum.h"

// 4J macOS - small QoL location commands. They all live as separate
// Command subclasses so the dispatcher can route by enum, but their
// implementations sit together in PlayerLocationCommands.cpp because
// they share the same plumbing (resolving the source player, calling
// PlayerConnection::teleport, etc).

class SpawnCommand : public Command {
public:
    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(std::shared_ptr<CommandSender> source,
                         std::vector<uint8_t>& commandData);
};

class SetHomeCommand : public Command {
public:
    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(std::shared_ptr<CommandSender> source,
                         std::vector<uint8_t>& commandData);
};

class HomeCommand : public Command {
public:
    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(std::shared_ptr<CommandSender> source,
                         std::vector<uint8_t>& commandData);
};

class BackCommand : public Command {
public:
    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(std::shared_ptr<CommandSender> source,
                         std::vector<uint8_t>& commandData);
};

class ReplyCommand : public Command {
public:
    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(std::shared_ptr<CommandSender> source,
                         std::vector<uint8_t>& commandData);
};
