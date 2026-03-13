#pragma once
////using namespace std;

#include "LevelGenerators.h"
#include "LevelRules.h"
class LevelGenerationOptions;
class RootGameRulesDefinition;
class LevelChunk;
class DLCPack;
class DLCGameRulesFile;
class DLCGameRulesHeader;
class StringTable;
class GameRuleDefinition;
class DataInputStream;
class DataOutputStream;
class WstringLookup;

#define GAME_RULE_SAVENAME L"requiredGameRules.grf"

// 4J-JEV: 
#define LEVEL_GEN_ID int
#define LEVEL_GEN_ID_NULL 0

class GameRuleManager
{
public:
	static const WCHAR *wchTagNameA[ConsoleGameRules::eGameRuleType_Count];
	static const WCHAR *wchAttrNameA[ConsoleGameRules::eGameRuleAttr_Count];

	static const short version_number = 2;

private:
	LevelGenerationOptions *m_currentLevelGenerationOptions;
	LevelRuleset *m_currentGameRuleDefinitions;
	LevelGenerators m_levelGenerators;
	LevelRules m_levelRules;

public:
	GameRuleManager();

	void loadGameRules(DLCPack *);

	LevelGenerationOptions *loadGameRules(uint8_t *dIn, unsigned int dSize);
	void loadGameRules(LevelGenerationOptions *lgo, uint8_t *dIn, unsigned int dSize);

	void saveGameRules(uint8_t **dOut, unsigned int *dSize); 

private:
	LevelGenerationOptions *readHeader(DLCGameRulesHeader *grh);

	void writeRuleFile(DataOutputStream *dos);

public:
	bool readRuleFile(LevelGenerationOptions *lgo, uint8_t *dIn, unsigned int dSize, StringTable *strings); //(DLCGameRulesFile *dlcFile, StringTable *strings);

private:
	void readAttributes(DataInputStream *dis, std::vector<std::wstring> *tagsAndAtts, GameRuleDefinition *rule);
	void readChildren(DataInputStream *dis, std::vector<std::wstring> *tagsAndAtts, std::unordered_map<int, ConsoleGameRules::EGameRuleType> *tagIdMap, GameRuleDefinition *rule);

public:
	void processSchematics(LevelChunk *levelChunk);
	void processSchematicsLighting(LevelChunk *levelChunk);
	void loadDefaultGameRules();

private:
	bool loadGameRulesPack(File *path);

	LEVEL_GEN_ID addLevelGenerationOptions(LevelGenerationOptions *);

public:
	std::vector<LevelGenerationOptions *> *getLevelGenerators() { return m_levelGenerators.getLevelGenerators(); }
	void setLevelGenerationOptions(LevelGenerationOptions *levelGen);
	LevelRuleset *getGameRuleDefinitions() { return m_currentGameRuleDefinitions; }
	LevelGenerationOptions *getLevelGenerationOptions() { return m_currentLevelGenerationOptions; }
	const wchar_t *GetGameRulesString(const std::wstring &key);

	// 4J-JEV:
	// Properly cleans-up and unloads the current set of gameRules.
	void unloadCurrentGameRules();
};
