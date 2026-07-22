
#include "minecraft/commands/common/EffectCommand.h"

#include <stdint.h>

#include <memory>
#include <string>

#include "minecraft/commands/CommandsEnum.h"

class CommandSender;

EGameCommand EffectCommand::getId() { return eGameCommand_Effect; }

int EffectCommand::getPermissionLevel() { return LEVEL_GAMEMASTERS; }

std::wstring EffectCommand::getUsage(CommandSender* source) {
    return L"commands.effect.usage";
}

void EffectCommand::execute(std::shared_ptr<CommandSender> source,
                            std::vector<uint8_t>& commandData) {
    
    
    

    
    
    
    
    
    
    
    
    
    
    
    
    

    
    
    
    

    
    
    
    
    
    
    
    
    
    
    

    
    
    

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    

    
    

    
}

std::wstring EffectCommand::getPlayerNames() {
    return L"";  
}

bool EffectCommand::isValidWildcardPlayerArgument(std::wstring args,
                                                  int argumentIndex) {
    return argumentIndex == 0;
}