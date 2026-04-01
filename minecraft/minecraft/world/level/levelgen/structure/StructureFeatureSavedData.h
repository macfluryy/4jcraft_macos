#pragma once

#include "minecraft/world/level/saveddata/SavedData.h"

class StructureFeatureSavedData : public SavedData {
private:
    static std::wstring TAG_FEATURES;
    CompoundTag* pieceTags;

public:
    StructureFeatureSavedData(const std::wstring& idName);
    ~StructureFeatureSavedData();

    void load(CompoundTag* tag);
    void save(CompoundTag* tag);
    CompoundTag* getFeatureTag(int chunkX, int chunkZ);
    void putFeatureTag(CompoundTag* tag, int chunkX, int chunkZ);
    std::wstring createFeatureTagId(int chunkX, int chunkZ);
    CompoundTag* getFullTag();
};