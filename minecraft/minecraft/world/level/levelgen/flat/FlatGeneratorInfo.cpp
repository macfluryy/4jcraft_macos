#include "FlatGeneratorInfo.h"
#include "console_helpers/StringHelpers.h"
#include "minecraft/world/level/biome/Biome.h"
#include "minecraft/world/level/levelgen/flat/FlatLayerInfo.h"
#include "minecraft/world/level/tile/Tile.h"

const std::wstring FlatGeneratorInfo::STRUCTURE_VILLAGE = L"village";
const std::wstring FlatGeneratorInfo::STRUCTURE_BIOME_SPECIFIC = L"biome_1";
const std::wstring FlatGeneratorInfo::STRUCTURE_STRONGHOLD = L"stronghold";
const std::wstring FlatGeneratorInfo::STRUCTURE_MINESHAFT = L"mineshaft";
const std::wstring FlatGeneratorInfo::STRUCTURE_BIOME_DECORATION =
    L"decoration";
const std::wstring FlatGeneratorInfo::STRUCTURE_LAKE = L"lake";
const std::wstring FlatGeneratorInfo::STRUCTURE_LAVA_LAKE = L"lava_lake";
const std::wstring FlatGeneratorInfo::STRUCTURE_DUNGEON = L"dungeon";

FlatGeneratorInfo::FlatGeneratorInfo() { biome = 0; }

FlatGeneratorInfo::~FlatGeneratorInfo() {
    for (auto it = layers.begin(); it != layers.end(); ++it) {
        delete *it;
    }
}

int FlatGeneratorInfo::getBiome() { return biome; }

void FlatGeneratorInfo::setBiome(int biome) { this->biome = biome; }

std::unordered_map<std::wstring,
                   std::unordered_map<std::wstring, std::wstring> >*
FlatGeneratorInfo::getStructures() {
    return &structures;
}

std::vector<FlatLayerInfo*>* FlatGeneratorInfo::getLayers() { return &layers; }

void FlatGeneratorInfo::updateLayers() {
    int y = 0;

    for (auto it = layers.begin(); it != layers.end(); ++it) {
        FlatLayerInfo* layer = *it;
        layer->setStart(y);
        y += layer->getHeight();
    }
}

std::wstring FlatGeneratorInfo::toString() { return L""; }

FlatLayerInfo* FlatGeneratorInfo::getLayerFromString(const std::wstring& input,
                                                     int yOffset) {
    return nullptr;
}

std::vector<FlatLayerInfo*>* FlatGeneratorInfo::getLayersFromString(
    const std::wstring& input) {
    if (input.empty()) return nullptr;

    std::vector<FlatLayerInfo*>* result = new std::vector<FlatLayerInfo*>();
    std::vector<std::wstring> depths = stringSplit(input, L',');

    int yOffset = 0;

    for (auto it = depths.begin(); it != depths.end(); ++it) {
        FlatLayerInfo* layer = getLayerFromString(*it, yOffset);
        if (layer == nullptr) return nullptr;
        result->push_back(layer);
        yOffset += layer->getHeight();
    }

    return result;
}

FlatGeneratorInfo* FlatGeneratorInfo::fromValue(const std::wstring& input) {
    return getDefault();
}

FlatGeneratorInfo* FlatGeneratorInfo::getDefault() {
    FlatGeneratorInfo* result = new FlatGeneratorInfo();

    result->setBiome(Biome::plains->id);
    result->getLayers()->push_back(new FlatLayerInfo(1, Tile::unbreakable_Id));
    result->getLayers()->push_back(new FlatLayerInfo(2, Tile::dirt_Id));
    result->getLayers()->push_back(new FlatLayerInfo(1, Tile::grass_Id));
    result->updateLayers();
    (*(result->getStructures()))[STRUCTURE_VILLAGE] =
        std::unordered_map<std::wstring, std::wstring>();

    return result;
}