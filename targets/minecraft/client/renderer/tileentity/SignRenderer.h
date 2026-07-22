#pragma once
#include "TileEntityRenderer.h"

class SignModel;
class ResourceLocation;

class SignRenderer : public TileEntityRenderer {
private:
    static ResourceLocation SIGN_LOCATION;
    SignModel* signModel;

public:
    SignRenderer();  
    virtual void render(std::shared_ptr<TileEntity> sign, double x, double y,
                        double z, float a, bool setColor, float alpha = 1.0f,
                        bool useCompiled = true);  
};
