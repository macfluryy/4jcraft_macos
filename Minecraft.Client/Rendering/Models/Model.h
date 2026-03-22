#pragma once

#include "../../../Minecraft.World/Util/Random.h"
#include "../Minecraft.Client/SkinBox.h"
class Mob;
class ModelPart;
class TexOffs;
class LivingEntity;

class Model {
public:
    float attackTime;
    bool riding;
    std::vector<ModelPart*> cubes;
    bool young;
    std::unordered_map<std::wstring, TexOffs*> mappedTexOffs;
    int texWidth;
    int texHeight;

    Model();  // 4J added
    virtual void render(std::shared_ptr<Entity> entity, float time, float r,
                        float bob, float yRot, float xRot, float scale,
                        bool usecompiled) {}
    virtual void setupAnim(float time, float r, float bob, float yRot,
                           float xRot, float scale,
                           std::shared_ptr<Entity> entity,
                           unsigned int uiBitmaskOverrideAnim = 0) {}
    virtual void prepareMobModel(std::shared_ptr<LivingEntity> mob, float time,
                                 float r, float a) {}
    virtual ModelPart* getRandomModelPart(Random random) {
        return cubes.at(random.nextInt((int)cubes.size()));
    }
    virtual ModelPart* AddOrRetrievePart(SKIN_BOX* pBox) { return NULL; }

    void setMapTex(std::wstring id, int x, int y);
    TexOffs* getMapTex(std::wstring id);

protected:
    float yHeadOffs;
    float zHeadOffs;
};
