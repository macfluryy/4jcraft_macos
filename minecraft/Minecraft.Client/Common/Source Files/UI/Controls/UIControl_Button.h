#pragma once

#include <string>

#include "UIControl_Base.h"
#include "Minecraft.Client/Common/Source Files/UI/UIString.h"
#include "Minecraft.Client/Linux/Iggy/include/iggy.h"
#include "Minecraft.Client/Common/Source Files/UI/UIScene.h"

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