#include "ChatScreen.h"

#include <memory>
#include <algorithm>

#include "platform/sdl2/Clipboard.h"
#include "platform/sdl2/Input.h"
#include "app/include/stubs.h"
#include "util/StringHelpers.h"
#include "minecraft/SharedConstants.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/gui/Font.h"
#include "minecraft/client/gui/Gui.h"
#include "minecraft/client/gui/Screen.h"
#include "minecraft/client/multiplayer/MultiPlayerLocalPlayer.h"

// Process-wide history buffer shared by every ChatScreen instance. Newest
// entries are pushed to the back; oldest entries are pruned from the
// front when MAX_HISTORY is reached.
std::deque<std::wstring> ChatScreen::s_history;

int ChatScreen::s_scrollOffset = 0;

ChatScreen::ChatScreen()
    : message(L""),
      frame(0),
      cursorPos(0),
      m_historyIndex(-1),
      m_pendingDraft() {}

void ChatScreen::init() { 
    Keyboard::enableRepeatEvents(true); 
}

void ChatScreen::removed() { 
    Keyboard::enableRepeatEvents(false); 
}

void ChatScreen::tick() { 
    frame++; 

    int wheel = InputManager.GetChatScrollDelta();
    if (wheel != 0) {
        scrollBy(wheel * 3);
    }
}

void ChatScreen::scrollBy(int delta) {
    s_scrollOffset += delta;
    // Clamp to the available message buffer.
    if (minecraft != nullptr && minecraft->gui != nullptr &&
        minecraft->player != nullptr) {
        int iPad = minecraft->player->GetXboxPad();
        int total = (int)minecraft->gui->getMessagesCount(iPad);
        // We always show 20 lines worth, so the highest meaningful
        // offset is total - 20.
        int maxOffset = total - 20;
        if (maxOffset < 0) maxOffset = 0;
        if (s_scrollOffset > maxOffset) s_scrollOffset = maxOffset;
    }
    if (s_scrollOffset < 0) s_scrollOffset = 0;
}

void ChatScreen::recordHistory(const std::wstring& submitted) {
    if (submitted.empty()) return;
    // Don't store consecutive duplicates: if the most recent entry is
    // already this message, leave the history alone.
    if (!s_history.empty() && s_history.back() == submitted) return;
    s_history.push_back(submitted);
    while (s_history.size() > MAX_HISTORY) {
        s_history.pop_front();
    }
}

void ChatScreen::browseHistoryUp() {
    if (s_history.empty()) return;

    // Stepping into history for the first time? Stash whatever the
    // player was typing so Down can bring it back.
    if (m_historyIndex == -1) {
        m_pendingDraft = message;
        m_historyIndex = static_cast<int>(s_history.size()) - 1;
    } else if (m_historyIndex > 0) {
        m_historyIndex--;
    }
    // else: already at the oldest entry, stay put.

    message = s_history[static_cast<std::size_t>(m_historyIndex)];
    cursorPos = message.length();
}

void ChatScreen::browseHistoryDown() {
    if (m_historyIndex == -1) return;

    // Move forward in history; once we step past the newest entry we
    // restore the in-progress draft and exit history-browsing mode.
    if (m_historyIndex + 1 < static_cast<int>(s_history.size())) {
        m_historyIndex++;
        message = s_history[static_cast<std::size_t>(m_historyIndex)];
    } else {
        m_historyIndex = -1;
        message = m_pendingDraft;
        m_pendingDraft.clear();
    }
    cursorPos = message.length();
}

