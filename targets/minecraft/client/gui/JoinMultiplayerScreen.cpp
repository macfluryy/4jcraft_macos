#include "JoinMultiplayerScreen.h"

#include <stdio.h>

#include <algorithm>
#include <chrono>
#include <string>
#include <vector>

#include "Button.h"
#include "EditBox.h"
#include "JavaServerListScreen.h"
#include "app/common/App_enums.h"
#include "app/common/src/Network/GameNetworkManager.h"
#include "app/common/src/Network/LanDiscovery.h"
#include "app/common/src/UI/All Platforms/UIStructs.h"
#include "app/common/src/UI/UIController.h"
#include "app/include/stubs.h"
#include "app/mac/MacGame.h"
#include "app/mac/Mac_UIController.h"
#include "platform/sdl2/Profile.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/Options.h"
#include "minecraft/client/gui/Screen.h"
#include "minecraft/locale/Language.h"
#include "util/StringHelpers.h"

JoinMultiplayerScreen::JoinMultiplayerScreen(Screen* lastScreen) {
    ipEdit = nullptr;
    this->lastScreen = lastScreen;
}

void JoinMultiplayerScreen::tick() {
    if (ipEdit != nullptr) ipEdit->tick();
    // 4J macOS - refresh the LAN-discovery snapshot at most once per ~250 ms
    // (this tick fires at 20 Hz, so every 5 ticks).
    static int s_throttle = 0;
    if (++s_throttle >= 5) {
        s_throttle = 0;
        refreshLanRows();
    }
}

void JoinMultiplayerScreen::refreshLanRows() {
    auto found = LanDiscovery::GetActiveServers();
    static size_t s_lastLogged = (size_t)-1;
    if (found.size() != s_lastLogged) {
        s_lastLogged = found.size();
        fprintf(stderr, "[LAN] JoinMenu sees %zu server(s)\n", found.size());
    }
    lanRows.clear();
    lanRows.reserve(found.size());
    for (const auto& s : found) {
        LanRow row;
        std::wstring nameW =
            s.worldName.empty() ? std::wstring(L"World") : s.worldName;
        std::wstring hostW;
        for (char c : s.host) hostW.push_back((wchar_t)c);
        // 4J macOS - plain ASCII label, no colour codes / unicode dot.
        // Earlier the label embedded section-sign colour runs and a
        // U+25CF bullet; those were not laid out by the bitmap font and
        // the row rendered invisibly. Keep it simple - colour is applied
        // by drawString's int colour argument instead.
        wchar_t buf[200] = {0};
        if (!s.motd.empty()) {
            swprintf(buf, 200, L"%ls - %ls  (%ls:%u) %u/%u", nameW.c_str(),
                     s.motd.c_str(), hostW.c_str(), (unsigned)s.tcpPort,
                     (unsigned)s.playerCount, (unsigned)s.maxPlayers);
        } else {
            swprintf(buf, 200, L"%ls  (%ls:%u)  %u/%u", nameW.c_str(),
                     hostW.c_str(), (unsigned)s.tcpPort,
                     (unsigned)s.playerCount, (unsigned)s.maxPlayers);
        }
        row.label = buf;
        row.host = s.host;
        row.port = s.tcpPort;
        lanRows.push_back(std::move(row));
    }
}

void JoinMultiplayerScreen::init() {
    Language* language = Language::getInstance();

    Keyboard::enableRepeatEvents(true);
    buttons.clear();
    // Single IP field, label above it. Connect / Cancel buttons stack
    // below.
    buttons.push_back(new Button(0, width / 2 - 100, height / 4 + 96 + 12,
                                 language->getElement(L"multiplayer.connect")));
    buttons.push_back(new Button(1, width / 2 - 100, height / 4 + 120 + 12,
                                 language->getElement(L"gui.cancel")));
    // 4J macOS - Java Edition Servers entry point (Req 1.1/1.2). A single
    // extra button below Cancel that only opens the standalone
    // JavaServerListScreen; it does not touch the IP field, Connect/Cancel,
    // tryConnect or the LAN list, so the observable LCE contract is unchanged
    // (Req 1.5 / 20.2 / 20.3).
    buttons.push_back(new Button(2, width / 2 - 100, height / 4 + 144 + 12,
                                 L"Java Edition Servers"));
    std::wstring ip = replaceAll(minecraft->options->lastMpIp, L"_", L":");
    buttons[0]->active = true;
    fprintf(stderr,
            "[TCP] JoinMultiplayerScreen::init - lastMpIp='%ls' "
            "lastMpNickname='%ls'\n",
            ip.c_str(), minecraft->options->lastMpNickname.c_str());

    int fieldX = width / 2 - 100;
    int ipY = height / 4 + 60;
    ipEdit = new EditBox(this, font, fieldX, ipY, 200, 20, ip);
    ipEdit->inFocus = true;
    ipEdit->setMaxLength(128);

    refreshLanRows();
}

