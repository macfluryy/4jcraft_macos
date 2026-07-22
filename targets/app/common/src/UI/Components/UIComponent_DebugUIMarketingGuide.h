#pragma once

#include <string>

#include "app/common/src/UI/All Platforms/UIEnums.h"
#include "app/common/src/UI/Controls/UIControl_Label.h"
#include "app/common/src/UI/UIScene.h"
#include "app/mac/Iggy/include/iggy.h"
#ifndef _ENABLEIGGY
#include "app/mac/Stubs/iggy_stubs.h"
#endif

class UILayer;

class UIComponent_DebugUIMarketingGuide : public UIScene {
private:
    IggyName m_funcSetPlatform;

    UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
    UI_MAP_NAME(m_funcSetPlatform, L"SetPlatform")
    UI_END_MAP_ELEMENTS_AND_NAMES()

public:
    UIComponent_DebugUIMarketingGuide(int iPad, void* initData,
                                      UILayer* parentLayer);

protected:
    
    virtual std::wstring getMoviePath();

public:
    virtual EUIScene getSceneType() {
        return eUIComponent_DebugUIMarketingGuide;
    }

    
    virtual bool stealsFocus() { return false; }

    
    virtual bool hasFocus(int iPad) { return false; }

    
    
    virtual bool hidesLowerScenes() { return false; }
};