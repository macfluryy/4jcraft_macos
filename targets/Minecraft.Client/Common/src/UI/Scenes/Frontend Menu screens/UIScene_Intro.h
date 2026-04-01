#pragma once

#include <string>

#include "Minecraft.Client/Common/src/UI/All Platforms/UIEnums.h"
#include "Minecraft.Client/Common/src/UI/UIScene.h"
#include "Minecraft.Client/Linux/Iggy/include/iggy.h"

class UILayer;

class UIScene_Intro : public UIScene {
private:
    bool m_bIgnoreNavigate;
    bool m_bAnimationEnded;

    IggyName m_funcSetIntroPlatform;
    UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
    UI_MAP_NAME(m_funcSetIntroPlatform, L"SetIntroPlatform")
    UI_END_MAP_ELEMENTS_AND_NAMES()

public:
    UIScene_Intro(int iPad, void* initData, UILayer* parentLayer);

    virtual EUIScene getSceneType() { return eUIScene_Intro; }

    // Returns true if this scene has focus for the pad passed in
    virtual bool hasFocus(int iPad) { return bHasFocus; }

protected:
    virtual std::wstring getMoviePath();

public:
    // INPUT
    virtual void handleInput(int iPad, int key, bool repeat, bool pressed,
                             bool released, bool& handled);

    virtual void handleAnimationEnd();
    virtual void handleGainFocus(bool navBack);

#if !defined(_ENABLEIGGY)
    virtual void tick();
#endif
};
