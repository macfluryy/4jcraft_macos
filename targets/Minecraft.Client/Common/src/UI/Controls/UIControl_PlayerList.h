#pragma once

#include <string>

#include "Minecraft.Client/Common/src/UI/Controls/UIControl_PlayerList.h"
#include "Minecraft.Client/Common/src/UI/UIScene.h"
#include "Minecraft.Client/Linux/Iggy/include/iggy.h"
#include "UIControl_ButtonList.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl_PlayerList.h"

class UIControl_PlayerList : public UIControl_ButtonList {
private:
    IggyName m_funcSetPlayerIcon, m_funcSetVOIPIcon;

public:
    virtual bool setupControl(UIScene* scene, IggyValuePath* parent,
                              const std::string& controlName);

    using UIControl_ButtonList::addItem;
    void addItem(const std::wstring& label, int iPlayerIcon, int iVOIPIcon);
    void setPlayerIcon(int iId, int iPlayerIcon);
    void setVOIPIcon(int iId, int iVOIPIcon);
};