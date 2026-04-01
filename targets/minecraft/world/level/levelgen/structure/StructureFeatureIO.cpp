#include "StructureFeatureIO.h"

#include <string>
#include <unordered_map>
#include <utility>

#include "Minecraft.Client/Linux/Linux_App.h"
#include "minecraft/world/level/levelgen/structure/MineShaftPieces.h"
#include "minecraft/world/level/levelgen/structure/MineShaftStart.h"
#include "minecraft/world/level/levelgen/structure/NetherBridgeFeature.h"
#include "minecraft/world/level/levelgen/structure/NetherBridgePieces.h"
#include "minecraft/world/level/levelgen/structure/RandomScatteredLargeFeature.h"
#include "minecraft/world/level/levelgen/structure/ScatteredFeaturePieces.h"
#include "minecraft/world/level/levelgen/structure/StrongholdFeature.h"
#include "minecraft/world/level/levelgen/structure/StrongholdPieces.h"
#include "minecraft/world/level/levelgen/structure/StructureFeatureIO.h"
#include "minecraft/world/level/levelgen/structure/StructurePiece.h"
#include "minecraft/world/level/levelgen/structure/StructureStart.h"
#include "minecraft/world/level/levelgen/structure/VillageFeature.h"
#include "minecraft/world/level/levelgen/structure/VillagePieces.h"
#include "nbt/CompoundTag.h"

class Level;

std::unordered_map<std::wstring, structureStartCreateFn>
    StructureFeatureIO::startIdClassMap;
std::unordered_map<unsigned int, std::wstring>
    StructureFeatureIO::startClassIdMap;

std::unordered_map<std::wstring, structurePieceCreateFn>
    StructureFeatureIO::pieceIdClassMap;
std::unordered_map<unsigned int, std::wstring>
    StructureFeatureIO::pieceClassIdMap;

void StructureFeatureIO::setStartId(EStructureStart clas,
                                    structureStartCreateFn createFn,
                                    const std::wstring& id) {
    startIdClassMap[id] = createFn;
    startClassIdMap[clas] = id;
}

void StructureFeatureIO::setPieceId(EStructurePiece clas,
                                    structurePieceCreateFn createFn,
                                    const std::wstring& id) {
    pieceIdClassMap[id] = createFn;
    pieceClassIdMap[clas] = id;
}

void StructureFeatureIO::staticCtor() {
    setStartId(eStructureStart_MineShaftStart, MineShaftStart::Create,
               L"Mineshaft");
    setStartId(eStructureStart_VillageStart,
               VillageFeature::VillageStart::Create, L"Village");
    setStartId(eStructureStart_NetherBridgeStart,
               NetherBridgeFeature::NetherBridgeStart::Create, L"Fortress");
    setStartId(eStructureStart_StrongholdStart,
               StrongholdFeature::StrongholdStart::Create, L"Stronghold");
    setStartId(eStructureStart_ScatteredFeatureStart,
               RandomScatteredLargeFeature::ScatteredFeatureStart::Create,
               L"Temple");

    MineShaftPieces::loadStatic();
    VillagePieces::loadStatic();
    NetherBridgePieces::loadStatic();
    StrongholdPieces::loadStatic();
    ScatteredFeaturePieces::loadStatic();
}

std::wstring StructureFeatureIO::getEncodeId(StructureStart* start) {
    auto it = startClassIdMap.find(start->GetType());
    if (it != startClassIdMap.end()) {
        return it->second;
    } else {
        return L"";
    }
}

std::wstring StructureFeatureIO::getEncodeId(StructurePiece* piece) {
    auto it = pieceClassIdMap.find(piece->GetType());
    if (it != pieceClassIdMap.end()) {
        return it->second;
    } else {
        return L"";
    }
}

StructureStart* StructureFeatureIO::loadStaticStart(CompoundTag* tag,
                                                    Level* level) {
    StructureStart* start = nullptr;

    auto it = startIdClassMap.find(tag->getString(L"id"));
    if (it != startIdClassMap.end()) {
        start = (it->second)();
    }

    if (start != nullptr) {
        start->load(level, tag);
    } else {
        app.DebugPrintf("Skipping Structure with id %ls",
                        tag->getString(L"id").c_str());
    }
    return start;
}

StructurePiece* StructureFeatureIO::loadStaticPiece(CompoundTag* tag,
                                                    Level* level) {
    StructurePiece* piece = nullptr;

    auto it = pieceIdClassMap.find(tag->getString(L"id"));
    if (it != pieceIdClassMap.end()) {
        piece = (it->second)();
    }

    if (piece != nullptr) {
        piece->load(level, tag);
    } else {
        app.DebugPrintf("Skipping Piece with id %ls",
                        tag->getString(L"id").c_str());
    }
    return piece;
}