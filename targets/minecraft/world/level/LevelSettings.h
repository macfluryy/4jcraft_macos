#pragma once
#include <stdint.h>

#include <string>

class LevelType;
class Abilities;
class LevelData;

#define _ADVENTURE_MODE_ENABLED


class GameType {
public:
    static GameType* NOT_SET;
    static GameType* SURVIVAL;
    static GameType* CREATIVE;
    static GameType* ADVENTURE;

    static void staticCtor();

private:
    int id;
    std::wstring name;

    GameType(int id, const std::wstring& name);

public:
    int getId();
    std::wstring getName();
    void updatePlayerAbilities(Abilities* abilities);
    bool isAdventureRestricted();
    bool isCreative();
    bool isSurvival();
    static GameType* byId(int id);
    static GameType* byName(const std::wstring& name);
};

class LevelSettings {
private:
    int64_t seed;
    GameType* gameType;
    bool generateMapFeatures;
    bool hardcore;
    bool newSeaLevel;
    LevelType* levelType;
    bool allowCommands;
    bool startingBonusItems;  
    std::wstring levelTypeOptions;
    int m_xzSize;  
    int m_hellScale;

    void _init(int64_t seed, GameType* gameType, bool generateMapFeatures,
               bool hardcore, bool newSeaLevel, LevelType* levelType,
               int xzSize,
               int hellScale);  

public:
    LevelSettings(int64_t seed, GameType* gameType, bool generateMapFeatures,
                  bool hardcore, bool newSeaLevel, LevelType* levelType,
                  int xzSize,
                  int hellScale);  
    LevelSettings(LevelData* levelData);
    LevelSettings*
    enableStartingBonusItems();  
    LevelSettings* enableSinglePlayerCommands();
    LevelSettings* setLevelTypeOptions(const std::wstring& options);
    bool hasStartingBonusItems();  
    int64_t getSeed();
    GameType* getGameType();
    bool isHardcore();
    LevelType* getLevelType();
    bool getAllowCommands();
    bool isGenerateMapFeatures();
    bool useNewSeaLevel();
    int getXZSize();     
    int getHellScale();  
    static GameType* validateGameType(int gameType);
    std::wstring getLevelTypeOptions();
};
