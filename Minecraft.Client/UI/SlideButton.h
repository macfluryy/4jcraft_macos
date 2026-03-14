#pragma once
#include "Button.h"
#include "../GameState/Options.h"
class Minecraft;

class SlideButton : public Button {
public:
    float value;
    bool sliding;

private:
    const Options::Option* option;

public:
    SlideButton(int id, int x, int y, const Options::Option* option,
                const std::wstring& msg, float value);

protected:
    virtual int getYImage(bool hovered);
    virtual void renderBg(Minecraft* minecraft, int xm, int ym);

public:
    virtual bool clicked(Minecraft* minecraft, int mx, int my);
    virtual void released(int mx, int my);
};