#include "ServerCommandDispatcher.h"

#include <memory>
#include <vector>

#include "TeleportCommand.h"
#include "minecraft/commands/Command.h"
#include "minecraft/commands/CommandSender.h"
#include "minecraft/commands/common/DefaultGameModeCommand.h"
#include "minecraft/commands/common/EnchantItemCommand.h"
#include "minecraft/commands/common/ExperienceCommand.h"
#include "minecraft/commands/common/GameModeCommand.h"
#include "minecraft/commands/common/GiveItemCommand.h"
#include "minecraft/commands/common/KillCommand.h"
#include "minecraft/commands/common/SummonCommand.h"
#include "minecraft/commands/common/ListPlayersCommand.h"
#include "minecraft/commands/common/KickPlayerCommand.h"
#include "minecraft/commands/common/MsgCommand.h"
#include "minecraft/commands/common/HealCommand.h"
#include "minecraft/commands/common/FeedCommand.h"
#include "minecraft/commands/common/SeedCommand.h"
#include "minecraft/commands/common/OpCommand.h"
#include "minecraft/commands/common/BanCommand.h"
#include "minecraft/commands/common/TpsCommand.h"
#include "minecraft/commands/common/PlayerLocationCommands.h"
#include "minecraft/commands/common/TimeCommand.h"
#include "minecraft/commands/common/ToggleDownfallCommand.h"
#include "minecraft/network/packet/ChatPacket.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/server/PlayerList.h"
#include "minecraft/server/level/ServerPlayer.h"

ServerCommandDispatcher::ServerCommandDispatcher() {
    addCommand(new TimeCommand());
    addCommand(new GameModeCommand());
    addCommand(new DefaultGameModeCommand());
    addCommand(new KillCommand());
    addCommand(new ToggleDownfallCommand());
    addCommand(new ExperienceCommand());
    addCommand(new TeleportCommand());
    addCommand(new GiveItemCommand());
    addCommand(new EnchantItemCommand());
    addCommand(new SummonCommand());
    // 4J Added - multiplayer admin / QoL commands
    addCommand(new ListPlayersCommand());
    addCommand(new KickPlayerCommand());
    addCommand(new MsgCommand());
    addCommand(new HealCommand());
    addCommand(new FeedCommand());
    addCommand(new SeedCommand());
    addCommand(new OpCommand());
    addCommand(new DeOpCommand());
    addCommand(new BanCommand());
    addCommand(new PardonCommand());
    addCommand(new TpsCommand());
    // Player QoL: spawn / home / back / reply
    addCommand(new SpawnCommand());
    addCommand(new SetHomeCommand());
    addCommand(new HomeCommand());
    addCommand(new BackCommand());
    addCommand(new ReplyCommand());
    // addCommand(new EmoteCommand());
    // addCommand(new ShowSeedCommand());
    // addCommand(new HelpCommand());
    // addCommand(new DebugCommand());
    // addCommand(new MessageCommand());

    // if (MinecraftServer::getInstance()->isDedicatedServer())
    //{
    //	addCommand(new OpCommand());
    //	addCommand(new DeOpCommand());
    //	addCommand(new StopCommand());
    //	addCommand(new SaveAllCommand());
    //	addCommand(new SaveOffCommand());
    //	addCommand(new SaveOnCommand());
    //	addCommand(new BanIpCommand());
    //	addCommand(new PardonIpCommand());
    //	addCommand(new BanPlayerCommand());
    //	addCommand(new ListBansCommand());
    //	addCommand(new PardonPlayerCommand());
    //	addCommand(new KickCommand());
    //	addCommand(new ListPlayersCommand());
    //	addCommand(new BroadcastCommand());
    //	addCommand(new WhitelistCommand());
    // }
    // else
    //{
    //	addCommand(new PublishLocalServerCommand());
    // }

    //        addCommand(new ServerTempDebugCommand());

    Command::setLogger(this);
}

void ServerCommandDispatcher::logAdminCommand(
    std::shared_ptr<CommandSender> source, int type,
    ChatPacket::EChatPacketMessage messageType, const std::wstring& message,
    int customData, const std::wstring& additionalMessage) {
    PlayerList* playerList = MinecraftServer::getInstance()->getPlayers();
    // for (Player player : MinecraftServer.getInstance().getPlayers().players)
    for (auto it = playerList->players.begin(); it != playerList->players.end();
         ++it) {
        std::shared_ptr<ServerPlayer> player = *it;
        if (player != source && playerList->isOp(player)) {
            // TODO: Change chat packet to be able to send more bits of data
            // 4J Stu - Take this out until we can add the name of the player
            // performing the action. Also if the target is a mod then maybe
            // don't need the message?
            // player->sendMessage(message, messageType, customData,
            // additionalMessage); player->sendMessage("\u00A77\u00A7o[" +
            // source.getName() + ": " + player.localize(message, args) + "]");
        }
    }

    if ((type & LOGTYPE_DONT_SHOW_TO_SELF) != LOGTYPE_DONT_SHOW_TO_SELF) {
        source->sendMessage(message, messageType, customData,
                            additionalMessage);
    }
}