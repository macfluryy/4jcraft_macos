#pragma once

#include <string>

#include "app/common/src/UI/Controls/UIControl_PlayerList.h"
#include "app/common/src/UI/UIScene.h"
#include "app/linux/Iggy/include/iggy.h"
#ifndef _ENABLEIGGY
#include "app/linux/Stubs/iggy_stubs.h"
#endif
#include "UIControl_ButtonList.h"

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