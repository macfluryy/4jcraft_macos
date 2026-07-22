#pragma once

#include <format>
#include <string>
#include <unordered_map>
#include <vector>

#include "StructureFeature.h"
#include "StructureStart.h"
#include "minecraft/world/level/ChunkPos.h"
#include "minecraft/world/level/levelgen/structure/StructureFeature.h"
#include "minecraft/world/level/levelgen/structure/StructureFeatureIO.h"

class Biome;
class ChunkPos;
class Level;
class Random;
class TilePos;



#ifdef _LARGE_WORLDS
#define MAX_STRONGHOLD_ATTEMPTS 30
#else
#define MAX_STRONGHOLD_ATTEMPTS 10
#endif

class StrongholdFeature : public StructureFeature {
public:
    static const std::wstring OPTION_DISTANCE;
    static const std::wstring OPTION_COUNT;
    static const std::wstring OPTION_SPREAD;

public:
    static void staticCtor();

private:
    static std::vector<Biome*> allowedBiomes;

    bool isSpotSelected;
    static const int strongholdPos_length =
        1;  
            
    ChunkPos* strongholdPos[strongholdPos_length];
    double distance;
    int spread;

    void _init();

public:
    StrongholdFeature();
    StrongholdFeature(std::unordered_map<std::wstring, std::wstring> options);
    ~StrongholdFeature();

    std::wstring getFeatureName();

protected:
    virtual bool isFeatureChunk(int x, int z, bool bIsSuperflat = false);
    std::vector<TilePos>* getGuesstimatedFeaturePositions();
    virtual StructureStart* createStructureStart(int x, int z);

public:
    class StrongholdStart : public StructureStart {
    public:
        static StructureStart* Create() { return new StrongholdStart(); }
        virtual EStructureStart GetType() {
            return eStructureStart_StrongholdStart;
        }

    public:
        StrongholdStart();
        StrongholdStart(Level* level, Random* random, int chunkX, int chunkZ);
    };
};
