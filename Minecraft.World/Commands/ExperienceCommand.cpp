#include "../Platform/stdafx.h"
#include "../Headers/net.minecraft.commands.h"
#include "../../Minecraft.Client/MinecraftServer.h"
#include "../../Minecraft.Client/Network/PlayerList.h"
#include "../Headers/net.minecraft.world.level.h"
#include "ExperienceCommand.h"

EGameCommand ExperienceCommand::getId() { return eGameCommand_Experience; }

int ExperienceCommand::getPermissionLevel() { return LEVEL_GAMEMASTERS; }

void ExperienceCommand::execute(std::shared_ptr<CommandSender> source,
                                byteArray commandData) {
    //	if (args.length > 0) {
    //		Player player;
    //		String inputAmount = args[0];
    //
    //		bool levels = inputAmount.endsWith("l") ||
    // inputAmount.endsWith("L"); 		if (levels &&
    // inputAmount.length() > 1) inputAmount = inputAmount.substring(0,
    // inputAmount.length() - 1);
    //
    //		int amount = convertArgToInt(source, inputAmount);
    //		bool take = amount < 0;
    //
    //		if (take) amount *= -1;
    //
    //		if (args.length > 1) {
    //			player = convertToPlayer(source, args[1]);
    //		} else {
    //			player = convertSourceToPlayer(source);
    //		}
    //
    //		if (levels) {
    //			if (take) {
    //				player.giveExperienceLevels(-amount);
    //				logAdminAction(source,
    //"commands.xp.success.negative.levels", amount, player.getAName());
    //} else { 				player.giveExperienceLevels(amount);
    // logAdminAction(source, "commands.xp.success.levels", amount,
    // player.getAName());
    //			}
    //		} else {
    //			if (take) {
    //				throw new
    // UsageException("commands.xp.failure.widthdrawXp");
    // } else { 				player.increaseXp(amount);
    // logAdminAction(source,
    //"commands.xp.success", amount, player.getAName());
    //			}
    //		}
    //
    //		return;
    //	}
    //
    //	throw new UsageException("commands.xp.usage");
}