#pragma once

#include <string>

#include "Minecraft.Client/Common/src/UI/Controls/UIControl_Base.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl_Label.h"
#include "Minecraft.Client/Common/src/UI/UIScene.h"
#include "Minecraft.Client/Common/src/UI/UIString.h"
#include "Minecraft.Client/Linux/Iggy/include/iggy.h"
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