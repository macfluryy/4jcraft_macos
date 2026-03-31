#pragma once

#include <string>

#include "UIControl.h"
#include "Minecraft.Client/Linux/Iggy/include/iggy.h"
#include "Minecraft.Client/Common/Source Files/UI/UIScene.h"

class UIControl_BitmapIcon : public UIControl {
private:
    IggyName m_funcSetTextureName;

public:
    virtual bool setupControl(UIScene* scene, IggyValuePath* parent,
                              const std::string& controlName);

    void setTextureName(const std::wstring& iconName);
};