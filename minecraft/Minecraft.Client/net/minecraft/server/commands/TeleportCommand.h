#pragma once

#include <stdint.h>
#include <memory>

#include "minecraft/commands/Command.h"
#include "4J.Common/4J_Compat.h"
#include "minecraft/commands/CommandsEnum.h"
#include "minecraft/network/packet/GameCommandPacket.h"

class TeleportCommand : public Command {
public:
    virtual EGameCommand getId();
    virtual void execute(std::shared_ptr<CommandSender> source,
                         std::vector<uint8_t>& commandData);

    static std::shared_ptr<GameCommandPacket> preparePacket(
        PlayerUID subject, PlayerUID destination);
};