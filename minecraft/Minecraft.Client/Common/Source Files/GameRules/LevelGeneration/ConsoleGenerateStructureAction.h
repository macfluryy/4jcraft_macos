#pragma once

#include "Minecraft.Client/Common/Source Files/GameRules/LevelRules/RuleDefinitions/GameRuleDefinition.h"

class ConsoleGenerateStructureAction : public GameRuleDefinition {
public:
    virtual int getEndX() = 0;
    virtual int getEndY() = 0;
    virtual int getEndZ() = 0;
};