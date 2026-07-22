
#include "Windows64_UIController.h"


#include "minecraft/client/Minecraft.h"
#include "minecraft/client/renderer/Textures.h"

#define _ENABLEIGGY

ConsoleUIController ui;

void ConsoleUIController::init(ID3D11Device* dev, ID3D11DeviceContext* ctx,
                               ID3D11RenderTargetView* pRenderTargetView,
                               ID3D11DepthStencilView* pDepthStencilView, S32 w,
                               S32 h) {
#ifdef _ENABLEIGGY
    m_pRenderTargetView = pRenderTargetView;
    m_pDepthStencilView = pDepthStencilView;

    
    preInit(w, h);

    gdraw_funcs = gdraw_D3D11_CreateContext(dev, ctx, w, h);

    if (!gdraw_funcs) {
        app.DebugPrintf("Failed to initialise GDraw!\n");
#ifndef _CONTENT_PACKAGE
        __debugbreak();
#endif
        app.FatalLoadError();
    }

    


















    gdraw_D3D11_SetResourceLimits(GDRAW_D3D11_RESOURCE_vertexbuffer, 5000,
                                  16 * 1024 * 1024);
    gdraw_D3D11_SetResourceLimits(GDRAW_D3D11_RESOURCE_texture, 5000,
                                  128 * 1024 * 1024);
    gdraw_D3D11_SetResourceLimits(GDRAW_D3D11_RESOURCE_rendertarget, 10,
                                  32 * 1024 * 1024);

    
    IggySetGDraw(gdraw_funcs);

    





    IggyAudioUseDirectSound();

    
    postInit();
#endif
}

void ConsoleUIController::render() {
#ifdef _ENABLEIGGY
    








    gdraw_D3D11_SetTileOrigin(m_pRenderTargetView, m_pDepthStencilView, nullptr,
                              0, 0);

    renderScenes();

    


    gdraw_D3D11_NoMoreGDrawThisFrame();
#endif
}

void ConsoleUIController::beginIggyCustomDraw4J(
    IggyCustomDrawCallbackRegion* region, CustomDrawData* customDrawRegion) {
    
    
    gdraw_D3D11_BeginCustomDraw_4J(region, customDrawRegion->mat);
}

CustomDrawData* ConsoleUIController::setupCustomDraw(
    UIScene* scene, IggyCustomDrawCallbackRegion* region) {
    CustomDrawData* customDrawRegion = new CustomDrawData();
    customDrawRegion->x0 = region->x0;
    customDrawRegion->x1 = region->x1;
    customDrawRegion->y0 = region->y0;
    customDrawRegion->y1 = region->y1;

    
    
    gdraw_D3D11_BeginCustomDraw_4J(region, customDrawRegion->mat);

    setupCustomDrawGameStateAndMatrices(scene, customDrawRegion);

    return customDrawRegion;
}

CustomDrawData* ConsoleUIController::calculateCustomDraw(
    IggyCustomDrawCallbackRegion* region) {
    CustomDrawData* customDrawRegion = new CustomDrawData();
    customDrawRegion->x0 = region->x0;
    customDrawRegion->x1 = region->x1;
    customDrawRegion->y0 = region->y0;
    customDrawRegion->y1 = region->y1;

    gdraw_D3D11_CalculateCustomDraw_4J(region, customDrawRegion->mat);

    return customDrawRegion;
}

void ConsoleUIController::endCustomDraw(IggyCustomDrawCallbackRegion* region) {
    endCustomDrawGameStateAndMatrices();

    gdraw_D3D11_EndCustomDraw(region);
}

void ConsoleUIController::setTileOrigin(S32 xPos, S32 yPos) {
    gdraw_D3D11_SetTileOrigin(m_pRenderTargetView, m_pDepthStencilView, nullptr,
                              xPos, yPos);
}

GDrawTexture* ConsoleUIController::getSubstitutionTexture(int textureId) {
    





    ID3D11ShaderResourceView* tex = RenderManager.TextureGetTexture(textureId);
    ID3D11Resource* resource;
    tex->GetResource(&resource);
    ID3D11Texture2D* tex2d = (ID3D11Texture2D*)resource;
    D3D11_TEXTURE2D_DESC desc;
    tex2d->GetDesc(&desc);
    GDrawTexture* gdrawTex = gdraw_D3D11_WrappedTextureCreate(tex);
    return gdrawTex;
}

void ConsoleUIController::destroySubstitutionTexture(void* destroyCallBackData,
                                                     GDrawTexture* handle) {
    


    gdraw_D3D11_WrappedTextureDestroy(handle);
}

void ConsoleUIController::shutdown() {
#ifdef _ENABLEIGGY
    


    gdraw_D3D11_DestroyContext();
#endif
}