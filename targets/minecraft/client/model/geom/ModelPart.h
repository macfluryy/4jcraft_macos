#pragma once
#include <string>
#include <vector>

#include "app/include/SkinBox.h"
#include "Model.h"
#include "minecraft/client/model/Polygon.h"
#include "minecraft/client/model/Vertex.h"

class Cube;
class Model;

class ModelPart {
public:
    float xTexSize;
    float yTexSize;
    float x, y, z;
    float xRot, yRot, zRot;
    bool bMirror;
    bool visible;
    bool neverRender;
    std::vector<Cube*> cubes;
    std::vector<ModelPart*> children;
    static const float RAD;
    float translateX, translateY, translateZ;

private:
    std::wstring id;
    int xTexOffs, yTexOffs;
    bool compiled;
    int list;
    Model* model;

public:
    void _init();  
    ModelPart();
    ModelPart(Model* model, const std::wstring& id);
    ModelPart(Model* model);
    ModelPart(Model* model, int xTexOffs, int yTexOffs);

    
    
    
    void construct(Model* model, const std::wstring& id);
    void construct(Model* model);
    void construct(Model* model, int xTexOffs, int yTexOffs);

    void addChild(ModelPart* child);
    ModelPart* retrieveChild(SKIN_BOX* pBox);
    ModelPart* mirror();
    ModelPart* texOffs(int xTexOffs, int yTexOffs);
    ModelPart* addBox(std::wstring id, float x0, float y0, float z0, int w,
                      int h, int d);
    ModelPart* addBox(float x0, float y0, float z0, int w, int h, int d);
    ModelPart* addBoxWithMask(float x0, float y0, float z0, int w, int h, int d,
                              int faceMask);  
    void addBox(float x0, float y0, float z0, int w, int h, int d, float g);
    void addHumanoidBox(
        float x0, float y0, float z0, int w, int h, int d,
        float g);  
    void addTexBox(float x0, float y0, float z0, int w, int h, int d, int tex);
    void setPos(float x, float y, float z);
    void render(float scale, bool usecompiled,
                bool bHideParentBodyPart = false);
    void renderRollable(float scale, bool usecompiled);
    void translateTo(float scale);
    ModelPart* setTexSize(int xs, int ys);
    void mimic(ModelPart* o);
    void compile(float scale);
    int getfU() { return xTexOffs; }
    int getfV() { return yTexOffs; }
};
