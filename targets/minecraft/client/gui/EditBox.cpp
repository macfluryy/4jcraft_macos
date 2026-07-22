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
    cursorPos = value.length(); 
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

    
    if (ch == 9) {
        screen->tabPressed();
        return; 
    }
    
    
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
    
    
    if (eventKey == Keyboard::KEY_DELETE) {
        if (cursorPos < value.length()) {
            value.erase(cursorPos, 1);
        }
        return;
    }
    
    
    if (eventKey == Keyboard::KEY_LEFT) {
        if (cursorPos > 0) cursorPos--;
        return;
    }
    
    if (eventKey == Keyboard::KEY_RIGHT) {
        if (cursorPos < value.length()) cursorPos++;
        return;
    }
    
    
    if (eventKey == Keyboard::KEY_HOME) {
        cursorPos = 0;
        return;
    }
    
    if (eventKey == Keyboard::KEY_END) {
        cursorPos = value.length();
        return;
    }
    
    
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
    
    
    if (newFocus) {
        
        if (active && enableBackgroundDrawing) {
            
            int textStartX = x + 4;
            if (mouseX > textStartX && font != nullptr) {
                
                size_t clickPos = (mouseX - textStartX) / 5; 
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
        
        frame = 0;
        cursorPos = value.length(); 
    }
    inFocus = newFocus;
}

void EditBox::render() {
    
    if (enableBackgroundDrawing) {
        fill(x - 1, y - 1, x + width + 1, y + height + 1, 0xffa0a0a0);
        fill(x, y, x + width, y + height, 0xff000000);
    }

    
    int textX = x;
    int textY = y;
    if (enableBackgroundDrawing) {
        textX += 4;
        textY += (height - 8) / 2;
    }

    if (active) {
        
        bool renderCursor = inFocus && (frame / 6 % 2 == 0);
        std::wstring displayValue = value;
        
        if (renderCursor) {
            
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