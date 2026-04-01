#pragma once

#include <string>

#include "Minecraft.Client/Common/src/UI/All Platforms/UIEnums.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl_Slider.h"
#include "Minecraft.Client/Common/src/UI/UIScene.h"
#include "Minecraft.Client/Linux/Iggy/include/rrCore.h"

class UILayer;

class UIScene_SettingsAudioMenu : public UIScene {
private:
    enum EControls { eControl_Music, eControl_Sound };

    UIControl_Slider m_sliderMusic, m_sliderSound;  // Sliders
    UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
    UI_MAP_ELEMENT(m_sliderMusic, "Music")
    UI_MAP_ELEMENT(m_sliderSound, "Sound")
    UI_END_MAP_ELEMENTS_AND_NAMES()

public:
    UIScene_SettingsAudioMenu(int iPad, void* initData, UILayer* parentLayer);
    virtual ~UIScene_SettingsAudioMenu();

    virtual EUIScene getSceneType() { return eUIScene_SettingsAudioMenu; }

    virtual void updateTooltips();
    virtual void updateComponents();

protected:
    // TODO: This should be pure virtual in this class
    virtual std::wstring getMoviePath();

public:
    // INPUT
    virtual void handleInput(int iPad, int key, bool repeat, bool pressed,
                             bool released, bool& handled);

    virtual void handleSliderMove(F64 sliderId, F64 currentValue);
};