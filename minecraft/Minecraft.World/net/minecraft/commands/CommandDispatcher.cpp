#include <string>
#include <utility>

#include "CommandDispatcher.h"
#include "Minecraft.Client/Linux/Linux_App.h"
#include "Minecraft.World/net/minecraft/commands/Command.h"
#include "Minecraft.World/net/minecraft/commands/CommandSender.h"

int CommandDispatcher::performCommand(std::shared_ptr<CommandSender> sender,
                                      EGameCommand command,
                                      std::vector<uint8_t>& commandData) {
    auto it = commandsById.find(command);

    if (it != commandsById.end()) {
        Command* command = it->second;
        if (command->canExecute(sender)) {
            command->execute(sender, commandData);
        } else {
#ifndef _CONTENT_PACKAGE
            sender->sendMessage(
                L"\u00A7cYou do not have permission to use this command.");
#endif
        }
    } else {
        app.DebugPrintf("Command %d not found!\n", command);
    }

    return 0;
}

Command* CommandDispatcher::addCommand(Command* command) {
    commandsById[command->getId()] = command;
    commands.insert(command);
    return command;
}