#pragma once

#include <string>

#include "Minecraft.Client/Common/src/UI/Controls/UIControl_DLCList.h"
#include "Minecraft.Client/Common/src/UI/UIScene.h"
#include "Minecraft.Client/Linux/Iggy/include/iggy.h"
#include "UIControl_ButtonList.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl_DLCList.h"

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
