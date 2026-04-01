#pragma once

#include <string>

#include "UIControl.h"
#include "Minecraft.Client/Common/src/UI/UIString.h"
#include "Minecraft.Client/Linux/Iggy/include/iggy.h"
#include "Minecraft.Client/Common/src/UI/UIScene.h"

// This class maps to the FJ_Base class in actionscript
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
    // virtual void setLabel(std::wstring label, bool instant = false, bool
    // force = false) { this->setLabel(UIString::CONSTANT(label), instant,
    // force); }

    const wchar_t* getLabel();
    virtual void setAllPossibleLabels(int labelCount, wchar_t labels[][256]);
    int getId() { return m_id; }

    virtual bool hasFocus();
};
