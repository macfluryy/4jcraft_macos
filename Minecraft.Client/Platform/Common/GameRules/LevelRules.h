#pragma once

class LevelRuleset;

class LevelRules
{
public:
	LevelRules();

	void addLevelRule(const std::wstring &displayName, std::uint8_t *pbData, unsigned int dataLength);
	void addLevelRule(const std::wstring &displayName, LevelRuleset *rootRule);

	void removeLevelRule(LevelRuleset *removing);
};
