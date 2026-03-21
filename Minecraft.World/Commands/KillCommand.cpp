#include "../Platform/stdafx.h"
#include "../Headers/net.minecraft.commands.h"
#include "../Headers/net.minecraft.world.entity.player.h"
#include "../Headers/net.minecraft.world.damagesource.h"
#include "../Util/BasicTypeContainers.h"
#include "KillCommand.h"

EGameCommand KillCommand::getId() { return eGameCommand_Kill; }

int KillCommand::getPermissionLevel() { return LEVEL_ALL; }

void KillCommand::execute(std::shared_ptr<CommandSender> source,
                          byteArray commandData) {
    std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(source);

    player->hurt(DamageSource::outOfWorld, Float::MAX_VALUE);

    source->sendMessage(L"Ouch. That look like it hurt.");
    // source.sendMessage(ChatMessageComponent.forTranslation("commands.kill.success"));
}