#pragma once
#include "../Screen.h"

class PauseScreen : public Screen {
private:
    int saveStep;
    int visibleTime;

public:
    PauseScreen();  // 4J added
    virtual void init();
    virtual void keyPressed(wchar_t eventCharacter, int eventKey);
    static void exitWorld(Minecraft* minecraft, bool save);

protected:
    using Screen::buttonClicked;

    virtual void buttonClicked(Button* button);

public:
    virtual void tick();
    virtual void render(int xm, int ym, float a);
};
