#pragma once

#include "Command.h"

class GameCommandPacket;

class GiveItemCommand : public Command {
public:
    virtual EGameCommand getId();
    virtual void execute(std::shared_ptr<CommandSender> source,
                         byteArray commandData);

public:
    static std::shared_ptr<GameCommandPacket> preparePacket(
        std::shared_ptr<Player> player, int item, int amount = 1, int aux = 0,
        const std::wstring& tag = L"");
};