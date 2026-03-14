#pragma once

#include "Layer.h"

class ZoomLayer : public Layer {
public:
    ZoomLayer(__int64 seedMixup, std::shared_ptr<Layer> parent);

    virtual intArray getArea(int xo, int yo, int w, int h);

protected:
    int random(int a, int b);
    int random(int a, int b, int c, int d);

public:
    static std::shared_ptr<Layer> zoom(__int64 seed, std::shared_ptr<Layer> sup,
                                       int count);
};