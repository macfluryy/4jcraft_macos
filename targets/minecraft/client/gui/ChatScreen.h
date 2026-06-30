#pragma once
#include <deque>
#include <string>

#include "Screen.h"

// Chat input screen with vanilla-Minecraft-style command/message history.
//
// History is persisted across ChatScreen instances by living in a static
// std::deque keyed by insertion order. Up to MAX_HISTORY messages are
// retained; consecutive duplicates are coalesced. Pressing the up arrow
// walks backwards through the history, the down arrow walks forwards;
// when the user steps past the most-recent entry the prompt is restored
// to whatever they had typed before they started browsing.
class ChatScreen : public Screen {
protected:
    std::wstring message;

private:
    int frame;

    // 4J macOS - cursor position tracking, kept simple: cursor sits at
    // the end of the message after every mutation so we don't need to
    // implement caret-aware editing alongside history browsing.
    size_t cursorPos;

    // ---- History browsing state ---------------------------------------
    // m_historyIndex == -1 means "not browsing history right now". When
    // the player presses Up we snapshot the in-progress text into
    // m_pendingDraft so we can restore it on the way back.
    int m_historyIndex;
    std::wstring m_pendingDraft;

    // The history itself is shared across every ChatScreen instance for
    // the lifetime of the process.
    static const std::size_t MAX_HISTORY = 100;
    static std::deque<std::wstring> s_history;

    // 4J macOS - chat scroll state. The Iggy/Flash chat scene only ever
    // shows the most recent 10 messages with no scrolling, so long
    // command output (e.g. /help, /list) is impossible to read once it
    // pushes off the visible window. We render an extended chat
    // overlay while the chat input is open and let the player scroll
    // it with the mouse wheel and Page Up / Page Down. Offset 0
    // means "show newest messages at the bottom"; positive offsets
    // expose older messages.
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
    // 4J macOS - utility methods
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
