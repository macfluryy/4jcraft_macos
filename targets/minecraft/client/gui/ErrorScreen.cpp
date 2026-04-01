#include "ErrorScreen.h"

#include "minecraft/client/gui/Screen.h"

ErrorScreen::ErrorScreen(const std::wstring& title,
                         const std::wstring& message) {
    this->title = title;
    this->message = message;
}

void ErrorScreen::init() {}

void ErrorScreen::render(int xm, int ym, float a) {
    //        fill(0, 0, width, height, 0x40000000);
    fillGradient(0, 0, width, height, 0xff402020, 0xff501010);

    drawCenteredString(font, title, width / 2, 90, 0xffffff);
    drawCenteredString(font, message, width / 2, 110, 0xffffff);

    Screen::render(xm, ym, a);
}

void ErrorScreen::keyPressed(wchar_t eventCharacter, int eventKey) {}