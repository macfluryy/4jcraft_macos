#include "../../../../../../Header Files/stdafx.h"
#include "net.minecraft.world.level.newbiome.layer.h"

FlatLayer::FlatLayer(int val) : Layer(0) { this->val = val; }

std::vector<int> FlatLayer::getArea(int xo, int yo, int w, int h) {
    std::vector<int> result(w * h);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            result[x + y * w] = val;
        }
    }
    return result;
}