void JoinMultiplayerScreen::removed() {
    Keyboard::enableRepeatEvents(false);
}

bool JoinMultiplayerScreen::tryConnect(const std::string& host, int port) {
    fprintf(stderr,
            "[TCP] JoinMultiplayerScreen connect -> %s:%d\n", host.c_str(),
            port);

    // Use whatever nickname is already saved in Options. The dedicated
    // Username screen on the title menu is the only place to change it.
    std::wstring nick = minecraft->options->lastMpNickname;
    if (nick.empty()) nick = L"Player";

    // 4J macOS - Two things have to happen at once:
    //   * The TCP connect needs to be issued before we hand control to
    //     the FullscreenProgress loader, because RunNetworkGameThreadProc
    //     looks at GetSocket() on the local player and we need that
    //     populated when it runs.
    //   * The Java JoinMultiplayerScreen has to come down, and the Iggy
    //     menu stack has to be replaced with the FullscreenProgress
    //     scene. Otherwise both UIs render on top of each other while
    //     gameplay is also drawing, producing the black-chunk artefacts
    //     and the stuck "Connect/Cancel" buttons.
    if (!app.TemporaryDirectConnectStartEx(host.c_str(), port,
                                           /*spawnOwnThread*/ false,
                                           nick.c_str())) {
        fprintf(stderr, "[TCP] Direct-connect setup failed.\n");
        return false;
    }

    minecraft->setScreen(nullptr);

    LoadingInputParams* loadingParams = new LoadingInputParams();
    loadingParams->func = &CGameNetworkManager::RunNetworkGameThreadProc;
    loadingParams->lpParam = app.m_pendingDirectConnectParam;
    app.m_pendingDirectConnectParam = nullptr;

    UIFullscreenProgressCompletionData* completionData =
        new UIFullscreenProgressCompletionData();
    completionData->bShowBackground = true;
    completionData->bShowLogo = true;
    completionData->type = e_ProgressCompletion_CloseAllPlayersUIScenes;
    completionData->iPad = ProfileManager.GetPrimaryPad();
    loadingParams->completionData = completionData;

    ui.NavigateToScene(ProfileManager.GetPrimaryPad(),
                       eUIScene_FullscreenProgress, loadingParams);
    return true;
}

void JoinMultiplayerScreen::buttonClicked(Button* button) {
    fprintf(stderr,
            "[TCP] JoinMultiplayerScreen::buttonClicked id=%d active=%d\n",
            button ? button->id : -1, button ? (int)button->active : -1);
    if (!button->active) return;
    if (button->id == 1) {
        minecraft->setScreen(lastScreen);
    } else if (button->id == 2) {
        // 4J macOS - open the standalone Java Edition Servers browser
        // (Req 1.2). Mirrors the Cancel/Connect pattern of constructing a
        // screen and handing it to setScreen; `this` is passed so the new
        // screen returns here on Back (Req 1.3).
        minecraft->setScreen(new JavaServerListScreen(this));
    } else if (button->id == 0) {
        std::wstring ip = trimString(ipEdit->getValue());
        fprintf(stderr, "[TCP] Connect pressed, IP='%ls'\n", ip.c_str());

        minecraft->options->lastMpIp = replaceAll(ip, L":", L"_");
        minecraft->options->save();

        // Parse "<host>", "<host>:<port>", "[<host>]:<port>".
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

        tryConnect(host, port);
    }
}

int JoinMultiplayerScreen::parseInt(const std::wstring& str, int def) {
    return fromWString<int>(str);
}

void JoinMultiplayerScreen::keyPressed(wchar_t ch, int eventKey) {
    if (ipEdit != nullptr) ipEdit->keyPressed(ch, eventKey);

    if (ch == 13) {
        buttonClicked(buttons[0]);
        // setScreen() may have destructed `this`.
        if (minecraft->screen != this) return;
    }
    buttons[0]->active = ipEdit->getValue().length() > 0;
}

