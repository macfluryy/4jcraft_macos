#pragma once

#include <string>

#include "Minecraft.Client/Common/src/UI/All Platforms/UIEnums.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl_Button.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl_DynamicLabel.h"
#include "Minecraft.Client/Common/src/UI/UIScene.h"
#include "Minecraft.Client/Linux/Iggy/include/rrCore.h"

class UILayer;

class UIScene_EULA : public UIScene {
private:
    enum EControls {
        eControl_Confirm,
    };

    bool m_bIgnoreInput;

    UIControl_Button m_buttonConfirm;
    UIControl_DynamicLabel m_labelDescription;
    UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
    UI_MAP_ELEMENT(m_buttonConfirm, "AcceptButton")
    UI_MAP_ELEMENT(m_labelDescription, "EULAtext")
    UI_END_MAP_ELEMENTS_AND_NAMES()

public:
    UIScene_EULA(int iPad, void* initData, UILayer* parentLayer);
    ~UIScene_EULA();

    virtual EUIScene getSceneType() { return eUIScene_EULA; }

    // Returns true if this scene has focus for the pad passed in
    virtual bool hasFocus(int iPad) { return bHasFocus; }
    virtual void updateTooltips();

protected:
    virtual std::wstring getMoviePath();

public:
    // INPUT
    virtual void handleInput(int iPad, int key, bool repeat, bool pressed,
                             bool released, bool& handled);

protected:
    void handlePress(F64 controlId, F64 childId);

    virtual long long getDefaultGtcButtons() { return 0; }
};
