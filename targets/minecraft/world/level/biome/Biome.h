#pragma once

#include <stdint.h>

#include <format>
#include <string>
#include <vector>

#include "app/common/App_enums.h"
#include "java/Class.h"
#include "minecraft/util/WeighedRandom.h"
#include "minecraft/world/entity/Mob.h"
#include "minecraft/world/level/LevelSource.h"

class Feature;
class MobCategory;
class BiomeDecorator;
class TreeFeature;
class BasicTree;
class BirchFeature;
class SwampTreeFeature;
class ChunkRebuildData;
class Level;
class Random;

class Biome {
    friend class ChunkRebuildData;

public:
    
    static void staticCtor();

    static Biome* biomes[256];

    static Biome* ocean;
    static Biome* plains;
    static Biome* desert;
    static Biome* extremeHills;
    static Biome* forest;
    static Biome* taiga;
    static Biome* swampland;
    static Biome* river;
    static Biome* hell;
    static Biome* sky;
    static Biome* frozenOcean;
    static Biome* frozenRiver;
    static Biome* iceFlats;
    static Biome* iceMountains;
    static Biome* mushroomIsland;
    static Biome* mushroomIslandShore;
    static Biome* beaches;
    static Biome* desertHills;
    static Biome* forestHills;
    static Biome* taigaHills;
    static Biome* smallerExtremeHills;
    static Biome* jungle;
    static Biome* jungleHills;

    static const int BIOME_COUNT = 23;  

public:
    std::wstring m_name;
    int color;
    uint8_t topMaterial;
    uint8_t material;
    int leafColor;
    float depth;
    float scale;
    float temperature;
    float downfall;
    

    BiomeDecorator* decorator;

    const int id;

    class MobSpawnerData : public WeighedRandomItem {
    public:
        eINSTANCEOF mobClass;
        int minCount;
        int maxCount;

        MobSpawnerData(eINSTANCEOF mobClass, int probabilityWeight,
                       int minCount, int maxCount)
            : WeighedRandomItem(probabilityWeight) {
            this->mobClass = mobClass;
            this->minCount = minCount;
            this->maxCount = maxCount;
        }
    };

protected:
    std::vector<MobSpawnerData*> enemies;
    std::vector<MobSpawnerData*> friendlies;
    std::vector<MobSpawnerData*> waterFriendlies;
    std::vector<MobSpawnerData*> friendlies_chicken;
    std::vector<MobSpawnerData*> friendlies_wolf;
    std::vector<MobSpawnerData*> friendlies_mushroomcow;
    std::vector<MobSpawnerData*> ambientFriendlies;

    Biome(int id);
    ~Biome();

    BiomeDecorator* createDecorator();

private:
    Biome* setTemperatureAndDownfall(float temp, float downfall);
    Biome* setDepthAndScale(float depth, float scale);

    bool snowCovered;
    bool _hasRain;

    
    eMinecraftColour m_grassColor;
    eMinecraftColour m_foliageColor;
    eMinecraftColour m_waterColor;
    eMinecraftColour m_skyColor;

    Biome* setNoRain();

protected:
    





public:
    virtual Feature* getTreeFeature(Random* random);
    virtual Feature* getGrassFeature(Random* random);

protected:
    Biome* setSnowCovered();
    Biome* setName(const std::wstring& name);
    Biome* setLeafColor(int leafColor);
    Biome* setColor(int color);

    
    Biome* setLeafFoliageWaterSkyColor(eMinecraftColour grassColor,
                                       eMinecraftColour foliageColor,
                                       eMinecraftColour waterColour,
                                       eMinecraftColour skyColour);

public:
    virtual int getSkyColor(float temp);

    std::vector<MobSpawnerData*>* getMobs(MobCategory* category);

    virtual bool hasSnow();
    virtual bool hasRain();
    virtual bool isHumid();

    virtual float getCreatureProbability();
    virtual int getDownfallInt();
    virtual int getTemperatureInt();
    virtual float getDownfall();     
    virtual float getTemperature();  

    virtual void decorate(Level* level, Random* random, int xo, int zo);

    virtual int getGrassColor();
    virtual int getFolageColor();
    virtual int getWaterColor();  
};