void JoinMultiplayerScreen::mouseClicked(int xClick, int yClick,
                                          int buttonNum) {
    Screen::mouseClicked(xClick, yClick, buttonNum);
    // 4J macOS - Screen::mouseClicked may have triggered setScreen() in
    // buttonClicked (Cancel) - guard against UAF.
    if (minecraft->screen != this) return;
    if (ipEdit != nullptr) ipEdit->mouseClicked(xClick, yClick, buttonNum);

    // 4J macOS - hit-test against the LAN list. Must match the geometry
    // computed in render() exactly.
    if (buttonNum != 0 || lanRows.empty()) return;
    int rowH = 11;
    int maxRows = 6;
    int listW = 280;
    int listX = (width - listW) / 2;
    int visible = std::min((int)lanRows.size(), maxRows);
    int blockH = (visible + 1) * rowH + 6;
    int listY = height - blockH - 4;
    if (listY < height / 4 + 60) listY = height / 4 + 60;
    for (int i = 0; i < visible; ++i) {
        int rowY = listY + 2 + (i + 1) * rowH;
        if (xClick >= listX && xClick <= listX + listW && yClick >= rowY &&
            yClick <= rowY + rowH) {
            std::wstring full;
            for (char c : lanRows[i].host) full.push_back((wchar_t)c);
            wchar_t portBuf[16] = {0};
            swprintf(portBuf, 16, L":%u", (unsigned)lanRows[i].port);
            full += portBuf;
            ipEdit->setValue(full);
            tryConnect(lanRows[i].host, (int)lanRows[i].port);
            return;
        }
    }
}

void JoinMultiplayerScreen::render(int xm, int ym, float a) {
    Language* language = Language::getInstance();

    renderBackground();

    drawCenteredString(font, language->getElement(L"multiplayer.title"),
                       width / 2, height / 4 - 60 + 20, 0xffffff);
    drawString(font, language->getElement(L"multiplayer.info1"),
               width / 2 - 140, height / 4 - 18, 0xa0a0a0);
    drawString(font, language->getElement(L"multiplayer.info2"),
               width / 2 - 140, height / 4 - 18 + 9, 0xa0a0a0);

    // Show the player which nickname will be used. Read-only here -
    // edit it from the Title -> Username screen.
    std::wstring nick = minecraft->options->lastMpNickname;
    if (nick.empty()) nick = L"(unset)";
    std::wstring playingAs = L"Playing as: " + nick;
    drawString(font, playingAs, width / 2 - 100, height / 4 + 36, 0xa0a0a0);

    drawString(font, language->getElement(L"multiplayer.ipinfo"),
               ipEdit->x, ipEdit->y - 11, 0xa0a0a0);
    ipEdit->render();

    Screen::render(xm, ym, a);

    // 4J macOS - LAN list. Drawn LAST (after the buttons, so nothing
    // overpaints it) and anchored to the BOTTOM of the screen with a
    // fixed pixel offset rather than height/4 + big-constant maths -
    // the old layout pushed it past the bottom edge on the common GUI
    // resolutions, which is why "JoinMenu sees N servers" was logged
    // but nothing showed.
    int rowH = 11;
    int maxRows = 6;
    int listW = 280;
    int listX = (width - listW) / 2;
    int visible = std::min((int)lanRows.size(), maxRows);
    int blockH = (visible + 1) * rowH + 6;   // +1 for the header row
    int listY = height - blockH - 4;          // 4px above the bottom edge
    if (listY < height / 4 + 60) listY = height / 4 + 60;  // never overlap IP

    // Panel behind the whole block for legibility.
    fill(listX - 3, listY - 2, listX + listW + 3, listY + blockH,
         0xC0000000);

    if (lanRows.empty()) {
        drawString(font, L"\u00A77LAN games: searching...", listX + 2,
                   listY + 2, 0xffffff);
    } else {
        drawString(font, L"\u00A7aLAN games:", listX + 2, listY + 2,
                   0xffffff);
        for (int i = 0; i < visible; ++i) {
            int rowY = listY + 2 + (i + 1) * rowH;
            bool hover = (xm >= listX && xm <= listX + listW && ym >= rowY &&
                          ym <= rowY + rowH);
            // Plain bright colour; hover highlights. No section codes in
            // the label itself (see refreshLanRows).
            drawString(font, lanRows[i].label, listX + 2, rowY,
                       hover ? 0xffff66 : 0x66ff66);
        }
    }
}
