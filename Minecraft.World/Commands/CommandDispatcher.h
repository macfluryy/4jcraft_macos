#pragma once

class Command;
class CommandSender;

class CommandDispatcher {
private:
#if 0
    std::unordered_map<EGameCommand, Command*, std::hash<int>> commandsById;
#else
    std::unordered_map<EGameCommand, Command*> commandsById;
#endif
    std::unordered_set<Command*> commands;

public:
    int performCommand(std::shared_ptr<CommandSender> sender,
                       EGameCommand command, byteArray commandData);
    Command* addCommand(Command* command);
};