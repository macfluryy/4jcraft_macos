#include "../../Minecraft.World/Platform/stdafx.h"
#include "../../Minecraft.World/Util/StringHelpers.h"
#include "../../Minecraft.Client/Utils/StringTable.h"
#include "ConsoleGameRules.h"
#include "LevelRuleset.h"

LevelRuleset::LevelRuleset()
{
	m_stringTable = NULL;
}

LevelRuleset::~LevelRuleset()
{
	for(AUTO_VAR(it, m_areas.begin()); it != m_areas.end(); ++it)
	{
		delete *it;
	}
}

void LevelRuleset::getChildren(std::vector<GameRuleDefinition *> *children)
{
	CompoundGameRuleDefinition::getChildren(children);
	for (AUTO_VAR(it, m_areas.begin()); it != m_areas.end(); it++)
		children->push_back(*it);
}

GameRuleDefinition *LevelRuleset::addChild(ConsoleGameRules::EGameRuleType ruleType)
{
	GameRuleDefinition *rule = NULL;
	if(ruleType == ConsoleGameRules::eGameRuleType_NamedArea)
	{
		rule = new NamedAreaRuleDefinition();
		m_areas.push_back((NamedAreaRuleDefinition *)rule);
	}
	else
	{
		rule = CompoundGameRuleDefinition::addChild(ruleType);
	}
	return rule;
}

void LevelRuleset::loadStringTable(StringTable *table)
{
	m_stringTable = table;
}

const wchar_t *LevelRuleset::getString(const std::wstring &key)
{
	if(m_stringTable == NULL)
	{
		return L"";
	}
	else
	{
		return m_stringTable->getString(key);
	}
}

AABB *LevelRuleset::getNamedArea(const std::wstring &areaName)
{
	AABB *area = NULL;
	for(AUTO_VAR(it, m_areas.begin()); it != m_areas.end(); ++it)
	{
		if( (*it)->getName().compare(areaName) == 0 )
		{
			area = (*it)->getArea();
			break;
		}
	}
	return area;
}
