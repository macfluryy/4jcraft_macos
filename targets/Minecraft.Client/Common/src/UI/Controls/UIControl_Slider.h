#pragma once

#include <string>
#include <vector>

#include "Minecraft.Client/Common/src/UI/Controls/UIControl_Slider.h"
#include "Minecraft.Client/Common/src/UI/UIScene.h"
#include "Minecraft.Client/Common/src/UI/UIString.h"
#include "Minecraft.Client/Linux/Iggy/include/iggy.h"
#include "Minecraft.Client/Linux/Iggy/include/rrCore.h"
#include "UIControl_Base.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl_Base.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl_Slider.h"

class UIControl_Slider : public UIControl_Base {
private:
    // int m_id;  // 4J-TomK this is part of class UIControl and doesn't need to
    // be here!
    int m_min;
    int m_max;
    int m_current;

    std::vector<std::wstring> m_allPossibleLabels;

    // 4J-TomK - function for setting slider position on touch
    IggyName m_funcSetRelativeSliderPos;
    IggyName m_funcGetRealWidth;

public:
    UIControl_Slider();

    virtual bool setupControl(UIScene* scene, IggyValuePath* parent,
                              const std::string& controlName);

    void init(UIString label, int id, int min, int max, int current);

    void handleSliderMove(int newValue);
    void SetSliderTouchPos(float fTouchPos);
    virtual void setAllPossibleLabels(int labelCount, wchar_t labels[][256]);

    S32 GetRealWidth();
    virtual void ReInit();
};
