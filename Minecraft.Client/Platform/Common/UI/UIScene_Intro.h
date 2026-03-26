#pragma once

#include "UIScene.h"

class UIScene_Intro : public UIScene {
private:
    bool m_bIgnoreNavigate;
    bool m_bAnimationEnded;

    IggyName m_funcSetIntroPlatform;
#if 0
    UIControl_Touch m_TouchToSkip;
#endif
    UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
#if 0
    UI_MAP_ELEMENT(m_TouchToSkip, "TouchToSkip")
#endif
    UI_MAP_NAME(m_funcSetIntroPlatform, L"SetIntroPlatform")
    UI_END_MAP_ELEMENTS_AND_NAMES()

public:
    UIScene_Intro(int iPad, void* initData, UILayer* parentLayer);

    virtual EUIScene getSceneType() { return eUIScene_Intro; }

    // Returns true if this scene has focus for the pad passed in
#if 1
    virtual bool hasFocus(int iPad) { return bHasFocus; }
#endif

protected:
    virtual std::wstring getMoviePath();

#if 0
    virtual long long getDefaultGtcButtons() { return 0; }
#endif

public:
    // INPUT
    virtual void handleInput(int iPad, int key, bool repeat, bool pressed,
                             bool released, bool& handled);

    virtual void handleAnimationEnd();
    virtual void handleGainFocus(bool navBack);

#if 0
    virtual void handleTouchInput(unsigned int iPad, S32 x, S32 y, int iId,
                                  bool bPressed, bool bRepeat, bool bReleased);
#endif

#ifndef _ENABLEIGGY
    virtual void tick();
#endif
};
