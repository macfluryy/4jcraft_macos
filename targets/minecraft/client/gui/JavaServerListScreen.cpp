#include "JavaServerListScreen.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include "Button.h"
#include "EditBox.h"
#include "app/include/stubs.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/Options.h"
#include "minecraft/client/gui/Screen.h"
#include "minecraft/locale/Language.h"
#include "util/StringHelpers.h"
#include "app/common/src/JavaEdition/JavaAddressParser.h"
#include "app/common/src/JavaEdition/JavaServerEntry.h"
#include "app/common/src/JavaEdition/JavaServerProxy.h"
#include "JavaChatSessionScreen.h"
#include "app/common/App_enums.h"
#include "app/common/src/Network/GameNetworkManager.h"
#include "app/common/src/UI/All Platforms/UIStructs.h"
#include "app/common/src/UI/UIController.h"
#include "app/mac/MacGame.h"
#include "app/mac/Mac_UIController.h"
#include "platform/sdl2/Profile.h"

namespace {

std::string narrowAsciiNick(const std::wstring& s) {
    std::string out;
    out.reserve(s.size());
    for (wchar_t wc : s) {
        out.push_back(static_cast<char>(static_cast<unsigned int>(wc) & 0x7f));
    }
    return out;
}

}

JavaServerListScreen::JavaServerListScreen(Screen* lastScreen)
    : m_lastScreen(lastScreen),
      m_store(Minecraft::getWorkingDirectory()),
      m_pinger(nullptr) {}

JavaServerListScreen::~JavaServerListScreen() {
    delete m_pinger;
    m_pinger = nullptr;
    clearEditBoxes();
}

void JavaServerListScreen::init() {
    Language* language = Language::getInstance();
    (void)language;

    Keyboard::enableRepeatEvents(true);
    JavaServerListStore::LoadResult loaded = m_store.load();
    m_list = loaded.list;
    m_corruptNotice = loaded.hadCorruptLines;

    if (m_pinger == nullptr) {
        m_pinger = new JavaServerPinger();
    }

    m_mode = Mode::List;
    m_selected = -1;
    m_editIndex = -1;
    m_pendingModeSwitch = false;
    m_statusMessage.clear();

    rebuildButtons();
    refreshPings();
}

void JavaServerListScreen::tick() {
    if (m_pendingModeSwitch) {
        applyPendingModeSwitch();
    }

    if (m_nameEdit != nullptr) m_nameEdit->tick();
    if (m_addrEdit != nullptr) m_addrEdit->tick();
    if (m_pinger != nullptr) {
        const size_t n = std::min(m_results.size(), m_pingIds.size());
        for (size_t i = 0; i < n; ++i) {
            if (m_pingIds[i] < 0) continue;
            PingResult r;
            if (m_pinger->pollResult(m_pingIds[i], r)) {
                m_results[i] = r;
            }
        }
    }
}

void JavaServerListScreen::removed() {
    Keyboard::enableRepeatEvents(false);
    if (m_pinger != nullptr) {
        m_pinger->cancelAll();
        delete m_pinger;
        m_pinger = nullptr;
    }
    clearEditBoxes();
}

void JavaServerListScreen::clearEditBoxes() {
    delete m_nameEdit;
    m_nameEdit = nullptr;
    delete m_addrEdit;
    m_addrEdit = nullptr;
}

void JavaServerListScreen::enterListMode() {
    m_pendingModeSwitch = true;
    m_pendingMode = Mode::List;
}

void JavaServerListScreen::enterAddMode() {
    m_pendingModeSwitch = true;
    m_pendingMode = Mode::AddEdit;
    m_pendingEditIndex = -1;
}

void JavaServerListScreen::enterEditMode(int index) {
    if (index < 0 || static_cast<size_t>(index) >= m_list.size()) return;
    m_pendingModeSwitch = true;
    m_pendingMode = Mode::AddEdit;
    m_pendingEditIndex = index;
}

