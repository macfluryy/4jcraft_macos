#pragma once

#include <string>

#include "platform/sdl2/Render.h"
#include "app/common/src/UI/All Platforms/UIEnums.h"
#include "app/common/src/UI/UIScene.h"
#include "app/mac/Iggy/include/rrCore.h"

class UILayer;

class UIComponent_MenuBackground : public UIScene {
private:
    bool m_bSplitscreen;

public:
    UIComponent_MenuBackground(int iPad, void* initData, UILayer* parentLayer);

protected:
    
    virtual std::wstring getMoviePath();

public:
    virtual EUIScene getSceneType() { return eUIComponent_MenuBackground; }

    
    virtual bool stealsFocus() { return false; }

    
    virtual bool hasFocus(int iPad) { return false; }

    
    
    virtual bool hidesLowerScenes() { return false; }

    
    virtual void render(S32 width, S32 height,
                        C4JRender::eViewportType viewport);
};