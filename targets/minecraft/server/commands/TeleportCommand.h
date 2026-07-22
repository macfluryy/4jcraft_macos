#pragma once

#include <stdint.h>

#include <memory>

#include "platform/PlatformTypes.h"
#include "minecraft/commands/Command.h"
#include "minecraft/commands/CommandsEnum.h"
#include "minecraft/network/packet/GameCommandPacket.h"

class TeleportCommand : public Command {
public:
    
    static const int TP_MODE_TO_PLAYER = 0;  
    static const int TP_MODE_TO_COORDS = 1;  
    static const int TP_MODE_PLAYER_TO_PLAYER = 2;  
    static const int TP_MODE_PLAYER_TO_COORDS = 3;  

public:
    virtual EGameCommand getId();
    virtual void execute(std::shared_ptr<CommandSender> source,
                         std::vector<uint8_t>& commandData);

    static std::shared_ptr<GameCommandPacket> preparePacket(
        PlayerUID subject, PlayerUID destination);
};
