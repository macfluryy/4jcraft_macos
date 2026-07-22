#pragma once

#include <vector>

#include "GameRule.h"

class GameRuleDefinition;



class GameRulesInstance : public GameRule {
public:
    
    
    enum EGameRulesInstanceType {
        eGameRulesInstanceType_ServerPlayer,
        eGameRulesInstanceType_Server,
        eGameRulesInstanceType_Count
    };

public:
    GameRulesInstance(GameRuleDefinition* definition, Connection* connection)
        : GameRule(definition, connection) {}
    
};