#include <vector>

#include "MessageScreen.h"
#include "minecraft/client/gui/Screen.h"

MessageScreen::MessageScreen(const std::wstring& message) {
    this->message = message;
}

void MessageScreen::keyPressed(char eventCharacter, int eventKey) {}

void MessageScreen::init() { buttons.clear(); }

void MessageScreen::buttonClicked(Button* button) {}

void MessageScreen::render(int xm, int ym, float a) {
    renderDirtBackground(0);
    drawCenteredString(font, message, width / 2, height / 2 - 50, 0xffffff);

    Screen::render(xm, ym, a);
}