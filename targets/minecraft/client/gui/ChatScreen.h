#pragma once
#include <deque>
#include <string>
#include "Screen.h"

class ChatScreen : public Screen {
protected:
    std::wstring message;

private:
    int frame;

    size_t cursorPos;

    int m_historyIndex;
    std::wstring m_pendingDraft;

    static const std::size_t MAX_HISTORY = 100;
    static std::deque<std::wstring> s_history;
    static int s_scrollOffset;

    void browseHistoryUp();
    void browseHistoryDown();
    void recordHistory(const std::wstring& submitted);
    void renderScrollableLog(int x, int y, int maxLines, int boxWidth);
    void scrollBy(int delta);

public:
    ChatScreen();
    
    virtual void init() override;
    virtual void removed() override;
    virtual void tick() override;

protected:
    void keyPressed(wchar_t ch, int eventKey) override;

public:
    void render(int xm, int ym, float a) override;

protected:
    void mouseClicked(int x, int y, int buttonNum) override;

public:
    std::wstring getMessage() const { return message; }
    void setMessage(const std::wstring& msg) { 
        message = msg;
        cursorPos = msg.length();
    }
    void clearMessage() { 
        message.clear();
        cursorPos = 0;
    }
    size_t getMessageLength() const { return message.length(); }
    bool isMessageEmpty() const { return message.empty(); }
};