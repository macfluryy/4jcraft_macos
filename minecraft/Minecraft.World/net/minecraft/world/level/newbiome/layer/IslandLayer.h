#pragma once

#include "Layer.h"

class IslandLayer : public Layer {
public:
    IslandLayer(int64_t seedMixup);

    std::vector<int> getArea(int xo, int yo, int w, int h);
};