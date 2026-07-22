#include "SquidModel.h"

#include <math.h>

#include <memory>
#include <numbers>

#include "minecraft/client/model/geom/Model.h"
#include "minecraft/client/model/geom/ModelPart.h"

SquidModel::SquidModel() : Model() {
    int yoffs = -16;
    body = new ModelPart(this, 0, 0);
    body->addBox(-6, -8, -6, 12, 16, 12);
    body->y += (8 + 16) + yoffs;

    for (int i = 0; i < TENTACLES_LENGTH; i++)  
    {
        tentacles[i] = new ModelPart(this, 48, 0);

        double angle = i * std::numbers::pi * 2.0 /
                       (double)TENTACLES_LENGTH;  
        float xo = cosf((float)angle) * 5;
        float yo = sinf((float)angle) * 5;
        tentacles[i]->addBox(-1, 0, -1, 2, 18, 2);

        tentacles[i]->x = xo;
        tentacles[i]->z = yo;
        tentacles[i]->y = (float)(31 + yoffs);

        angle = i * std::numbers::pi * -2.0 / (double)TENTACLES_LENGTH +
                std::numbers::pi * .5;  
        tentacles[i]->yRot = (float)angle;

        
        
        tentacles[i]->compile(1.0f / 16.0f);
    }
    body->compile(1.0f / 16.0f);
}

void SquidModel::setupAnim(float time, float r, float bob, float yRot,
                           float xRot, float scale,
                           std::shared_ptr<Entity> entity,
                           unsigned int uiBitmaskOverrideAnim) {
    for (int i = 0; i < TENTACLES_LENGTH; i++)  
    {
        
        tentacles[i]->xRot = bob;
    }
}

void SquidModel::render(std::shared_ptr<Entity> entity, float time, float r,
                        float bob, float yRot, float xRot, float scale,
                        bool usecompiled) {
    setupAnim(time, r, bob, yRot, xRot, scale, entity);

    body->render(scale, usecompiled);
    for (int i = 0; i < TENTACLES_LENGTH;
         i++)  
               
    {
        tentacles[i]->render(scale, usecompiled);
    }
}