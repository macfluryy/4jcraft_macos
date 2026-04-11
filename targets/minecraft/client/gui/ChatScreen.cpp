#include "ChatScreen.h"

#include <memory>
#include <algorithm>

#include "app/include/stubs.h"
#include "util/StringHelpers.h"
#include "minecraft/SharedConstants.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/gui/Gui.h"
#include "minecraft/client/gui/Screen.h"
#include "minecraft/client/multiplayer/MultiPlayerLocalPlayer.h"

ChatScreen::ChatScreen() : frame(0), message(L"") { 
    // 4J macOS - initialize message
}

void ChatScreen::init() { 
    Keyboard::enableRepeatEvents(true); 
}

void ChatScreen::removed() { 
    Keyboard::enableRepeatEvents(false); 
}

void ChatScreen::tick() { 
    frame++; 
}

void ChatScreen::keyPressed(wchar_t ch, int eventKey) {
    // 4J macOS - improved escape handling
    if (eventKey == Keyboard::KEY_ESCAPE) {
        if (minecraft != nullptr) {
            minecraft->setScreen(nullptr);
        }
        return;
    }
    
    // 4J macOS - improved return handling with better message validation
    if (eventKey == Keyboard::KEY_RETURN) {
        std::wstring msg = trimString(message);
        if (!msg.empty()) {
            if (minecraft != nullptr && minecraft->player != nullptr) {
                if (!minecraft->handleClientSideCommand(msg)) {
                    minecraft->player->chat(msg);
                }
            }
        }
        if (minecraft != nullptr) {
            minecraft->setScreen(nullptr);
        }
        return;
    }
    
    // 4J macOS - improved backspace handling
    if (eventKey == Keyboard::KEY_BACK) {
        if (!message.empty()) {
            message = message.substr(0, message.length() - 1);
        }
        return;
    }
    
    // 4J macOS - improved character validation
    if (SharedConstants::acceptableLetters.find(ch) != std::wstring::npos) {
        if (message.length() < (size_t)SharedConstants::maxChatLength) {
            message += ch;
        }
    }
}

void ChatScreen::render(int xm, int ym, float a) {
    // Draw chat input box background
    fill(2, height - 14, width - 2, height - 2, 0x80000000);
    
    // 4J macOS - improved cursor rendering
    bool showCursor = frame / 6 % 2 == 0;
    std::wstring displayMessage = L"> " + message;
    if (showCursor) {
        displayMessage += L"_";
    }
    
    drawString(font, displayMessage, 4, height - 12, 0xe0e0e0);

    Screen::render(xm, ym, a);
}

void ChatScreen::mouseClicked(int x, int y, int buttonNum) {
    if (buttonNum == 0) {
        // 4J macOS - improved null checking and string handling
        if (minecraft != nullptr && minecraft->gui != nullptr) {
            if (!minecraft->gui->selectedName.empty()) {
                // Add space before name if needed
                if (!message.empty() && message[message.length() - 1] != L' ') {
                    message += L" ";
                }
                message += minecraft->gui->selectedName;
                
                // Truncate if too long
                size_t maxLength = SharedConstants::maxChatLength;
                if (message.length() > maxLength) {
                    message = message.substr(0, maxLength);
                }
            } else {
                Screen::mouseClicked(x, y, buttonNum);
            }
        } else {
            Screen::mouseClicked(x, y, buttonNum);
        }
    }
}
