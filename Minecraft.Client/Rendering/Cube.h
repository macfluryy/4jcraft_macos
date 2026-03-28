#pragma once
#include <array>
#include "../../Minecraft.World/Util/ArrayWithLength.h"
#include "Vertex.h"
#include "Polygon.h"

class Model;

class Cube {
private:
    std::array<Vertex, 8> vertices;
    PolygonArray polygons;

public:
    const float x0, y0, z0, x1, y1, z1;
    std::wstring id;

public:
    Cube(ModelPart* modelPart, int xTexOffs, int yTexOffs, float x0, float y0,
         float z0, int w, int h, int d, float g, int faceMask = 63,
         bool bFlipPoly3UVs = false);  // 4J - added faceMask

    void render(Tesselator* t, float scale);
    Cube* setId(const std::wstring& id);
};
