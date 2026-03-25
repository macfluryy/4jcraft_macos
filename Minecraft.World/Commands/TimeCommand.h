#pragma once

#include "Command.h"

class TimeCommand : public Command {
public:
    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(std::shared_ptr<CommandSender> source,
                         byteArray commandData);

protected:
    void doSetTime(std::shared_ptr<CommandSender> source, int value);
    void doAddTime(std::shared_ptr<CommandSender> source, int value);

public:
    static std::shared_ptr<GameCommandPacket> preparePacket(bool night);
};