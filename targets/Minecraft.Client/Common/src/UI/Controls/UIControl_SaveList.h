#pragma once

#include <string>

#include "Minecraft.Client/Common/src/UI/Controls/UIControl_SaveList.h"
#include "Minecraft.Client/Common/src/UI/UIScene.h"
#include "Minecraft.Client/Linux/Iggy/include/iggy.h"
#include "UIControl_ButtonList.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl_SaveList.h"

class UIControl_SaveList : public UIControl_ButtonList {
private:
    IggyName m_funcSetTextureName;

public:
    virtual bool setupControl(UIScene* scene, IggyValuePath* parent,
                              const std::string& controlName);

    using UIControl_ButtonList::addItem;

    void addItem(const std::wstring& label);
    void addItem(const std::string& label);

    void addItem(const std::wstring& label, int data);
    void addItem(const std::string& label, int data);

    void addItem(const std::string& label, const std::wstring& iconName);
    void addItem(const std::wstring& label, const std::wstring& iconName);
    void setTextureName(int iId, const std::wstring& iconName);

private:
    void addItem(const std::string& label, const std::wstring& iconName,
                 int data);
    void addItem(const std::wstring& label, const std::wstring& iconName,
                 int data);
};