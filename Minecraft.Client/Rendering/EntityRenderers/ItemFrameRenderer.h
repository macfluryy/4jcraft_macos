#pragma once
#include "EntityRenderer.h"

class ItemFrameRenderer : public EntityRenderer {
private:
    Icon* backTexture;

    //@Override
public:
    void registerTerrainTextures(IconRegister* iconRegister);
    virtual void render(std::shared_ptr<Entity> _itemframe, double x, double y,
                        double z, float rot, float a);

private:
    void drawFrame(std::shared_ptr<ItemFrame> itemFrame);
    void drawItem(std::shared_ptr<ItemFrame> entity);
};
