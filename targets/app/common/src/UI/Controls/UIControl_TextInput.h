#pragma once

#include <string>

#include "app/common/src/UI/Controls/UIControl_Base.h"
#include "app/common/src/UI/Controls/UIControl_TextInput.h"
#include "app/common/src/UI/UIScene.h"
#include "app/common/src/UI/UIString.h"
#include "app/linux/Iggy/include/iggy.h"
#ifndef _ENABLEIGGY
#include "app/linux/Stubs/iggy_stubs.h"
#endif
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