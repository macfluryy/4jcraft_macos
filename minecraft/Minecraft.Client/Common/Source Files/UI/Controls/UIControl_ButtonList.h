#pragma once

#include <string>
#include <vector>

#include "UIControl_Base.h"
#include "Minecraft.Client/Common/Source Files/UI/UIString.h"
#include "Minecraft.Client/Linux/Iggy/include/iggy.h"
#include "Minecraft.Client/Common/Source Files/UI/UIScene.h"

class UIControl_ButtonList : public UIControl_Base {
protected:
    IggyName m_addNewItemFunc, m_removeAllItemsFunc, m_funcHighlightItem,
        m_funcRemoveItem, m_funcSetButtonLabel, m_funcSetTouchFocus,
        m_funcCanTouchTrigger;

    int m_itemCount;
    int m_iCurrentSelection;

public:
    UIControl_ButtonList();

    virtual bool setupControl(UIScene* scene, IggyValuePath* parent,
                              const std::string& controlName);

    void init(int id);
    virtual void ReInit();

    void clearList();

    void addItem(const std::wstring& label);
    void addItem(const std::string& label);

    void addItem(const std::wstring& label, int data);
    void addItem(const std::string& label, int data);

    void removeItem(int index);

    int getItemCount() { return m_itemCount; }

    void setCurrentSelection(int iSelection);
    int getCurrentSelection();

    void updateChildFocus(int iChild);

    void setButtonLabel(int iButtonId, const std::wstring& label);
};

class UIControl_DynamicButtonList : public UIControl_ButtonList {
protected:
    std::vector<UIString> m_labels;

public:
    virtual void tick();

    virtual void addItem(UIString label, int data = -1);

    virtual void removeItem(int index);
};