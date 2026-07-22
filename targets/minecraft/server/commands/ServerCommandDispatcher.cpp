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
    
    addCommand(new SpawnCommand());
    addCommand(new SetHomeCommand());
    addCommand(new HomeCommand());
    addCommand(new BackCommand());
    addCommand(new ReplyCommand());
    
    
    
    
    

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    

    

    Command::setLogger(this);
}

void ServerCommandDispatcher::logAdminCommand(
    std::shared_ptr<CommandSender> source, int type,
    ChatPacket::EChatPacketMessage messageType, const std::wstring& message,
    int customData, const std::wstring& additionalMessage) {
    PlayerList* playerList = MinecraftServer::getInstance()->getPlayers();
    
    for (auto it = playerList->players.begin(); it != playerList->players.end();
         ++it) {
        std::shared_ptr<ServerPlayer> player = *it;
        if (player != source && playerList->isOp(player)) {
            
            
            
            
            
            
            
        }
    }

    if ((type & LOGTYPE_DONT_SHOW_TO_SELF) != LOGTYPE_DONT_SHOW_TO_SELF) {
        source->sendMessage(message, messageType, customData,
                            additionalMessage);
    }
}