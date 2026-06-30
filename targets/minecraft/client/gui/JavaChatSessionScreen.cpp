#include "JavaChatSessionScreen.h"

#include <algorithm>
#include <string>

#include "Button.h"
#include "EditBox.h"
#include "app/include/stubs.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/gui/Screen.h"
#include "minecraft/locale/Language.h"
#include "util/StringHelpers.h"

namespace {

void appendUtf8Codepoint(std::string& out, unsigned int cp) {
    if (cp < 0x80) {
        out.push_back(static_cast<char>(cp));
    } else if (cp < 0x800) {
        out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
        out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
}

bool isSectionSign(wchar_t c) { return c == L'§'; }

}

std::string JavaChatSessionScreen::wideToUtf8(const std::wstring& wide) {
    std::string out;
    out.reserve(wide.size());
    size_t i = 0;
    while (i < wide.size()) {
        unsigned int cp = static_cast<unsigned int>(wide[i]);
        if constexpr (sizeof(wchar_t) == 2) {
            if (cp >= 0xD800 && cp <= 0xDBFF && i + 1 < wide.size()) {
                unsigned int lo = static_cast<unsigned int>(wide[i + 1]);
                if (lo >= 0xDC00 && lo <= 0xDFFF) {
                    cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
                    ++i;
                }
            }
        }
        appendUtf8Codepoint(out, cp);
        ++i;
    }
    return out;
}

std::wstring JavaChatSessionScreen::stripFormatCodes(const std::wstring& s) {
    std::wstring out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (isSectionSign(s[i]) && i + 1 < s.size()) {
            ++i;
            continue;
        }
        out.push_back(s[i]);
    }
    return out;
}

int JavaChatSessionScreen::chatAreaRight() const {
    if (!m_tabList.empty()) return width - kTabPanelWidth - 4;
    return width - 4;
}

int JavaChatSessionScreen::chatAreaBottom() const {
    return height - 36;
}

int JavaChatSessionScreen::logLineHeight() const { return 9; }

void JavaChatSessionScreen::appendLog(const std::wstring& line) {
    m_log.push_back(line);
    while (static_cast<int>(m_log.size()) > kMaxLogLines) {
        m_log.pop_front();
    }
    if (m_logScroll > 0) ++m_logScroll;
}

JavaChatSessionScreen::JavaChatSessionScreen(Screen* lastScreen,
                                             const std::wstring& serverName,
                                             const std::string& host,
                                             uint16_t port,
                                             const std::string& nicknameUtf8)
    : m_lastScreen(lastScreen),
      m_serverName(serverName) {
    m_conn = std::make_unique<JavaConnection>(host, port, nicknameUtf8);
}

JavaChatSessionScreen::~JavaChatSessionScreen() {
    if (m_conn) {
        m_conn->requestDisconnect();
    }
    delete m_input;
    m_input = nullptr;
}

void JavaChatSessionScreen::init() {
    Keyboard::enableRepeatEvents(true);

    const int inputY = height - 24;
    m_input = new EditBox(this, font, 4, inputY, width - 112, 18);
    m_input->setMaxLength(100);
    m_input->focus(true);

    buttons.clear();
    buttons.push_back(
        new Button(ID_DISCONNECT, width - 104, height - 24, 100, 18,
                   L"Disconnect"));

    m_conn->start();
}

void JavaChatSessionScreen::removed() {
    Keyboard::enableRepeatEvents(false);
    if (m_conn) {
        m_conn->requestDisconnect();
    }
    delete m_input;
    m_input = nullptr;
}

void JavaChatSessionScreen::tick() {
    if (m_input != nullptr) m_input->tick();

    if (!m_conn) return;

    std::vector<JavaConnectionEvent> events;
    m_conn->drainEvents(events);

    for (auto& ev : events) {
        switch (ev.type) {
            case JavaConnectionEventType::StateChanged:
                m_connState = ev.state;
                if (ev.state == JavaConnectionState::Playing) {
                    for (Button* b : buttons) {
                        if (b && b->id == ID_DISCONNECT) {
                            b->msg = L"Disconnect";
                        }
                    }
                }
                if (ev.state == JavaConnectionState::Disconnected) {
                    Language* lang = Language::getInstance();
                    for (Button* b : buttons) {
                        if (b && b->id == ID_DISCONNECT) {
                            b->msg = lang->getElement(L"gui.back");
                        }
                    }
                    if (m_input) m_input->active = false;
                }
                break;

            case JavaConnectionEventType::Chat:
                appendLog(ev.text);
                break;

            case JavaConnectionEventType::SystemMessage: {
                appendLog(L"§7" + ev.text);
                break;
            }

            case JavaConnectionEventType::Disconnected:
                m_disconnectReason = ev.text;
                m_connState = JavaConnectionState::Disconnected;
                if (!ev.text.empty()) {
                    appendLog(L"§c[Disconnected] " + ev.text);
                }
                {
                    Language* lang = Language::getInstance();
                    for (Button* b : buttons) {
                        if (b && b->id == ID_DISCONNECT) {
                            b->msg = lang->getElement(L"gui.back");
                        }
                    }
                }
                if (m_input) m_input->active = false;
                break;

            case JavaConnectionEventType::TabListReplaced:
                m_tabList = ev.tabList;
                break;

            case JavaConnectionEventType::TabHeaderFooter:
                m_tabHeader = ev.tabHeader;
                m_tabFooter = ev.tabFooter;
                break;

            case JavaConnectionEventType::ChunkData:
            case JavaConnectionEventType::PlayerTeleport:
            case JavaConnectionEventType::SpawnMob:
            case JavaConnectionEventType::EntityDestroy:
            case JavaConnectionEventType::EntityMove:
            case JavaConnectionEventType::EntityTeleport:
                break;
        }
    }
}

