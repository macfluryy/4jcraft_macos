#pragma once
#include <string>

#include "Screen.h"

class ChatScreen : public Screen {
protected:
    std::wstring message;

private:
    int frame;

    // 4J macOS - add cursor position tracking
    size_t cursorPos;

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
