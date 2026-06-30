#pragma once
#include <string>

#include "Screen.h"

class EditBox;
class Button;
class UsernameScreen : public Screen {
private:
    Screen* lastScreen;
    EditBox* nickEdit;

public:
    UsernameScreen(Screen* lastScreen);

    virtual void init() override;
    virtual void removed() override;
    virtual void tick() override;

protected:
    virtual void buttonClicked(Button* button) override;
    virtual void keyPressed(wchar_t ch, int eventKey) override;
    virtual void mouseClicked(int x, int y, int buttonNum) override;

public:
    virtual void render(int xm, int ym, float a) override;
};