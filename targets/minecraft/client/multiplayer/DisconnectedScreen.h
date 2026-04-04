#pragma once
#include <string>

#include "minecraft/client/gui/Screen.h"

class DisconnectedScreen : public Screen {
private:
    std::wstring title, reason;

public:
    DisconnectedScreen(const std::wstring& title, const std::wstring reason,
                       void* reasonObjects, ...);
    virtual void tick() override;

protected:
    using Screen::keyPressed;

    virtual void keyPressed(char eventCharacter, int eventKey);

public:
    virtual void init() override;

protected:
    virtual void buttonClicked(Button* button) override;

public:
    virtual void render(int xm, int ym, float a) override;
};
