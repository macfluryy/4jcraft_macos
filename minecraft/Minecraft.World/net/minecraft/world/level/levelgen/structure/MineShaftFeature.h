#pragma once

#include <string>
#include <unordered_map>

#include "StructureFeature.h"
#include "Minecraft.World/net/minecraft/world/level/levelgen/structure/StructureFeature.h"

class MineShaftFeature : public StructureFeature {
public:
    static const std::wstring OPTION_CHANCE;

private:
    double chance;

public:
    MineShaftFeature();

    std::wstring getFeatureName();

    MineShaftFeature(std::unordered_map<std::wstring, std::wstring> options);

protected:
    virtual bool isFeatureChunk(int x, int z, bool bIsSuperflat = false);
    virtual StructureStart* createStructureStart(int x, int z);
};