#pragma once
#include "../Screen.h"

class MessageScreen : public Screen {
private:
    std::wstring message;

public:
    MessageScreen(const std::wstring& message);

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