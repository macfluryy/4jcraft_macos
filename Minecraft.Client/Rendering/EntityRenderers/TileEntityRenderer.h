#pragma once
#include "../../Textures/Textures.h"
#include "../../Textures/ResourceLocation.h"

class TileEntityRenderDispatcher;
class TileEntity;
class Level;
class Font;

class TileEntityRenderer {
protected:
    TileEntityRenderDispatcher* tileEntityRenderDispatcher;

public:
    virtual void render(std::shared_ptr<TileEntity> entity, double x, double y,
                        double z, float a, bool setColor, float alpha,
                        bool useCompiled) = 0;  // 4J added setColor param,
                                                // alpha and useCompiled
    virtual void onNewLevel(Level* level) {}

protected:
    void bindTexture(
        ResourceLocation* location);  // 4J - changed from std::wstring to int
    void bindTexture(
        const std::wstring& urlTexture,
        ResourceLocation* location);  // 4J - changed from std::wstring to int
private:
    Level* getLevel();

public:
    virtual void init(TileEntityRenderDispatcher* tileEntityRenderDispatcher);
    Font* getFont();
};
