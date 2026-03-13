#pragma once

#include "GameRuleDefinition.h"

class Pos;
class UseTileRuleDefinition;
class ItemInstance;

class CollectItemRuleDefinition : public GameRuleDefinition
{
private:
	// These values should map directly to the xsd definition for this Rule
	int m_itemId;
	unsigned char m_auxValue;
	int m_quantity;

public:
	CollectItemRuleDefinition();
	~CollectItemRuleDefinition();

	ConsoleGameRules::EGameRuleType getActionType() { return ConsoleGameRules::eGameRuleType_CollectItemRule; }

	virtual void writeAttributes(DataOutputStream *, unsigned int numAttributes);
	virtual void addAttribute(const std::wstring &attributeName, const std::wstring &attributeValue);

	virtual int getGoal();
	virtual int getProgress(GameRule *rule);
	
	virtual int getIcon() { return m_itemId; }
	virtual int getAuxValue() { return m_auxValue; }

	void populateGameRule(GameRulesInstance::EGameRulesInstanceType type, GameRule *rule);

	bool onCollectItem(GameRule *rule, std::shared_ptr<ItemInstance> item);

	static std::wstring generateXml(std::shared_ptr<ItemInstance> item);

private:	
	//static std::wstring generateXml(CollectItemRuleDefinition *ruleDef);
};