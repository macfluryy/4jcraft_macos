#pragma once

#include <string>

#include "app/common/src/UI/Controls/UIControl_Base.h"
#include "app/common/src/UI/Controls/UIControl_Cursor.h"
#include "app/common/src/UI/UIScene.h"
#include "app/linux/Iggy/include/iggy.h"
#include "UIControl_Base.h"

class UIControl_Cursor : public UIControl_Base {
public:
    UIControl_Cursor();

    virtual bool setupControl(UIScene* scene, IggyValuePath* parent,
                              const std::string& controlName);
};