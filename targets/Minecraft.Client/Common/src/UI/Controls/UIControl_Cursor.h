#pragma once

#include <string>

#include "Minecraft.Client/Common/src/UI/Controls/UIControl_Base.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl_Cursor.h"
#include "Minecraft.Client/Common/src/UI/UIScene.h"
#include "Minecraft.Client/Linux/Iggy/include/iggy.h"
#include "UIControl_Base.h"

class UIControl_Cursor : public UIControl_Base {
public:
    UIControl_Cursor();

    virtual bool setupControl(UIScene* scene, IggyValuePath* parent,
                              const std::string& controlName);
};