#pragma once

#include "app/common/src/UI/Controls/UIControl_MinecraftHorse.h"
#include "app/linux/Iggy/include/iggy.h"
#ifndef _ENABLEIGGY
#include "app/linux/Stubs/iggy_stubs.h"
#endif
#include "UIControl.h"

class UIControl_MinecraftHorse : public UIControl {
private:
    float m_fScreenWidth, m_fScreenHeight;
    float m_fRawWidth, m_fRawHeight;

public:
    UIControl_MinecraftHorse();

    void render(IggyCustomDrawCallbackRegion* region);
};