#pragma once
#include "Layer.h"

class ShoreLayer : public Layer {
public:
    ShoreLayer(int64_t seed, std::shared_ptr<Layer> parent);
    virtual intArray getArea(int xo, int yo, int w, int h);
};