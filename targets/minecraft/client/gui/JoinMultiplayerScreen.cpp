#include "JoinMultiplayerScreen.h"

#include <stdio.h>

#include <string>
#include <vector>

#include "Button.h"
#include "EditBox.h"
#include "app/include/stubs.h"
#include "app/mac/MacGame.h"
#include "util/StringHelpers.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/Options.h"
#include "minecraft/client/gui/Screen.h"
#include "minecraft/locale/Language.h"

JoinMultiplayerScreen::JoinMultiplayerScreen(Screen* lastScreen) {
    ipEdit = nullptr;
    this->lastScreen = lastScreen;
}

void JoinMultiplayerScreen::tick() { ipEdit->tick(); }

void JoinMultiplayerScreen::init() {
    Language* language = Language::getInstance();

    Keyboard::enableRepeatEvents(true);
    buttons.clear();
    buttons.push_back(new Button(0, width / 2 - 100, height / 4 + 24 * 4 + 12,
                                 language->getElement(L"multiplayer.connect")));
    buttons.push_back(new Button(1, width / 2 - 100, height / 4 + 24 * 5 + 12,
                                 language->getElement(L"gui.cancel")));
    std::wstring ip = replaceAll(minecraft->options->lastMpIp, L"_", L":");
    // 4J macOS - keep Connect always clickable so we can react with a log
    // even when the IP edit is empty (the screen-level keyPressed callback
    // is not always wired through SDL on macOS, so the active flag was never
    // toggling on text input).
    buttons[0]->active = true;
    fprintf(stderr,
            "[TCP] JoinMultiplayerScreen::init - lastMpIp='%ls' (active=1)\n",
            ip.c_str());

    ipEdit = new EditBox(this, font, width / 2 - 100, height / 4 - 10 + 50 + 18,
                         200, 20, ip);
    ipEdit->inFocus = true;
    ipEdit->setMaxLength(128);
}

void JoinMultiplayerScreen::removed() { Keyboard::enableRepeatEvents(false); }

void JoinMultiplayerScreen::buttonClicked(Button* button) {
    fprintf(stderr,
            "[TCP] JoinMultiplayerScreen::buttonClicked id=%d active=%d\n",
            button ? button->id : -1, button ? (int)button->active : -1);
    if (!button->active) return;
    if (button->id == 1) {
        minecraft->setScreen(lastScreen);
    } else if (button->id == 0) {
        std::wstring ip = trimString(ipEdit->getValue());
        fprintf(stderr, "[TCP] Connect pressed, IP='%ls'\n", ip.c_str());

        minecraft->options->lastMpIp = replaceAll(ip, L":", L"_");
        minecraft->options->save();

        // 4J macOS - parse "<host>" or "<host>:<port>" (also tolerates the
        // bracketed IPv6 form "[<host>]:<port>") and hand off to the
        // direct-connect TCP startup path.
        std::wstring whost;
        int port = 25565;
        if (!ip.empty() && ip[0] == L'[') {
            auto pos = ip.find(L']');
            if (pos != std::wstring::npos) {
                whost = ip.substr(1, pos - 1);
                std::wstring rest = trimString(ip.substr(pos + 1));
                if (!rest.empty() && rest[0] == L':') {
                    port = parseInt(rest.substr(1), 25565);
                }
            } else {
                whost = ip;
            }
        } else {
            auto pos = ip.find(L':');
            if (pos != std::wstring::npos) {
                whost = ip.substr(0, pos);
                port = parseInt(ip.substr(pos + 1), 25565);
            } else {
                whost = ip;
            }
        }
        if (port <= 0 || port >= 65536) port = 25565;

        // Convert wide host to ASCII (IPs / hostnames don't carry non-ASCII).
        std::string host;
        host.reserve(whost.size());
        for (wchar_t wc : whost) {
            if (wc < 128) host.push_back((char)wc);
        }
        if (host.empty()) {
            fprintf(stderr,
                    "[TCP] JoinMultiplayerScreen: empty host, ignoring.\n");
            return;
        }

        fprintf(stderr,
                "[TCP] JoinMultiplayerScreen connect -> %s:%d\n", host.c_str(),
                port);
        if (!app.TemporaryDirectConnectStart(host.c_str(), port)) {
            fprintf(stderr,
                    "[TCP] Direct-connect failed; staying on multiplayer "
                    "screen.\n");
        }
    }
}

int JoinMultiplayerScreen::parseInt(const std::wstring& str, int def) {
    return fromWString<int>(str);
}

void JoinMultiplayerScreen::keyPressed(wchar_t ch, int eventKey) {
    ipEdit->keyPressed(ch, eventKey);

    if (ch == 13) {
        buttonClicked(buttons[0]);
    }
    buttons[0]->active = ipEdit->getValue().length() > 0;
}

void JoinMultiplayerScreen::mouseClicked(int x, int y, int buttonNum) {
    Screen::mouseClicked(x, y, buttonNum);

    ipEdit->mouseClicked(x, y, buttonNum);
}

void JoinMultiplayerScreen::render(int xm, int ym, float a) {
    Language* language = Language::getInstance();

    // fill(0, 0, width, height, 0x40000000);
    renderBackground();

    drawCenteredString(font, language->getElement(L"multiplayer.title"),
                       width / 2, height / 4 - 60 + 20, 0xffffff);
    drawString(font, language->getElement(L"multiplayer.info1"),
               width / 2 - 140, height / 4 - 60 + 60 + 9 * 0, 0xa0a0a0);
    drawString(font, language->getElement(L"multiplayer.info2"),
               width / 2 - 140, height / 4 - 60 + 60 + 9 * 1, 0xa0a0a0);
    drawString(font, language->getElement(L"multiplayer.ipinfo"),
               width / 2 - 140, height / 4 - 60 + 60 + 9 * 4, 0xa0a0a0);

    ipEdit->render();

    Screen::render(xm, ym, a);
}