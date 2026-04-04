#pragma once

#include <string>

#include "app/common/src/UI/Controls/UIControl_Base.h"
#include "app/common/src/UI/Controls/UIControl_Label.h"
#include "app/common/src/UI/UIScene.h"
#include "app/common/src/UI/UIString.h"
#include "app/linux/Iggy/include/iggy.h"
#ifndef _ENABLEIGGY
#include "app/linux/Stubs/iggy_stubs.h"
#endif
#include "UIControl_Base.h"

class UIControl_Label : public UIControl_Base {
private:
    bool m_reinitEnabled;

public:
    UIControl_Label();

    virtual bool setupControl(UIScene* scene, IggyValuePath* parent,
                              const std::string& controlName);

    void init(UIString label);
    virtual void ReInit();

    void disableReinitialisation() { m_reinitEnabled = false; }
};