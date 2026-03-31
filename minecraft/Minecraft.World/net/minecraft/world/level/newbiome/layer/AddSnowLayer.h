#pragma once
#include "Layer.h"

class AddSnowLayer : public Layer {
public:
    AddSnowLayer(int64_t seedMixup, std::shared_ptr<Layer> parent);
    virtual std::vector<int> getArea(int xo, int yo, int w, int h);
};