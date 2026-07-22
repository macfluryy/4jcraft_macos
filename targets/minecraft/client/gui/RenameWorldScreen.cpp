#include "RenameWorldScreen.h"

#include <vector>

#include "Button.h"
#include "EditBox.h"
#include "app/include/stubs.h"
#include "util/StringHelpers.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/gui/Screen.h"
#include "minecraft/locale/Language.h"
#include "minecraft/world/level/storage/LevelStorageSource.h"

RenameWorldScreen::RenameWorldScreen(Screen* lastScreen,
                                     const std::wstring& levelId) {
    nameEdit = nullptr;
    this->lastScreen = lastScreen;
    this->levelId = levelId;
}

void RenameWorldScreen::tick() {
    if (nameEdit != nullptr) nameEdit->tick();
}

void RenameWorldScreen::init() {
    Language* language = Language::getInstance();

    Keyboard::enableRepeatEvents(true);
    buttons.clear();

    buttons.push_back(new Button(
        0, width / 2 - 100, height / 4 + 96 + 12, 98, 20,
        language->getElement(L"selectWorld.renameButton")));
    buttons.push_back(new Button(1, width / 2 + 2, height / 4 + 96 + 12, 98, 20,
                                 language->getElement(L"gui.cancel")));

    
    nameEdit = new EditBox(this, font, width / 2 - 100, 60, 200, 20, levelId);
    nameEdit->setMaxLength(32);
    nameEdit->focus(true);

    buttons[0]->active = trimString(nameEdit->getValue()).length() > 0;
}

void RenameWorldScreen::removed() {
    Keyboard::enableRepeatEvents(false);
    if (nameEdit != nullptr) {
        delete nameEdit;
        nameEdit = nullptr;
    }
}

void RenameWorldScreen::buttonClicked(Button* button) {
    if (!button->active) return;
    if (button->id == 1) {
        minecraft->setScreen(lastScreen);
    } else if (button->id == 0) {
        LevelStorageSource* levelSource = minecraft->getLevelSource();
        levelSource->renameLevel(levelId, trimString(nameEdit->getValue()));

        minecraft->setScreen(lastScreen);
    }
}

void RenameWorldScreen::keyPressed(wchar_t ch, int eventKey) {
    if (nameEdit == nullptr) return;
    nameEdit->keyPressed(ch, eventKey);
    if (!buttons.empty()) {
        buttons[0]->active = trimString(nameEdit->getValue()).length() > 0;
    }

    if (ch == 13 && !buttons.empty()) {
        buttonClicked(buttons[0]);
    }
}

void RenameWorldScreen::mouseClicked(int x, int y, int buttonNum) {
    Screen::mouseClicked(x, y, buttonNum);
    if (nameEdit != nullptr) nameEdit->mouseClicked(x, y, buttonNum);
}

void RenameWorldScreen::render(int xm, int ym, float a) {
    Language* language = Language::getInstance();

    
    renderBackground();

    drawCenteredString(font, language->getElement(L"selectWorld.renameTitle"),
                       width / 2, height / 4 - 60 + 20, 0xffffff);
    drawString(font, language->getElement(L"selectWorld.enterName"),
               width / 2 - 100, 47, 0xa0a0a0);

    if (nameEdit != nullptr) nameEdit->render();

    Screen::render(xm, ym, a);
}