#pragma once

#include <string>

#include "app/common/src/UI/Controls/UIControl_DLCList.h"
#include "app/common/src/UI/UIScene.h"
#include "app/linux/Iggy/include/iggy.h"
#ifndef _ENABLEIGGY
#include "app/linux/Stubs/iggy_stubs.h"
#endif
#include "UIControl_ButtonList.h"

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
