#pragma once

#include "Minecraft.Client/Common/src/UI/Controls/UIControl_MinecraftPlayer.h"
#include "Minecraft.Client/Linux/Iggy/include/iggy.h"
#include "UIControl.h"

class UIControl_MinecraftPlayer : public UIControl {
private:
    float m_fScreenWidth, m_fScreenHeight;
    float m_fRawWidth, m_fRawHeight;

public:
    UIControl_MinecraftPlayer();

    void render(IggyCustomDrawCallbackRegion* region);
};