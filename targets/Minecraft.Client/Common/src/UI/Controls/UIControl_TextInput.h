#pragma once

#include <string>

#include "Minecraft.Client/Common/src/UI/Controls/UIControl_TextInput.h"
#include "Minecraft.Client/Common/src/UI/UIScene.h"
#include "Minecraft.Client/Common/src/UI/UIString.h"
#include "Minecraft.Client/Linux/Iggy/include/iggy.h"
#include "UIControl_Base.h"

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