#pragma once
#include "../Screen.h"

class DisconnectedScreen : public Screen {
private:
    std::wstring title, reason;

public:
    DisconnectedScreen(const std::wstring& title, const std::wstring reason,
                       void* reasonObjects, ...);
    virtual void tick();

protected:
    using Screen::keyPressed;

    virtual void keyPressed(char eventCharacter, int eventKey);

public:
    virtual void init();

protected:
    virtual void buttonClicked(Button* button);

public:
    virtual void render(int xm, int ym, float a);
};
