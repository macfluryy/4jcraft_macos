#pragma once

#include <string>

#include "UIControl_Base.h"
#include "Minecraft.Client/Common/Source Files/UI/UIString.h"
#include "Minecraft.Client/Common/Source Files/UI/UIScene.h"
#include "Minecraft.Client/Linux/Iggy/include/iggy.h"

class UIControl_TextInput : public UIControl_Base {
private:
    IggyName m_textName, m_funcChangeState, m_funcSetCharLimit;
    bool m_bHasFocus;

public:
    UIControl_TextInput();

    virtual bool setupControl(UIScene* scene, IggyValuePath* parent,
                              const std::string& controlName);

    void init(UIString label, int id);
    void ReInit();

    virtual void setFocus(bool focus);

    void SetCharLimit(int iLimit);
};