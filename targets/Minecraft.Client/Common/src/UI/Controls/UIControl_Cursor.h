#pragma once

#include <string>

#include "UIControl_Base.h"
#include "Minecraft.Client/Linux/Iggy/include/iggy.h"
#include "Minecraft.Client/Common/src/UI/UIScene.h"

class UIControl_Cursor : public UIControl_Base {
public:
    UIControl_Cursor();

    virtual bool setupControl(UIScene* scene, IggyValuePath* parent,
                              const std::string& controlName);
};