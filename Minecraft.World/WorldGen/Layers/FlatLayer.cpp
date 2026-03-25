#include "../../Platform/stdafx.h"
#include "../../Headers/net.minecraft.world.level.newbiome.layer.h"

FlatLayer::FlatLayer(int val) : Layer(0) { this->val = val; }

intArray FlatLayer::getArea(int xo, int yo, int w, int h) {
    intArray result{static_cast<unsigned int>(w * h)};
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            result[x + y * w] = val;
        }
    }
    return result;
}
