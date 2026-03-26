#pragma once

class Command;
class CommandSender;

class CommandDispatcher {
private:
    std::unordered_map<EGameCommand, Command*> commandsById;
    std::unordered_set<Command*> commands;

public:
    int performCommand(std::shared_ptr<CommandSender> sender,
                       EGameCommand command, byteArray commandData);
    Command* addCommand(Command* command);
};