void JavaChatSessionScreen::submitChat() {
    if (!m_input || !m_conn) return;
    std::wstring text = trimString(m_input->getValue());
    if (text.empty()) return;
    appendLog(L"§f<You> " + text);
    m_conn->sendChat(wideToUtf8(text));
    m_input->setValue(L"");
}

void JavaChatSessionScreen::buttonClicked(Button* button) {
    if (!button || !button->active) return;
    if (button->id == ID_DISCONNECT) {
        if (m_conn) m_conn->requestDisconnect();
        minecraft->setScreen(m_lastScreen);
    }
}

void JavaChatSessionScreen::keyPressed(wchar_t ch, int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE) {
        if (m_conn) m_conn->requestDisconnect();
        minecraft->setScreen(m_lastScreen);
        return;
    }
    if (ch == L'\r' || ch == L'\n' || eventKey == Keyboard::KEY_RETURN) {
        if (m_connState == JavaConnectionState::Playing) {
            submitChat();
        }
        return;
    }
    if (m_input && m_input->active) {
        m_input->keyPressed(ch, eventKey);
    }
}

void JavaChatSessionScreen::mouseClicked(int x, int y, int buttonNum) {
    Screen::mouseClicked(x, y, buttonNum);
    if (minecraft->screen != this) return;
    if (m_input && m_input->active) m_input->mouseClicked(x, y, buttonNum);
}

void JavaChatSessionScreen::render(int xm, int ym, float a) {
    renderBackground();

    std::wstring title = L"Java Edition » " + m_serverName;
    drawCenteredString(font, title, width / 2, 4, 0xffffff);

    std::wstring statusLine;
    switch (m_connState) {
        case JavaConnectionState::Connecting:
            statusLine = L"§7Connecting…";
            break;
        case JavaConnectionState::LoggingIn:
            statusLine = L"§7Logging in…";
            break;
        case JavaConnectionState::Playing:
            statusLine = L"§aConnected";
            break;
        case JavaConnectionState::Disconnected:
            if (!m_disconnectReason.empty()) {
                statusLine =
                    L"§cDisconnected: " +
                    stripFormatCodes(m_disconnectReason).substr(
                        0, 60);
            } else {
                statusLine = L"§cDisconnected";
            }
            break;
    }
    drawString(font, statusLine, 4, 14, 0xffffff);

    const int logX = 4;
    const int logTop = 26;
    const int logBottom = chatAreaBottom();
    const int logRight = chatAreaRight();
    const int lineH = logLineHeight();
    const int maxVisible =
        std::min(kMaxVisibleLines, (logBottom - logTop) / lineH);

    fill(logX - 2, logTop - 2, logRight + 2, logBottom + 2, 0x90000000);

    const int totalLines = static_cast<int>(m_log.size());
    if (m_logScroll > totalLines - maxVisible)
        m_logScroll = std::max(0, totalLines - maxVisible);
    if (m_logScroll < 0) m_logScroll = 0;

    const int visibleEnd = totalLines - m_logScroll;
    const int visibleStart = std::max(0, visibleEnd - maxVisible);
    for (int i = visibleStart; i < visibleEnd; ++i) {
        const int row = i - visibleStart;
        const int lineY = logTop + row * lineH;
        drawString(font, m_log[static_cast<size_t>(i)], logX, lineY, 0xffffff);
    }

    if (m_logScroll > 0) {
        drawString(font,
                   L"§7↑ " + toWString<int>(m_logScroll) + L" more",
                   logX, logBottom - lineH, 0xffffff);
    }

    if (!m_tabList.empty()) {
        const int panelX = width - kTabPanelWidth;
        const int panelY = 26;
        fill(panelX - 2, panelY - 2, width - 2, logBottom + 2, 0x90000000);

        int ty = panelY;
        if (!m_tabHeader.empty()) {
            drawString(font, m_tabHeader, panelX, ty, 0xaaaaff);
            ty += 9;
        }
        drawString(font, L"§eOnline:", panelX, ty, 0xffffff);
        ty += 9;

        const int maxTabRows = (logBottom - ty) / 9 - 1;
        const int shown = std::min(static_cast<int>(m_tabList.size()),
                                   std::max(0, maxTabRows));
        for (int i = 0; i < shown; ++i) {
            const auto& e = m_tabList[static_cast<size_t>(i)];
            std::wstring row = e.name;
            if (e.ping >= 0) {
                row += L" §7" + toWString<int>(e.ping) + L"ms";
            }
            drawString(font, row, panelX, ty, 0xffffff);
            ty += 9;
        }
        if (static_cast<int>(m_tabList.size()) > shown) {
            drawString(font,
                       L"§7+" + toWString<int>(
                           static_cast<int>(m_tabList.size()) - shown) +
                           L" more",
                       panelX, ty, 0xffffff);
        }
        if (!m_tabFooter.empty()) {
            drawString(font, m_tabFooter, panelX, logBottom - 9, 0xaaaaff);
        }
    }

    if (m_input) {
        if (m_connState == JavaConnectionState::Playing) {
            drawString(font, L"§7Chat:", 4, height - 35, 0xffffff);
        } else if (m_connState == JavaConnectionState::Disconnected) {
            drawString(font, L"§cPress Back to return", 4, height - 35,
                       0xffffff);
        } else {
            drawString(font, L"§7Waiting for connection…", 4,
                       height - 35, 0xffffff);
        }
        m_input->render();
    }

    Screen::render(xm, ym, a);
}
