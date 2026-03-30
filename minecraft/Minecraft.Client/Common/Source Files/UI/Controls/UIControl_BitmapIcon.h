#pragma once

#include "UIControl.h"

class UIControl_BitmapIcon : public UIControl {
private:
    IggyName m_funcSetTextureName;

public:
    virtual bool setupControl(UIScene* scene, IggyValuePath* parent,
                              const std::string& controlName);

    void setTextureName(const std::wstring& iconName);
};