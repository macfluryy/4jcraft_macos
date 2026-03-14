#pragma once
#include "../Screen.h"

class ChatScreen : public Screen {
protected:
    std::wstring message;

private:
    int frame;

public:
    ChatScreen();  // 4J added
    virtual void init();
    virtual void removed();
    virtual void tick();

private:
    static const std::wstring allowedChars;

protected:
    void keyPressed(wchar_t ch, int eventKey);

public:
    void render(int xm, int ym, float a);

protected:
    void mouseClicked(int x, int y, int buttonNum);
};