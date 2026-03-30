#pragma once
// using namespace std;

#include "../LevelRules/RuleDefinitions/GameRuleDefinition.h"
#include "Minecraft.World/net/minecraft/world/level/levelgen/structure/StructureFeature.h"

class StartFeature : public GameRuleDefinition {
private:
    int m_chunkX, m_chunkZ, m_orientation;
    StructureFeature::EFeatureTypes m_feature;

public:
    StartFeature();

    virtual ConsoleGameRules::EGameRuleType getActionType() {
        return ConsoleGameRules::eGameRuleType_StartFeature;
    }

    virtual void writeAttributes(DataOutputStream* dos, unsigned int numAttrs);
    virtual void addAttribute(const std::wstring& attributeName,
                              const std::wstring& attributeValue);

    bool isFeatureChunk(int chunkX, int chunkZ,
                        StructureFeature::EFeatureTypes feature,
                        int* orientation);
};