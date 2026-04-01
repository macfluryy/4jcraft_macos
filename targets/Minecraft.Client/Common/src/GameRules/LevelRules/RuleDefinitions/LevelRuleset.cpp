#include "LevelRuleset.h"

#include "Minecraft.Client/Common/src/GameRules/LevelRules/RuleDefinitions/NamedAreaRuleDefinition.h"
#include "Minecraft.Client/Common/src/Localisation/StringTable.h"
#include "Minecraft.Client/Common/src/GameRules/ConsoleGameRulesConstants.h"
#include "Minecraft.Client/Common/src/GameRules/LevelRules/RuleDefinitions/CompoundGameRuleDefinition.h"

class AABB;

LevelRuleset::LevelRuleset() { m_stringTable = nullptr; }

LevelRuleset::~LevelRuleset() {
    for (auto it = m_areas.begin(); it != m_areas.end(); ++it) {
        delete *it;
    }
}

void LevelRuleset::getChildren(std::vector<GameRuleDefinition*>* children) {
    CompoundGameRuleDefinition::getChildren(children);
    for (auto it = m_areas.begin(); it != m_areas.end(); it++)
        children->push_back(*it);
}

GameRuleDefinition* LevelRuleset::addChild(
    ConsoleGameRules::EGameRuleType ruleType) {
    GameRuleDefinition* rule = nullptr;
    if (ruleType == ConsoleGameRules::eGameRuleType_NamedArea) {
        rule = new NamedAreaRuleDefinition();
        m_areas.push_back((NamedAreaRuleDefinition*)rule);
    } else {
        rule = CompoundGameRuleDefinition::addChild(ruleType);
    }
    return rule;
}

void LevelRuleset::loadStringTable(StringTable* table) {
    m_stringTable = table;
}

const wchar_t* LevelRuleset::getString(const std::wstring& key) {
    if (m_stringTable == nullptr) {
        return L"";
    } else {
        return m_stringTable->getString(key);
    }
}

AABB* LevelRuleset::getNamedArea(const std::wstring& areaName) {
    AABB* area = nullptr;
    for (auto it = m_areas.begin(); it != m_areas.end(); ++it) {
        if ((*it)->getName().compare(areaName) == 0) {
            area = (*it)->getArea();
            break;
        }
    }
    return area;
}
