#pragma once

#include <string>

#include "Minecraft.Client/Common/src/UI/Controls/UIControl_BitmapIcon.h"
#include "Minecraft.Client/Common/src/UI/UIScene.h"
#include "Minecraft.Client/Linux/Iggy/include/iggy.h"
#include "UIControl.h"

class UIControl_BitmapIcon : public UIControl {
private:
    IggyName m_funcSetTextureName;

public:
    virtual bool setupControl(UIScene* scene, IggyValuePath* parent,
                              const std::string& controlName);

    void setTextureName(const std::wstring& iconName);
};