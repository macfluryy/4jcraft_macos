#include "EditBox.h"

#include "app/include/stubs.h"
#include "minecraft/SharedConstants.h"
#include "minecraft/client/gui/Screen.h"

EditBox::EditBox(Screen* screen, Font* font, int x, int y, int width,
                 int height, const std::wstring& value) 
    : screen(screen), font(font), x(x), y(y), width(width), height(height),
      maxLength(0), active(true), inFocus(false), frame(0), 
      enableBackgroundDrawing(true), cursorPos(0) {
    setValue(value);
}

void EditBox::setValue(const std::wstring& value) { 
    this->value = value;
    cursorPos = value.length(); // 4J macOS - move cursor to end
}

std::wstring EditBox::getValue() const { 
    return value; 
}

void EditBox::tick() { 
    frame++;
}

void EditBox::keyPressed(wchar_t ch, int eventKey) {
    if (!active || !inFocus) {
        return;
    }

    // 4J macOS - improved tab handling
    if (ch == 9) {
        screen->tabPressed();
        return; // don't add tab character
    }
    
    // 4J macOS - handle backspace correctly
    if (eventKey == Keyboard::KEY_BACK) {
        if (cursorPos > 0 && value.length() > 0) {
            if (cursorPos > value.length()) {
                cursorPos = value.length();
            }
            value.erase(cursorPos - 1, 1);
            cursorPos--;
        }
        return;
    }
    
    // 4J macOS - handle delete key
    if (eventKey == Keyboard::KEY_DELETE) {
        if (cursorPos < value.length()) {
            value.erase(cursorPos, 1);
        }
        return;
    }
    
    // 4J macOS - handle arrow keys for cursor movement
    if (eventKey == Keyboard::KEY_LEFT) {
        if (cursorPos > 0) cursorPos--;
        return;
    }
    
    if (eventKey == Keyboard::KEY_RIGHT) {
        if (cursorPos < value.length()) cursorPos++;
        return;
    }
    
    // 4J macOS - handle home/end keys
    if (eventKey == Keyboard::KEY_HOME) {
        cursorPos = 0;
        return;
    }
    
    if (eventKey == Keyboard::KEY_END) {
        cursorPos = value.length();
        return;
    }
    
    // 4J macOS - improved character validation
    if (SharedConstants::acceptableLetters.find(ch) != std::wstring::npos) {
        if (maxLength == 0 || value.length() < (size_t)maxLength) {
            if (cursorPos >= value.length()) {
                value += ch;
            } else {
                value.insert(cursorPos, 1, ch);
            }
            cursorPos++;
        }
    }
}

void EditBox::mouseClicked(int mouseX, int mouseY, int buttonNum) {
    bool newFocus = isMouseInBounds(mouseX, mouseY);
    
    // 4J macOS - improved focus handling
    if (newFocus) {
        // Calculate cursor position based on click
        if (active && enableBackgroundDrawing) {
            // Rough cursor position calculation
            int textStartX = x + 4;
            if (mouseX > textStartX && font != nullptr) {
                // Calculate approximate cursor position
                size_t clickPos = (mouseX - textStartX) / 5; // rough estimate
                if (clickPos <= value.length()) {
                    cursorPos = clickPos;
                }
            }
        }
    }
    
    focus(newFocus);
}

void EditBox::focus(bool newFocus) {
    if (newFocus && !inFocus) {
        // reset the underscore counter to give quicker selection feedback
        frame = 0;
        cursorPos = value.length(); // move cursor to end on focus
    }
    inFocus = newFocus;
}

void EditBox::render() {
    // 4J macOS - improved background rendering
    if (enableBackgroundDrawing) {
        fill(x - 1, y - 1, x + width + 1, y + height + 1, 0xffa0a0a0);
        fill(x, y, x + width, y + height, 0xff000000);
    }

    // 4J macOS - conditional text offset
    int textX = x;
    int textY = y;
    if (enableBackgroundDrawing) {
        textX += 4;
        textY += (height - 8) / 2;
    }

    if (active) {
        // 4J macOS - improved cursor rendering
        bool renderCursor = inFocus && (frame / 6 % 2 == 0);
        std::wstring displayValue = value;
        
        if (renderCursor) {
            // Insert cursor at cursor position
            if (cursorPos < displayValue.length()) {
                displayValue.insert(cursorPos, L"|");
            } else {
                displayValue += L"_";
            }
        }
        
        drawString(font, displayValue, textX, textY,
                   (enableBackgroundDrawing ? 0xe0e0e0 : 0xffffff));
    } else {
        drawString(font, value, textX, textY,
                   (enableBackgroundDrawing ? 0xe0e0e0 : 0xffffff));
    }
}

void EditBox::setMaxLength(int maxLength) { 
    this->maxLength = maxLength; 
}

int EditBox::getMaxLength() const { 
    return maxLength; 
}

void EditBox::setEnableBackgroundDrawing(bool enable) {
    enableBackgroundDrawing = enable;
}