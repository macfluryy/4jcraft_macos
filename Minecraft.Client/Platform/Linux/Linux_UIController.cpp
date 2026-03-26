// should we keep Linux_UIController.cpp?*
#include "../../../Minecraft.World/Platform/stdafx.h"
#include "Linux_UIController.h"

// Temp
#include "../../Minecraft.h"
#include "../../Textures/Textures.h"

// GDraw GL backend for Linux
#include "Iggy/gdraw/gdraw.h"
#include "4J_Render.h"

ConsoleUIController ui;

static void restoreFixedFunctionStateAfterIggy() {
    RenderManager.StateSetColour(1.0f, 1.0f, 1.0f, 1.0f);
    RenderManager.StateSetAlphaTestEnable(true);
    RenderManager.StateSetAlphaFunc(GL_GREATER, 0.1f);

    RenderManager.StateSetDepthTestEnable(true);
    RenderManager.StateSetDepthFunc(GL_LEQUAL);
    RenderManager.StateSetDepthMask(true);

    RenderManager.StateSetFaceCull(true);
    RenderManager.StateSetActiveTexture(GL_TEXTURE1);
    RenderManager.StateSetTextureEnable(false);
    RenderManager.MatrixMode(GL_TEXTURE);
    RenderManager.MatrixSetIdentity();

    RenderManager.StateSetActiveTexture(GL_TEXTURE0);
    RenderManager.StateSetTextureEnable(true);
    RenderManager.MatrixMode(GL_TEXTURE);

    RenderManager.MatrixSetIdentity();
    RenderManager.MatrixMode(GL_MODELVIEW);

    RenderManager.Set_matrixDirty();
}

void ConsoleUIController::init(S32 w, S32 h) {
#ifdef _ENABLEIGGY
    // Shared init
    preInit(w, h);

    // init
    gdraw_funcs = gdraw_GL_CreateContext(w, h, 0);

    if (!gdraw_funcs) {
        app.DebugPrintf("Failed to initialise GDraw GL!\n");
        app.FatalLoadError();
    }

    gdraw_GL_SetResourceLimits(GDRAW_GL_RESOURCE_vertexbuffer, 5000,
                               16 * 1024 * 1024);
    gdraw_GL_SetResourceLimits(GDRAW_GL_RESOURCE_texture, 5000,
                               128 * 1024 * 1024);
    gdraw_GL_SetResourceLimits(GDRAW_GL_RESOURCE_rendertarget, 10,
                               32 * 1024 * 1024);

    IggySetGDraw(gdraw_funcs);
#endif
    postInit();
}

void ConsoleUIController::render() {
#ifdef _ENABLEIGGY
    if (!gdraw_funcs) return;

    gdraw_GL_SetTileOrigin(0, 0, 0);
    if (!app.GetGameStarted()) {
        glDisable(GL_SCISSOR_TEST);
        glClearDepth(1.0);
        glDepthMask(GL_TRUE);
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    // render
    renderScenes();

    gdraw_GL_NoMoreGDrawThisFrame();
    restoreFixedFunctionStateAfterIggy();
#endif
}

void ConsoleUIController::beginIggyCustomDraw4J(
    IggyCustomDrawCallbackRegion* region, CustomDrawData* customDrawRegion) {
    gdraw_GL_BeginCustomDraw_4J(region, customDrawRegion->mat);
}

CustomDrawData* ConsoleUIController::setupCustomDraw(
    UIScene* scene, IggyCustomDrawCallbackRegion* region) {
    CustomDrawData* customDrawRegion = new CustomDrawData();
    customDrawRegion->x0 = region->x0;
    customDrawRegion->x1 = region->x1;
    customDrawRegion->y0 = region->y0;
    customDrawRegion->y1 = region->y1;

    gdraw_GL_BeginCustomDraw_4J(region, customDrawRegion->mat);

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

    gdraw_GL_CalculateCustomDraw_4J(region, customDrawRegion->mat);

    return customDrawRegion;
}

void ConsoleUIController::endCustomDraw(IggyCustomDrawCallbackRegion* region) {
    endCustomDrawGameStateAndMatrices();

    gdraw_GL_EndCustomDraw(region);
}

void ConsoleUIController::setTileOrigin(S32 xPos, S32 yPos) {
    gdraw_GL_SetTileOrigin(xPos, yPos, 0);
}

GDrawTexture* ConsoleUIController::getSubstitutionTexture(int textureId) {
    // todo impl
    return nullptr;
}

void ConsoleUIController::destroySubstitutionTexture(void* destroyCallBackData,
                                                     GDrawTexture* handle) {
    if (handle) gdraw_GL_WrappedTextureDestroy(handle);
}

void ConsoleUIController::shutdown() {
#ifdef _ENABLEIGGY
    if (gdraw_funcs) {
        gdraw_GL_DestroyContext();
        gdraw_funcs = nullptr;
    }
#endif
}