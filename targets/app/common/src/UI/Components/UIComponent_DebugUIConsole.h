#pragma once

#include <deque>
#include <string>

#include "app/common/src/UI/All Platforms/UIEnums.h"
#include "app/common/src/UI/Controls/UIControl_Label.h"
#include "app/common/src/UI/UIScene.h"

class UILayer;

class UIComponent_DebugUIConsole : public UIScene {
private:
    UIControl_Label m_labels[10];
    UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
    UI_MAP_ELEMENT(m_labels[0], "consoleLine1")
    UI_MAP_ELEMENT(m_labels[1], "consoleLine2")
    UI_MAP_ELEMENT(m_labels[2], "consoleLine3")
    UI_MAP_ELEMENT(m_labels[3], "consoleLine4")
    UI_MAP_ELEMENT(m_labels[4], "consoleLine5")
    UI_MAP_ELEMENT(m_labels[5], "consoleLine6")
    UI_MAP_ELEMENT(m_labels[6], "consoleLine7")
    UI_MAP_ELEMENT(m_labels[7], "consoleLine8")
    UI_MAP_ELEMENT(m_labels[8], "consoleLine9")
    UI_MAP_ELEMENT(m_labels[9], "consoleLine10")
    UI_END_MAP_ELEMENTS_AND_NAMES()

    std::deque<std::string> m_textList;

    bool m_bTextChanged;

public:
    UIComponent_DebugUIConsole(int iPad, void* initData, UILayer* parentLayer);

    virtual void tick();

protected:
    
    virtual std::wstring getMoviePath();

public:
    virtual EUIScene getSceneType() { return eUIComponent_DebugUIConsole; }

    
    virtual bool stealsFocus() { return false; }

    
    virtual bool hasFocus(int iPad) { return false; }

    
    
    virtual bool hidesLowerScenes() { return false; }

    void addText(const std::string& text);
};