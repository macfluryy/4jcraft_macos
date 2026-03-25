#pragma once

#include "Layer.h"

class SwampRiversLayer : public Layer {
public:
    SwampRiversLayer(int64_t seed, std::shared_ptr<Layer> parent);

    intArray getArea(int xo, int yo, int w, int h);
};