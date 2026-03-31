#pragma once

#include <string>

#include "UIControl_Base.h"
#include "Minecraft.Client/Common/Source Files/UI/UIString.h"
#include "Minecraft.Client/Linux/Iggy/include/iggy.h"
#include "Minecraft.Client/Common/Source Files/UI/UIScene.h"

class UIControl_Progress : public UIControl_Base {
private:
    IggyName m_setProgressFunc, m_showBarFunc;
    int m_min;
    int m_max;
    int m_current;
    float m_lastPercent;
    bool m_showingBar;

public:
    UIControl_Progress();

    virtual bool setupControl(UIScene* scene, IggyValuePath* parent,
                              const std::string& controlName);

    void init(UIString label, int id, int min, int max, int current);
    virtual void ReInit();

    void setProgress(int current);
    void showBar(bool show);
};