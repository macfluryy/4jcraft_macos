#pragma once

#include <stdint.h>

#include <format>
#include <memory>
#include <string>
#include <vector>

#include "minecraft/world/level/GameRules.h"

class Player;
class CompoundTag;
class LevelSettings;
class LevelType;
class GameType;

class LevelData {
    friend class DerivedLevelData;

private:
    int64_t seed;
    LevelType* m_pGenerator;  
    std::wstring generatorOptions;
    int xSpawn;
    int ySpawn;
    int zSpawn;
    int64_t gameTime;
    int64_t dayTime;
    int64_t lastPlayed;
    int64_t sizeOnDisk;
    
    int dimension;
    std::wstring levelName;
    int version;

    bool raining;
    int rainTime;

    bool thundering;
    int thunderTime;
    GameType* gameType;
    bool generateMapFeatures;
    bool hardcore;
    bool allowCommands;
    bool initialized;
    bool newSeaLevel;        
    bool hasBeenInCreative;  
    
    
    
    
    
    unsigned int m_gameHostSettings = 0;  
    bool spawnBonusChest;    
    int m_xzSize;            
#ifdef _LARGE_WORLDS
    int m_xzSizeOld;  
    int m_hellScaleOld;
    bool m_classicEdgeMoat;
    bool m_smallEdgeMoat;
    bool m_mediumEdgeMoat;
#endif
    int m_hellScale;  

    
    int xStronghold;
    int yStronghold;
    int zStronghold;
    bool bStronghold;

    int xStrongholdEndPortal;
    int zStrongholdEndPortal;
    bool bStrongholdEndPortal;

    GameRules gameRules;

protected:
    LevelData();

public:
    LevelData(CompoundTag* tag);
    LevelData(LevelSettings* levelSettings, const std::wstring& levelName);
    LevelData(LevelData* copy);
    CompoundTag* createTag();
    CompoundTag* createTag(std::vector<std::shared_ptr<Player> >* players);

    enum { DIMENSION_NETHER = -1, DIMENSION_OVERWORLD = 0, DIMENSION_END = 1 };

protected:
    virtual void setTagData(
        CompoundTag* tag);  

public:
    virtual int64_t getSeed();
    virtual int getXSpawn();
    virtual int getYSpawn();
    virtual int getZSpawn();
    virtual int getXStronghold();
    virtual int getZStronghold();
    virtual int getXStrongholdEndPortal();
    virtual int getZStrongholdEndPortal();
    virtual int64_t getGameTime();
    virtual int64_t getDayTime();
    virtual int64_t getSizeOnDisk();
    virtual CompoundTag* getLoadedPlayerTag();
    
    virtual void setSeed(int64_t seed);
    virtual void setXSpawn(int xSpawn);
    virtual void setYSpawn(int ySpawn);
    virtual void setZSpawn(int zSpawn);
    virtual void setHasStronghold();
    virtual bool getHasStronghold();
    virtual void setXStronghold(int xStronghold);
    virtual void setZStronghold(int zStronghold);
    virtual void setHasStrongholdEndPortal();
    virtual bool getHasStrongholdEndPortal();
    virtual void setXStrongholdEndPortal(int xStrongholdEndPortal);
    virtual void setZStrongholdEndPortal(int zStrongholdEndPortal);

    virtual void setGameTime(int64_t time);
    virtual void setDayTime(int64_t time);
    virtual void setSizeOnDisk(int64_t sizeOnDisk);
    virtual void setLoadedPlayerTag(CompoundTag* loadedPlayerTag);
    
    virtual void setSpawn(int xSpawn, int ySpawn, int zSpawn);
    virtual std::wstring getLevelName();
    virtual void setLevelName(const std::wstring& levelName);
    virtual int getVersion();
    virtual void setVersion(int version);
    virtual int64_t getLastPlayed();
    virtual bool isThundering();
    virtual void setThundering(bool thundering);
    virtual int getThunderTime();
    virtual void setThunderTime(int thunderTime);
    virtual bool isRaining();
    virtual void setRaining(bool raining);
    virtual int getRainTime();
    virtual void setRainTime(int rainTime);
    virtual GameType* getGameType();
    virtual bool isGenerateMapFeatures();
    virtual bool getSpawnBonusChest();
    virtual void setGameType(GameType* gameType);
    virtual bool useNewSeaLevel();
    virtual bool getHasBeenInCreative();            
    virtual void setHasBeenInCreative(bool value);  
    
    unsigned int getGameHostSettings() { return m_gameHostSettings; }
    void setGameHostSettings(unsigned int v) { m_gameHostSettings = v; }
    virtual LevelType* getGenerator();
    virtual void setGenerator(LevelType* generator);
    virtual std::wstring getGeneratorOptions();
    virtual void setGeneratorOptions(const std::wstring& options);
    virtual bool isHardcore();
    virtual bool getAllowCommands();
    virtual void setAllowCommands(bool allowCommands);
    virtual bool isInitialized();
    virtual void setInitialized(bool initialized);
    virtual GameRules* getGameRules();
    virtual int getXZSize();  
#ifdef _LARGE_WORLDS
    virtual int getXZSizeOld();  
    virtual void getMoatFlags(bool* bClassicEdgeMoat, bool* bSmallEdgeMoat,
                              bool* bMediumEdgeMoat);  
    virtual int getXZHellSizeOld();                    

#endif
    virtual int getHellScale();  
};