void JavaServerListScreen::applyPendingModeSwitch() {
    m_pendingModeSwitch = false;
    clickedButton = nullptr;
    clearEditBoxes();

    m_mode = m_pendingMode;
    m_statusMessage.clear();

    if (m_mode == Mode::AddEdit) {
        m_editIndex = m_pendingEditIndex;
        int fieldX = width / 2 - 100;
        m_nameEdit = new EditBox(this, font, fieldX, height / 4 + 12, 200, 20);
        m_nameEdit->setMaxLength(64);
        m_addrEdit = new EditBox(this, font, fieldX, height / 4 + 60, 200, 20);
        m_addrEdit->setMaxLength(128);

        if (m_editIndex >= 0 &&
            static_cast<size_t>(m_editIndex) < m_list.size()) {
            const JavaServerEntry& e = m_list.at(m_editIndex);
            m_nameEdit->setValue(e.name);
            std::wstring addr;
            for (unsigned char c : e.host) addr.push_back((wchar_t)c);
            if (e.port != 25565) addr += L":" + toWString<int>((int)e.port);
            m_addrEdit->setValue(addr);
        }
        m_nameEdit->focus(true);
        m_addrEdit->focus(false);
    } else {
        m_editIndex = -1;
        refreshPings();
    }

    rebuildButtons();
}

void JavaServerListScreen::rebuildButtons() {
    for (Button* b : buttons) {
        delete b;
    }
    buttons.clear();

    Language* language = Language::getInstance();

    if (m_mode == Mode::List) {
        const int bw = 72;
        const int bh = 20;
        const int gap = 4;
        const int total = bw * 5 + gap * 4;
        int x = width / 2 - total / 2;
        const int y = height - 52;
        buttons.push_back(new Button(ID_CONNECT, x, y, bw, bh,
                                     language->getElement(L"selectServer.select")));
        x += bw + gap;
        buttons.push_back(new Button(ID_ADD, x, y, bw, bh,
                                     language->getElement(L"selectServer.add")));
        x += bw + gap;
        buttons.push_back(new Button(ID_EDIT, x, y, bw, bh,
                                     language->getElement(L"selectServer.edit")));
        x += bw + gap;
        buttons.push_back(new Button(ID_DELETE, x, y, bw, bh,
                                     language->getElement(L"selectServer.delete")));
        x += bw + gap;
        buttons.push_back(new Button(ID_REFRESH, x, y, bw, bh,
                                     language->getElement(L"selectServer.refresh")));
        buttons.push_back(new Button(ID_BACK, width / 2 - 100, height - 28,
                                     language->getElement(L"gui.cancel")));
    } else {
        const int bw = 98;
        const int y = height / 4 + 96 + 12;
        buttons.push_back(new Button(ID_SAVE, width / 2 - 100, y, bw, 20,
                                     language->getElement(L"gui.done")));
        buttons.push_back(new Button(ID_CANCEL, width / 2 + 2, y, bw, 20,
                                     language->getElement(L"gui.cancel")));
    }

    updateButtonStates();
}

void JavaServerListScreen::updateButtonStates() {
    if (m_mode != Mode::List) return;
    const bool hasSelection =
        m_selected >= 0 && static_cast<size_t>(m_selected) < m_list.size();
    for (Button* b : buttons) {
        if (b == nullptr) continue;
        if (b->id == ID_CONNECT || b->id == ID_EDIT || b->id == ID_DELETE) {
            b->active = hasSelection;
        }
    }
}

void JavaServerListScreen::commitAddEdit() {
    if (m_nameEdit == nullptr || m_addrEdit == nullptr) return;

    const std::wstring name = trimString(m_nameEdit->getValue());
    const std::wstring addr = trimString(m_addrEdit->getValue());
    const ParsedAddress parsed = parseAddress(addr);
    if (parsed.hostEmpty) {
        m_statusMessage = L"Server address must not be empty";
        return;
    }

    JavaServerEntry entry;
    entry.name = name;
    entry.host = parsed.host;
    entry.port = parsed.port;

    bool ok;
    if (m_editIndex >= 0 &&
        static_cast<size_t>(m_editIndex) < m_list.size()) {
        ok = m_list.edit(static_cast<size_t>(m_editIndex), entry);
    } else {
        ok = m_list.add(entry);
    }
    if (!ok) {
        m_statusMessage = L"Server address must not be empty";
        return;
    }

    m_store.save(m_list);
    m_selected = -1;
    enterListMode();
}

void JavaServerListScreen::deleteSelected() {
    if (m_selected < 0 || static_cast<size_t>(m_selected) >= m_list.size()) {
        return;
    }
    m_list.remove(static_cast<size_t>(m_selected));
    m_store.save(m_list);
    m_selected = -1;
    enterListMode();
}

