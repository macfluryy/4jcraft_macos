#include "TexOffs.h"
#include "Model.h"
#include "Minecraft.Client/net/minecraft/client/model/geom/Model.h"

Model::Model() {
    riding = false;
    young = true;
    texWidth = 64;
    texHeight = 32;
}

void Model::setMapTex(std::wstring id, int x, int y) {
    mappedTexOffs[id] = new TexOffs(x, y);
}

TexOffs* Model::getMapTex(std::wstring id) {
    // 4J-PB - assuming there will always be this one
    return mappedTexOffs[id];
}