#pragma once

#include <string>

#include "app/common/src/UI/All Platforms/UIEnums.h"
#include "app/common/src/UI/UIScene.h"

class UILayer;

class UIComponent_Logo : public UIScene {
public:
    UIComponent_Logo(int iPad, void* initData, UILayer* parentLayer);

protected:
    
    virtual std::wstring getMoviePath();

public:
    virtual EUIScene getSceneType() { return eUIComponent_Logo; }

    
    virtual bool stealsFocus() { return false; }

    
    virtual bool hasFocus(int iPad) { return false; }

    
    
    virtual bool hidesLowerScenes() { return false; }
};