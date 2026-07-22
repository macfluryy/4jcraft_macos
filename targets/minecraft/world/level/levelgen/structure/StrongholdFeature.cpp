#include "StrongholdFeature.h"

#include <assert.h>
#include <math.h>

#include <list>
#include <numbers>
#include <utility>
#include <vector>

#include "app/common/App_enums.h"
#include "app/common/src/GameRules/LevelGeneration/LevelGenerationOptions.h"
#include "app/mac/MacGame.h"
#include "app/mac/Stubs/winapi_stubs.h"
#include "StrongholdPieces.h"
#include "java/JavaMath.h"
#include "java/Random.h"
#include "minecraft/util/Mth.h"
#include "minecraft/world/level/ChunkPos.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/TilePos.h"
#include "minecraft/world/level/biome/Biome.h"
#include "minecraft/world/level/biome/BiomeSource.h"
#include "minecraft/world/level/dimension/Dimension.h"
#include "minecraft/world/level/levelgen/LargeFeature.h"
#include "minecraft/world/level/levelgen/structure/StructureFeature.h"
#include "minecraft/world/level/levelgen/structure/StructurePiece.h"
#include "minecraft/world/level/levelgen/structure/StructureStart.h"

const std::wstring StrongholdFeature::OPTION_DISTANCE = L"distance";
const std::wstring StrongholdFeature::OPTION_COUNT = L"count";
const std::wstring StrongholdFeature::OPTION_SPREAD = L"spread";

std::vector<Biome*> StrongholdFeature::allowedBiomes;

void StrongholdFeature::staticCtor() {
    allowedBiomes.push_back(Biome::desert);
    allowedBiomes.push_back(Biome::forest);
    allowedBiomes.push_back(Biome::extremeHills);
    allowedBiomes.push_back(Biome::swampland);
    allowedBiomes.push_back(Biome::taiga);
    allowedBiomes.push_back(Biome::iceFlats);
    allowedBiomes.push_back(Biome::iceMountains);
    allowedBiomes.push_back(Biome::desertHills);
    allowedBiomes.push_back(Biome::forestHills);
    allowedBiomes.push_back(Biome::smallerExtremeHills);
    allowedBiomes.push_back(Biome::taigaHills);
    allowedBiomes.push_back(Biome::jungle);
    allowedBiomes.push_back(Biome::jungleHills);
};

void StrongholdFeature::_init() {
    distance = 32;
    spread = 3;

    
    for (int i = 0; i < strongholdPos_length; i++) {
        strongholdPos[i] = nullptr;
    }
    isSpotSelected = false;
}

StrongholdFeature::StrongholdFeature() : StructureFeature() { _init(); }

StrongholdFeature::StrongholdFeature(
    std::unordered_map<std::wstring, std::wstring> options) {
    _init();

    for (auto it = options.begin(); it != options.end(); ++it) {
        if (it->first.compare(OPTION_DISTANCE) == 0) {
            distance = Mth::getDouble(it->second, distance, 1);
        } else if (it->first.compare(OPTION_COUNT) == 0) {
            
            
            
            assert(false);
        } else if (it->first.compare(OPTION_SPREAD) == 0) {
            spread = Mth::getInt(it->second, spread, 1);
        }
    }
}

StrongholdFeature::~StrongholdFeature() {
    for (int i = 0; i < strongholdPos_length; i++) {
        delete strongholdPos[i];
    }
}

std::wstring StrongholdFeature::getFeatureName() {
    return LargeFeature::STRONGHOLD;
}

