#pragma once
#include "Minecraft.World/net/minecraft/world/level/chunk/ChunkSource.h"

class Level;

class LargeFeature {
public:
    static const std::wstring STRONGHOLD;

protected:
    int radius;
    Random* random;
    Level* level;

public:
    LargeFeature();
    virtual ~LargeFeature();

    virtual void apply(ChunkSource* ChunkSource, Level* level, int xOffs,
                       int zOffs, std::vector<uint8_t>& blocks);

protected:
    virtual void addFeature(Level* level, int x, int z, int xOffs, int zOffs,
                            std::vector<uint8_t>& blocks) {}
};