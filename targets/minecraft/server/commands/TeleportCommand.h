#pragma once

#include <stdint.h>

#include <memory>

#include "platform/PlatformTypes.h"
#include "minecraft/commands/Command.h"
#include "minecraft/commands/CommandsEnum.h"
#include "minecraft/network/packet/GameCommandPacket.h"

class TeleportCommand : public Command {
public:
    // 4J - extended TP modes
    static const int TP_MODE_TO_PLAYER = 0;  // tp <player>  (move source to target player)
    static const int TP_MODE_TO_COORDS = 1;  // tp <x> <y> <z>
    static const int TP_MODE_PLAYER_TO_PLAYER = 2;  // tp <subject> <target>
    static const int TP_MODE_PLAYER_TO_COORDS = 3;  // tp <subject> <x> <y> <z>

public:
    virtual EGameCommand getId();
    virtual void execute(std::shared_ptr<CommandSender> source,
                         std::vector<uint8_t>& commandData);

    static std::shared_ptr<GameCommandPacket> preparePacket(
        PlayerUID subject, PlayerUID destination);
};
