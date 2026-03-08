#pragma once
#include "Layer.h"

class ShoreLayer : public Layer
{
public:
    ShoreLayer(__int64 seed, std::shared_ptr<Layer> parent);
    virtual intArray getArea(int xo, int yo, int w, int h);
};