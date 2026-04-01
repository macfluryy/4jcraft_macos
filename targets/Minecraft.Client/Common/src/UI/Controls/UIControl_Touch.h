#pragma once

#include <string>

#include "Minecraft.Client/Common/src/UI/Controls/UIControl_Base.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl_Touch.h"
#include "Minecraft.Client/Common/src/UI/UIScene.h"
#include "Minecraft.Client/Linux/Iggy/include/iggy.h"
#include "UIControl_Base.h"

class UIControl_Touch : public UIControl_Base {
private:
public:
    UIControl_Touch();

    virtual bool setupControl(UIScene* scene, IggyValuePath* parent,
                              const std::string& controlName);

    void init(int id);
    virtual void ReInit();
};