bool StrongholdFeature::isFeatureChunk(int x, int z, bool bIsSuperflat) {
    if (!isSpotSelected) {
        Random random;

        random.setSeed(level->getSeed());
        double angle = random.nextDouble() * std::numbers::pi * 2.0;
        int circle = 1;

        
        
        bool hasFoundValidPos = false;
        int findAttempts = 0;
        do {
            for (int i = 0; i < strongholdPos_length; i++) {
                double dist = 0.0;
#ifdef _LARGE_WORLDS
                if (level->dimension->getXZSize() < (2.25f * 32.0f)) {
                    
                    dist =
                        (1.25 + random.nextDouble()) * (3 + random.nextInt(4));
                } else {
                    
                    dist = (1.25 * circle + random.nextDouble()) *
                           (distance * circle);
                }
#else
                
                
                
                
                
                
                
                
                
                
                
                if (level->getOriginalSaveVersion() >=
                    SAVE_FILE_VERSION_MOVED_STRONGHOLD) {
                    
                    
                    
                    
                    
                    dist =
                        (1.25 + random.nextDouble()) * (3 + random.nextInt(4));
                } else {
                    
                    dist = (1.25 + random.nextDouble()) *
                           (5.0 + random.nextInt(7));
                }
#endif

                int selectedX = (int)(Math::round(cos(angle) * dist));
                int selectedZ = (int)(Math::round(sin(angle) * dist));

                TilePos* position = level->getBiomeSource()->findBiome(
                    ((unsigned int)selectedX << 4) + 8,
                    ((unsigned int)selectedZ << 4) + 8, 7 << 4, allowedBiomes,
                    &random);
                if (position != nullptr) {
                    selectedX = position->x >> 4;
                    selectedZ = position->z >> 4;

#ifndef _CONTENT_PACKAGE
                    if (position->x > 2560 || position->x < -2560 ||
                        position->z > 2560 || position->z < -2560) {
                        __debugbreak();
                    }
#endif

                    app.DebugPrintf(
                        "Placed stronghold in valid biome at (%d, %d), (%d, "
                        "%d)\n",
                        selectedX, selectedZ, position->x, position->z);
                    
                    app.AddTerrainFeaturePosition(eTerrainFeature_Stronghold,
                                                  selectedX, selectedZ);

                    
                    hasFoundValidPos = true;
                    delete position;
                }

                delete strongholdPos[i];
                strongholdPos[i] = new ChunkPos(selectedX, selectedZ);

                angle += std::numbers::pi * 2.0 / (double)strongholdPos_length;
            }

            
            
            ++findAttempts;

            
#ifdef _LARGE_WORLDS
            angle = random.nextDouble() * std::numbers::pi * 2.0 * circle /
                    (double)spread;
#endif
        } while (!hasFoundValidPos && findAttempts < MAX_STRONGHOLD_ATTEMPTS);

        if (!hasFoundValidPos) {
            
            
            
            
            app.AddTerrainFeaturePosition(eTerrainFeature_Stronghold,
                                          strongholdPos[0]->x,
                                          strongholdPos[0]->z);
        }

        isSpotSelected = true;
    }

    for (int i = 0; i < strongholdPos_length; i++) {
        bool forcePlacement = false;
        LevelGenerationOptions* levelGenOptions =
            app.getLevelGenerationOptions();
        if (levelGenOptions != nullptr) {
            forcePlacement =
                levelGenOptions->isFeatureChunk(x, z, eFeature_Stronghold);
        }

        ChunkPos* pos = strongholdPos[i];
        if (forcePlacement || (pos && x == pos->x && z == pos->z)) {
            return true;
        }
    }
    return false;
}

std::vector<TilePos>* StrongholdFeature::getGuesstimatedFeaturePositions() {
    std::vector<TilePos>* positions = new std::vector<TilePos>();
    for (int i = 0; i < strongholdPos_length; i++) {
        ChunkPos* chunkPos = strongholdPos[i];
        if (chunkPos != nullptr) {
            positions->push_back(chunkPos->getMiddleBlockPosition(64));
        }
    }
    return positions;
}

StructureStart* StrongholdFeature::createStructureStart(int x, int z) {
    StrongholdStart* start = new StrongholdStart(level, random, x, z);

    
    while (start->getPieces()->empty() ||
           ((StrongholdPieces::StartPiece*)start->getPieces()->front())
                   ->portalRoomPiece == nullptr) {
        delete start;
        
        start = new StrongholdStart(level, random, x, z);
    }

    return start;

    
    
}

StrongholdFeature::StrongholdStart::StrongholdStart() {
    
}

StrongholdFeature::StrongholdStart::StrongholdStart(Level* level,
                                                    Random* random, int chunkX,
                                                    int chunkZ)
    : StructureStart(chunkX, chunkZ) {
    StrongholdPieces::resetPieces();

    StrongholdPieces::StartPiece* startRoom = new StrongholdPieces::StartPiece(
        0, random, (chunkX << 4) + 2, (chunkZ << 4) + 2, level);
    pieces.push_back(startRoom);
    startRoom->addChildren(startRoom, &pieces, random);

    std::vector<StructurePiece*>* pendingChildren = &startRoom->pendingChildren;
    while (!pendingChildren->empty()) {
        int pos = random->nextInt((int)pendingChildren->size());
        auto it = pendingChildren->begin() + pos;
        StructurePiece* structurePiece = *it;
        pendingChildren->erase(it);
        structurePiece->addChildren(startRoom, &pieces, random);
    }

    calculateBoundingBox();
    moveBelowSeaLevel(level, random, 10);
}