#pragma once
#include <deque>
#include <memory>
#include <string>
#include <vector>

#include "Screen.h"
#include "app/common/src/JavaEdition/JavaConnection.h"

class EditBox;
class Button;

class JavaChatSessionScreen : public Screen {
public:
    JavaChatSessionScreen(Screen* lastScreen,
                          const std::wstring& serverName,
                          const std::string& host,
                          uint16_t port,
                          const std::string& nicknameUtf8);
    ~JavaChatSessionScreen();

    void init() override;
    void tick() override;
    void render(int xm, int ym, float a) override;
    void removed() override;

protected:
    void buttonClicked(Button* button) override;
    void keyPressed(wchar_t ch, int eventKey) override;
    void mouseClicked(int x, int y, int buttonNum) override;

private:
    static constexpr int kMaxLogLines = 200;
    static constexpr int kMaxVisibleLines = 18;
    static constexpr int kTabPanelWidth = 160;

    enum ButtonId { ID_DISCONNECT = 1 };

    Screen* m_lastScreen;
    std::wstring m_serverName;

    std::unique_ptr<JavaConnection> m_conn;

    void appendLog(const std::wstring& line);
    void submitChat();

    int chatAreaRight() const;
    int chatAreaBottom() const;
    int logLineHeight() const;

    static std::wstring stripFormatCodes(const std::wstring& s);

    static std::string wideToUtf8(const std::wstring& wide);

    JavaConnectionState m_connState = JavaConnectionState::Connecting;
    std::wstring m_disconnectReason;

    std::deque<std::wstring> m_log;
    int m_logScroll = 0;

    std::vector<JavaTabListEntry> m_tabList;
    std::wstring m_tabHeader;
    std::wstring m_tabFooter;

    EditBox* m_input = nullptr;
};