#pragma once
#include "Screen.h"

class ChatScreen : public Screen {
protected:
    std::wstring message;

private:
    int frame;

public:
    ChatScreen();  // 4J added
    virtual void init() override;
    virtual void removed() override;
    virtual void tick() override;

private:
    static const std::wstring allowedChars;

protected:
    void keyPressed(wchar_t ch, int eventKey) override;

public:
    void render(int xm, int ym, float a) override;

protected:
    void mouseClicked(int x, int y, int buttonNum) override;
};