void JavaServerListScreen::connectSelected() {
    if (m_selected < 0 || static_cast<size_t>(m_selected) >= m_list.size()) {
        return;
    }
    const JavaServerEntry& entry = m_list.at(static_cast<size_t>(m_selected));

    std::wstring nick = minecraft->options->lastMpNickname;
    if (nick.empty()) nick = L"Player";

    std::wstring displayName = entry.name;
    if (displayName.empty()) {
        for (unsigned char c : entry.host) displayName.push_back((wchar_t)c);
    }

    if (g_activeJavaProxy != nullptr) {
        g_activeJavaProxy->requestStop();
        delete g_activeJavaProxy;
        g_activeJavaProxy = nullptr;
    }
    auto* proxy = new JavaServerProxy();
    int proxyPort = proxy->startListening();
    if (proxyPort <= 0) {
        delete proxy;
        minecraft->setScreen(new JavaChatSessionScreen(
            this, displayName, entry.host, entry.port, narrowAsciiNick(nick)));
        return;
    }
    proxy->startWorker(entry.host, entry.port, narrowAsciiNick(nick),
                       displayName);
    g_activeJavaProxy = proxy;

    if (!app.TemporaryDirectConnectStartEx("127.0.0.1", proxyPort,
                                            false,
                                           nick.c_str())) {
        proxy->requestStop();
        delete proxy;
        g_activeJavaProxy = nullptr;
        return;
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
}

void JavaServerListScreen::refreshPings() {
    if (m_pinger == nullptr) return;

    const size_t n = m_list.size();
    m_results.assign(n, PingResult{});
    m_pingIds.assign(n, -1);

    for (size_t i = 0; i < n; ++i) {
        const JavaServerEntry& e = m_list.at(i);
        const int id = m_nextPingId++;
        m_pingIds[i] = id;
        m_pinger->startPing(id, e.host, e.port);
    }
}

int JavaServerListScreen::listTop() const { return 54; }
int JavaServerListScreen::rowHeight() const { return 24; }

int JavaServerListScreen::maxVisibleRows() const {
    const int avail = (height - 60) - listTop();
    if (avail <= 0) return 0;
    return avail / rowHeight();
}

std::wstring JavaServerListScreen::formatStatusLine(int index) const {
    if (index < 0 || static_cast<size_t>(index) >= m_results.size()) {
        return L"";
    }
    const PingResult& r = m_results[index];
    switch (r.status) {
        case PingStatus::Querying:
            return L"\u00A77Querying...";
        case PingStatus::Unreachable:
            return L"\u00A7cCan't reach server";
        case PingStatus::Online: {
            std::wstring line;
            if (!r.motd.empty()) line += r.motd;
            std::wstring players;
            if (r.online >= 0 && r.max >= 0) {
                players = toWString<int>(r.online) + L"/" + toWString<int>(r.max);
            }
            std::wstring ping;
            if (r.pingMs >= 0) ping = toWString<int>(r.pingMs) + L"ms";
            std::wstring tail;
            if (!players.empty()) tail += L"  \u00A7a" + players;
            if (!ping.empty()) tail += L"  \u00A7b" + ping;
            return line + tail;
        }
    }
    return L"";
}

void JavaServerListScreen::buttonClicked(Button* button) {
    if (button == nullptr || !button->active) return;

    switch (button->id) {
        case ID_BACK:
            minecraft->setScreen(m_lastScreen);
            return;
        case ID_CONNECT:
            connectSelected();
            return;
        case ID_ADD:
            enterAddMode();
            return;
        case ID_EDIT:
            enterEditMode(m_selected);
            return;
        case ID_DELETE:
            deleteSelected();
            return;
        case ID_REFRESH:
            refreshPings();
            return;
        case ID_SAVE:
            commitAddEdit();
            return;
        case ID_CANCEL:
            enterListMode();
            return;
        default:
            return;
    }
}

void JavaServerListScreen::keyPressed(wchar_t ch, int eventKey) {
    if (m_mode == Mode::AddEdit) {
        if (eventKey == Keyboard::KEY_ESCAPE) {
            enterListMode();
            return;
        }
        if (m_nameEdit != nullptr) m_nameEdit->keyPressed(ch, eventKey);
        if (m_addrEdit != nullptr) m_addrEdit->keyPressed(ch, eventKey);
        if (ch == 13 || eventKey == Keyboard::KEY_RETURN) {
            commitAddEdit();
        }
        return;
    }

    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->setScreen(m_lastScreen);
        return;
    }
}

