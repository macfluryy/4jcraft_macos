#pragma once
//using namespace std;

#include "GameRuleDefinition.h"

class BiomeOverride : public GameRuleDefinition
{
private:
	std::uint8_t m_topTile;
	std::uint8_t m_tile;
	int m_biomeId;

public:
	BiomeOverride();

	virtual ConsoleGameRules::EGameRuleType getActionType() { return ConsoleGameRules::eGameRuleType_BiomeOverride; }
	
	virtual void writeAttributes(DataOutputStream *dos, unsigned int numAttrs);
	virtual void addAttribute(const std::wstring &attributeName, const std::wstring &attributeValue);

	bool isBiome(int id);
	void getTileValues(std::uint8_t &tile, std::uint8_t &topTile);
};
