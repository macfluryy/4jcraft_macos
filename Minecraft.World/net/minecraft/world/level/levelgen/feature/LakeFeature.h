#pragma once
#include "Feature.h"
#include "../../material/Material.h"

class LakeFeature : public Feature {
private:
    int tile;

public:
    LakeFeature(int tile);

    virtual bool place(Level* level, Random* random, int x, int y, int z);
};