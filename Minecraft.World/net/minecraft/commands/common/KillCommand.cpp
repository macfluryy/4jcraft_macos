#include "../../../../Header Files/stdafx.h"
#include "../net.minecraft.commands.h"
#include "../../world/entity/player/net.minecraft.world.entity.player.h"
#include "../../world/damageSource/net.minecraft.world.damagesource.h"
#include "KillCommand.h"
#include <limits>

EGameCommand KillCommand::getId() { return eGameCommand_Kill; }

int KillCommand::getPermissionLevel() { return LEVEL_ALL; }

void KillCommand::execute(std::shared_ptr<CommandSender> source,
                          byteArray commandData) {
    std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(source);

    player->hurt(DamageSource::outOfWorld, std::numeric_limits<float>::max());

    source->sendMessage(L"Ouch. That look like it hurt.");
    // source.sendMessage(ChatMessageComponent.forTranslation("commands.kill.success"));
}
