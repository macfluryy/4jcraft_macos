#include "UsernameScreen.h"

#include <ctime>
#include <string>

#include "Button.h"
#include "EditBox.h"
#include "app/include/stubs.h"
#include "app/mac/MacGame.h"
#include "platform/sdl2/Profile.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/Options.h"
#include "minecraft/client/User.h"
#include "minecraft/locale/Language.h"
#include "util/StringHelpers.h"

namespace {
constexpr int BTN_SAVE = 0;
constexpr int BTN_CANCEL = 1;
constexpr int kMaxNicknameLen = 16;
}

UsernameScreen::UsernameScreen(Screen* lastScreen)
    : lastScreen(lastScreen), nickEdit(nullptr) {}

void UsernameScreen::init() {
    Keyboard::enableRepeatEvents(true);
    buttons.clear();

    Language* language = Language::getInstance();
    int fieldX = width / 2 - 100;
    int fieldY = height / 2 - 10;

    std::wstring current = minecraft->options->lastMpNickname;
    if (current.empty()) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        unsigned int suffix =
            (unsigned int)((ts.tv_nsec ^ (ts.tv_sec << 4)) % 10000);
        wchar_t buf[32];
        swprintf(buf, sizeof(buf) / sizeof(buf[0]), L"Player%u", suffix);
        current = buf;
    }
    nickEdit = new EditBox(this, font, fieldX, fieldY, 200, 20, current);
    nickEdit->setMaxLength(kMaxNicknameLen);
    nickEdit->inFocus = true;

    buttons.push_back(
        new Button(BTN_SAVE, width / 2 - 100, height / 2 + 28, 98, 20,
                   language->getElement(L"gui.done")));
    buttons.push_back(
        new Button(BTN_CANCEL, width / 2 + 2, height / 2 + 28, 98, 20,
                   language->getElement(L"gui.cancel")));
}

void UsernameScreen::removed() {
    Keyboard::enableRepeatEvents(false);
    delete nickEdit;
    nickEdit = nullptr;
}

void UsernameScreen::tick() {
    if (nickEdit != nullptr) nickEdit->tick();
}

void UsernameScreen::buttonClicked(Button* button) {
    if (button == nullptr || !button->active) return;
    if (button->id == BTN_CANCEL) {
        minecraft->setScreen(lastScreen);
        return;
    }
    if (button->id == BTN_SAVE) {
        std::wstring nick =
            nickEdit != nullptr ? trimString(nickEdit->getValue()) : L"";
        if (nick.empty()) {
            return;
        }
        if (nick != minecraft->options->lastMpNickname) {
            minecraft->options->lastMpNickname = nick;
            minecraft->options->save();
        }
        for (int p = 0; p < XUSER_MAX_COUNT; ++p) {
            SetUserGamertag(p, nick);
        }
        if (minecraft->user != nullptr) {
            minecraft->user->name = nick;
        }
        minecraft->setScreen(lastScreen);
    }
}

void UsernameScreen::keyPressed(wchar_t ch, int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->setScreen(lastScreen);
        return;
    }
    if (nickEdit != nullptr) nickEdit->keyPressed(ch, eventKey);
    if (ch == 13) {
        buttonClicked(buttons.empty() ? nullptr : buttons[0]);
    }
}

void UsernameScreen::mouseClicked(int x, int y, int buttonNum) {
    Screen::mouseClicked(x, y, buttonNum);
    if (minecraft->screen != this) return;
    if (nickEdit != nullptr) nickEdit->mouseClicked(x, y, buttonNum);
}

void UsernameScreen::render(int xm, int ym, float a) {
    renderBackground();

    drawCenteredString(font, L"Username", width / 2, 40, 0xffffff);
    drawCenteredString(font,
                       L"This is the name that other players will see in",
                       width / 2, height / 2 - 50, 0xa0a0a0);
    drawCenteredString(font, L"chat, the tab list and on death messages.",
                       width / 2, height / 2 - 38, 0xa0a0a0);

    drawString(font, L"Nickname:", width / 2 - 100, height / 2 - 22,
               0xa0a0a0);
    if (nickEdit != nullptr) nickEdit->render();

    Screen::render(xm, ym, a);
}