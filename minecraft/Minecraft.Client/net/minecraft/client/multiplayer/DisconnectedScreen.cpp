#include <vector>

#include "DisconnectedScreen.h"
#include "Minecraft.Client/net/minecraft/client/title/TitleScreen.h"
#include "Minecraft.Client/net/minecraft/client/gui/Button.h"
#include "minecraft/locale/Language.h"
#include "Minecraft.Client/net/minecraft/client/Minecraft.h"
#include "Minecraft.Client/net/minecraft/client/gui/Screen.h"

DisconnectedScreen::DisconnectedScreen(const std::wstring& title,
                                       const std::wstring reason,
                                       void* reasonObjects, ...) {
    Language* language = Language::getInstance();

    this->title = language->getElement(title);
    if (reasonObjects != nullptr) {
        this->reason = language->getElement(reason, reasonObjects);
    } else {
        this->reason = language->getElement(reason);
    }
}

void DisconnectedScreen::tick() {}

void DisconnectedScreen::keyPressed(char eventCharacter, int eventKey) {}

void DisconnectedScreen::init() {
    Language* language = Language::getInstance();

    buttons.clear();
    buttons.push_back(new Button(0, width / 2 - 100, height / 4 + 24 * 5 + 12,
                                 language->getElement(L"gui.toMenu")));
}

void DisconnectedScreen::buttonClicked(Button* button) {
    if (button->id == 0) {
        minecraft->setScreen(new TitleScreen());
    }
}

void DisconnectedScreen::render(int xm, int ym, float a) {
    renderBackground();

    drawCenteredString(font, title, width / 2, height / 2 - 50, 0xffffff);
    drawCenteredString(font, reason, width / 2, height / 2 - 10, 0xffffff);

    Screen::render(xm, ym, a);
}
