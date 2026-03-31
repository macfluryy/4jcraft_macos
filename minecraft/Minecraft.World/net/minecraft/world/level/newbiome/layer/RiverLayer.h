#pragma once

#include "Layer.h"

class RiverLayer : public Layer {
public:
    RiverLayer(int64_t seedMixup, std::shared_ptr<Layer> parent);
    std::vector<int> getArea(int xo, int yo, int w, int h);
};