void JavaServerListScreen::tabPressed() {
    if (m_mode != Mode::AddEdit || m_nameEdit == nullptr ||
        m_addrEdit == nullptr) {
        return;
    }
    const bool nameFocused = m_nameEdit->isFocused();
    m_nameEdit->focus(!nameFocused);
    m_addrEdit->focus(nameFocused);
}

void JavaServerListScreen::mouseClicked(int x, int y, int buttonNum) {
    Screen::mouseClicked(x, y, buttonNum);
    if (minecraft->screen != this) return;

    if (m_mode == Mode::AddEdit) {
        if (m_nameEdit != nullptr) m_nameEdit->mouseClicked(x, y, buttonNum);
        if (m_addrEdit != nullptr) m_addrEdit->mouseClicked(x, y, buttonNum);
        return;
    }

    if (buttonNum != 0) return;
    const int top = listTop();
    const int rh = rowHeight();
    const int listX = width / 2 - 150;
    const int listW = 300;
    const int visible =
        std::min(static_cast<int>(m_list.size()), maxVisibleRows());
    for (int i = 0; i < visible; ++i) {
        const int rowY = top + i * rh;
        if (x >= listX && x <= listX + listW && y >= rowY &&
            y <= rowY + rh) {
            m_selected = i;
            updateButtonStates();
            return;
        }
    }
}


void JavaServerListScreen::render(int xm, int ym, float a) {
    renderBackground();

    drawCenteredString(font, L"Java Edition Servers", width / 2, 20, 0xffffff);

    int noticeY = 34;
    if (m_corruptNotice) {
        drawCenteredString(
            font,
            L"\u00A7eSome saved servers could not be loaded (corrupt data)",
            width / 2, noticeY, 0xffffff);
        noticeY += 10;
    }
    if (!m_statusMessage.empty()) {
        drawCenteredString(font, L"\u00A7c" + m_statusMessage, width / 2,
                           noticeY, 0xffffff);
    }

    if (m_mode == Mode::AddEdit) {
        drawCenteredString(font,
                           m_editIndex >= 0 ? L"Edit Java Server"
                                            : L"Add Java Server",
                           width / 2, height / 4 - 20, 0xffffff);
        if (m_nameEdit != nullptr) {
            drawString(font, L"Server Name", m_nameEdit->x,
                       m_nameEdit->y - 11, 0xa0a0a0);
            m_nameEdit->render();
        }
        if (m_addrEdit != nullptr) {
            drawString(font, L"Server Address (host or host:port)",
                       m_addrEdit->x, m_addrEdit->y - 11, 0xa0a0a0);
            m_addrEdit->render();
        }
        Screen::render(xm, ym, a);
        return;
    }

    if (m_list.size() == 0) {
        drawCenteredString(font,
                           L"\u00A77No Java servers saved yet. Use Add to "
                           L"create one.",
                           width / 2, height / 2 - 10, 0xffffff);
    } else {
        const int top = listTop();
        const int rh = rowHeight();
        const int listX = width / 2 - 150;
        const int listW = 300;
        const int visible =
            std::min(static_cast<int>(m_list.size()), maxVisibleRows());
        for (int i = 0; i < visible; ++i) {
            const int rowY = top + i * rh;
            const bool selected = (i == m_selected);
            const bool hover = (xm >= listX && xm <= listX + listW &&
                                ym >= rowY && ym <= rowY + rh);
            if (selected) {
                fill(listX - 2, rowY - 1, listX + listW + 2, rowY + rh - 1,
                     0x80ffffff);
            } else if (hover) {
                fill(listX - 2, rowY - 1, listX + listW + 2, rowY + rh - 1,
                     0x40ffffff);
            }
            const JavaServerEntry& e = m_list.at(static_cast<size_t>(i));
            std::wstring nameLine = e.name;
            if (nameLine.empty()) {
                for (unsigned char c : e.host) nameLine.push_back((wchar_t)c);
            }
            drawString(font, nameLine, listX, rowY + 1, 0xffffff);
            drawString(font, formatStatusLine(i), listX, rowY + 12, 0xa0a0a0);
        }
    }

    Screen::render(xm, ym, a);
}