void ChatScreen::keyPressed(wchar_t ch, int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE) {
        if (minecraft != nullptr) {
            minecraft->setScreen(nullptr);
        }
        return;
    }
    
    if (eventKey == Keyboard::KEY_RETURN) {
        std::wstring msg = trimString(message);
        if (!msg.empty()) {
            recordHistory(msg);
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
    
    if (eventKey == Keyboard::KEY_BACK) {
        if (cursorPos > 0 && !message.empty()) {
            message.erase(cursorPos - 1, 1);
            cursorPos--;
            // Editing the displayed text means the player has wandered
            // off the history rail; treat this as their new draft.
            if (m_historyIndex != -1) {
                m_pendingDraft = message;
                m_historyIndex = -1;
            }
        }
        return;
    }

    if (eventKey == Keyboard::KEY_DELETE) {
        if (cursorPos < message.length()) {
            message.erase(cursorPos, 1);
            if (m_historyIndex != -1) {
                m_pendingDraft = message;
                m_historyIndex = -1;
            }
        }
        return;
    }

    if (eventKey == Keyboard::KEY_LEFT) {
        if (cursorPos > 0) cursorPos--;
        return;
    }
    if (eventKey == Keyboard::KEY_RIGHT) {
        if (cursorPos < message.length()) cursorPos++;
        return;
    }
    if (eventKey == Keyboard::KEY_HOME) {
        cursorPos = 0;
        return;
    }
    if (eventKey == Keyboard::KEY_END) {
        cursorPos = message.length();
        return;
    }

    if (eventKey == Keyboard::KEY_UP) {
        browseHistoryUp();
        return;
    }
    if (eventKey == Keyboard::KEY_DOWN) {
        browseHistoryDown();
        return;
    }

    if (eventKey == 75) {
        scrollBy(10);
        return;
    }
    if (eventKey == 78) {
        scrollBy(-10);
        return;
    }
    
    if (SharedConstants::acceptableLetters.find(ch) != std::wstring::npos) {
        if (message.length() < (size_t)SharedConstants::maxChatLength) {
            message.insert(cursorPos, 1, ch);
            cursorPos++;
            // As soon as the user types into a history entry, treat the
            // current contents as a new draft and stop browsing.
            if (m_historyIndex != -1) {
                m_pendingDraft = message;
                m_historyIndex = -1;
            }
        }
    }
}

void ChatScreen::renderScrollableLog(int x0, int y0, int maxLines,
                                     int boxWidth) {
    if (minecraft == nullptr || minecraft->gui == nullptr ||
        minecraft->player == nullptr || font == nullptr) {
        return;
    }

    int iPad = minecraft->player->GetXboxPad();
    int total = (int)minecraft->gui->getMessagesCount(iPad);
    if (total == 0) return;

    // guiMessages[] in Gui.cpp is stored newest-first (index 0 = most
    // recent). With offset 0 we want to show indices [0..maxLines-1].
    // With positive offset we shift the visible window deeper into the
    // history, so visible[0] = guiMessages[offset].
    int start = s_scrollOffset;
    if (start > total - 1) start = total - 1;
    if (start < 0) start = 0;

    int visibleCount = std::min(maxLines, total - start);
    if (visibleCount <= 0) return;

    const int lineH = 9;
    int boxH = visibleCount * lineH + 2;

    // Translucent panel behind the text.
    fill(x0, y0 - boxH, x0 + boxWidth, y0, 0x80000000);

    // Render newest message at the BOTTOM (right above the input).
    // We iterate visibleCount slots; slot 0 is bottom.
    for (int i = 0; i < visibleCount; ++i) {
        int messageIdx = start + i;
        std::wstring msg = minecraft->gui->getMessage(iPad, messageIdx);
        int yLine = y0 - 2 - (i + 1) * lineH;
        font->drawShadow(msg, x0 + 2, yLine, 0xFFFFFF);
    }

    // Scroll indicator on the right edge - shows how far into the
    // history the user has scrolled. Hidden when fully at the bottom
    // and the buffer is short enough to fit on screen.
    int maxOffset = total - maxLines;
    if (maxOffset > 0) {
        int trackH = boxH;
        int trackX = x0 + boxWidth - 4;
        fill(trackX, y0 - trackH, trackX + 2, y0, 0x40FFFFFF);
        // Thumb size proportional to visible window.
        int thumbH = std::max(10, trackH * maxLines / total);
        // 0 offset -> thumb at the BOTTOM of the track (newest is
        // visible). maxOffset -> thumb at the TOP (oldest is visible).
        float t = (float)s_scrollOffset / (float)maxOffset;
        int thumbY = y0 - thumbH - (int)((trackH - thumbH) * t);
        fill(trackX, thumbY, trackX + 2, thumbY + thumbH, 0xC0FFFFFF);
    }
}

void ChatScreen::render(int xm, int ym, float a) {
    renderScrollableLog(2, height - 16, 20, width - 4);

    // Draw chat input box background
    fill(2, height - 14, width - 2, height - 2, 0x80000000);

    bool showCursor = frame / 6 % 2 == 0;
    const std::wstring prefix = L"> ";
    drawString(font, prefix + message, 4, height - 12, 0xe0e0e0);
    if (showCursor) {
        std::wstring before = prefix + message.substr(0, cursorPos);
        int caretX = 4 + font->width(before);
        // Pixel-thin caret block.
        drawString(font, L"_", caretX, height - 12, 0xffffff);
    }

    Screen::render(xm, ym, a);
}

void ChatScreen::mouseClicked(int x, int y, int buttonNum) {
    if (buttonNum == 0) {
        if (minecraft != nullptr && minecraft->gui != nullptr &&
            minecraft->player != nullptr) {
            const int x0 = 2;
            const int y0 = height - 16;
            const int lineH = 9;
            const int maxLines = 20;
            const int boxWidth = width - 4;
            int iPad = minecraft->player->GetXboxPad();
            int total = (int)minecraft->gui->getMessagesCount(iPad);
            int start = s_scrollOffset;
            if (start > total - 1) start = total - 1;
            if (start < 0) start = 0;
            int visibleCount = std::min(maxLines, total - start);
            if (visibleCount > 0 && x >= x0 && x < x0 + boxWidth) {
                for (int i = 0; i < visibleCount; ++i) {
                    int yLine = y0 - 2 - (i + 1) * lineH;
                    if (y >= yLine && y < yLine + lineH) {
                        int messageIdx = start + i;
                        std::wstring msg =
                            minecraft->gui->getMessage(iPad, messageIdx);
                        if (!msg.empty() && Clipboard::SetText(msg)) {
                            // Echo a tiny confirmation in the chat. We
                            // do this through Gui::addMessage so the new
                            // line appears in the same overlay.
                            std::wstring echo =
                                L"\u00A7e[copied to clipboard]";
                            minecraft->gui->addMessage(echo, iPad);
                        }
                        return;
                    }
                }
            }
        }

        if (minecraft != nullptr && minecraft->gui != nullptr) {
            if (!minecraft->gui->selectedName.empty()) {
                std::wstring insertion = minecraft->gui->selectedName;
                if (cursorPos > 0 && message[cursorPos - 1] != L' ') {
                    insertion = L" " + insertion;
                }
                size_t maxLength = SharedConstants::maxChatLength;
                size_t budget = maxLength > message.length()
                                    ? maxLength - message.length()
                                    : 0;
                if (insertion.length() > budget) {
                    insertion.resize(budget);
                }
                if (!insertion.empty()) {
                    message.insert(cursorPos, insertion);
                    cursorPos += insertion.length();
                }
                if (m_historyIndex != -1) {
                    m_pendingDraft = message;
                    m_historyIndex = -1;
                }
            } else {
                Screen::mouseClicked(x, y, buttonNum);
            }
        } else {
            Screen::mouseClicked(x, y, buttonNum);
        }
    }
}
