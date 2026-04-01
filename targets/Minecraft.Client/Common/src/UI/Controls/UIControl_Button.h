#pragma once

#include <string>

#include "Minecraft.Client/Common/src/UI/Controls/UIControl_Base.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl_Button.h"
#include "Minecraft.Client/Common/src/UI/UIScene.h"
#include "Minecraft.Client/Common/src/UI/UIString.h"
#include "Minecraft.Client/Linux/Iggy/include/iggy.h"
#include "UIControl_Base.h"

class UIControl_Button : public UIControl_Base {
private:
    IggyName m_funcEnableButton;

public:
    UIControl_Button();

    virtual bool setupControl(UIScene* scene, IggyValuePath* parent,
                              const std::string& controlName);

    void init(UIString label, int id);
    // void init(const std::wstring &label, int id) {
    // init(UIString::CONSTANT(label), id); }

    virtual void ReInit();

    void setEnable(bool enable);
};