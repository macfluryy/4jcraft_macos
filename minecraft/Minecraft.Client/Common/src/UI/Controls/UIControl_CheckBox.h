#pragma once

#include <string>

#include "UIControl_Base.h"
#include "Minecraft.Client/Common/src/UI/UIString.h"
#include "Minecraft.Client/Linux/Iggy/include/iggy.h"
#include "Minecraft.Client/Common/src/UI/UIScene.h"

class UIControl_CheckBox : public UIControl_Base {
private:
    IggyName m_checkedProp, m_funcEnable, m_funcSetCheckBox;

    bool m_bChecked, m_bEnabled;

public:
    UIControl_CheckBox();

    virtual bool setupControl(UIScene* scene, IggyValuePath* parent,
                              const std::string& controlName);

    void init(UIString label, int id, bool checked);

    bool IsChecked();
    bool IsEnabled();
    void SetEnable(bool enable);
    void setChecked(bool checked);
    void TouchSetCheckbox(bool checked);

    virtual void ReInit();
};
