#pragma once

#include "TileEntityRenderer.h"
#include "../Models/ChestModel.h"

class EnderChestRenderer : public TileEntityRenderer {
private:
    ChestModel chestModel;

public:
    void render(std::shared_ptr<TileEntity> _chest, double x, double y,
                double z, float a, bool setColor, float alpha = 1.0f,
                bool useCompiled = true);  // 4J added setColor param
};
