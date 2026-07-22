#pragma once

#include <string>

#include "app/common/src/UI/Controls/UIControl.h"
#include "app/common/src/UI/UIScene.h"
#include "app/common/src/UI/UIString.h"
#include "app/mac/Iggy/include/iggy.h"
#ifndef _ENABLEIGGY
#include "app/mac/Stubs/iggy_stubs.h"
#endif


class UIControl_Base : public UIControl {
protected:
    IggyName m_initFunc;
    IggyName m_setLabelFunc;
    IggyName m_funcGetLabel;
    IggyName m_funcCheckLabelWidths;

    bool m_bLabelChanged;
    UIString m_label;

public:
    UIControl_Base();

    virtual bool setupControl(UIScene* scene, IggyValuePath* parent,
                              const std::string& controlName);

    virtual void tick();

    virtual void setLabel(UIString label, bool instant = false,
                          bool force = false);
    
    
    

    const wchar_t* getLabel();
    virtual void setAllPossibleLabels(int labelCount, wchar_t labels[][256]);
    int getId() { return m_id; }

    virtual bool hasFocus();
};
