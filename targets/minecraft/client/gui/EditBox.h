#pragma once
#include <string>
#include <memory>

#include "GuiComponent.h"

class Screen;
class Font;

class EditBox : public GuiComponent {
private:
    Screen* screen;
    Font* font;
    
public:
    int x, y;
    int width, height;
    std::wstring value;
    int maxLength;
    bool active;
    bool inFocus; // 4J macOS - critical for proper focus handling
    int frame;
    bool enableBackgroundDrawing;
    
    // 4J macOS - add cursor position tracking
    size_t cursorPos;
    
    EditBox(Screen* screen, Font* font, int x, int y, int width, int height, 
            const std::wstring& value = L"");
    
    void setValue(const std::wstring& value);
    std::wstring getValue() const;
    
    void tick();
    void keyPressed(wchar_t ch, int eventKey);
    void mouseClicked(int mouseX, int mouseY, int buttonNum);
    void focus(bool newFocus);
    void render();
    
    void setMaxLength(int maxLength);
    int getMaxLength() const;
    void setEnableBackgroundDrawing(bool enable);
    
    bool isFocused() const { return inFocus; }
    bool isMouseInBounds(int mx, int my) const {
        return active && 
               mx >= x && mx < x + width && 
               my >= y && my < y + height;
    }
};