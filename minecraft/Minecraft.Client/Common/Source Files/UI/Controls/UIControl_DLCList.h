#pragma once

#include <string>

#include "UIControl_ButtonList.h"
#include "Minecraft.Client/Linux/Iggy/include/iggy.h"
#include "Minecraft.Client/Common/Source Files/UI/UIScene.h"

class UIControl_DLCList : public UIControl_ButtonList {
private:
    IggyName m_funcShowTick;

public:
    virtual bool setupControl(UIScene* scene, IggyValuePath* parent,
                              const std::string& controlName);

    using UIControl_ButtonList::addItem;
    void addItem(const std::string& label, bool showTick, int iId);
    void addItem(const std::wstring& label, bool showTick, int iId);
    void showTick(int iId, bool showTick);
};
