#pragma once

#include "Minecraft.World/net/minecraft/commands/Command.h"

class TeleportCommand : public Command {
public:
    virtual EGameCommand getId();
    virtual void execute(std::shared_ptr<CommandSender> source,
                         std::vector<uint8_t>& commandData);

    static std::shared_ptr<GameCommandPacket> preparePacket(
        PlayerUID subject, PlayerUID destination);
};