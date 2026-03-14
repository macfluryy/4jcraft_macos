#pragma once
#include "GuiComponent.h"

class Font;
class Screen;

class EditBox : public GuiComponent {
private:
    Font* font;
    int x;
    int y;
    int width;
    int height;
    std::wstring value;
    unsigned int maxLength;
    int frame;

public:
    bool inFocus;
    bool active;

private:
    Screen* screen;

public:
    EditBox(Screen* screen, Font* font, int x, int y, int width, int height,
            const std::wstring& value);
    void setValue(const std::wstring& value);
    std::wstring getValue();
    void tick();
    void keyPressed(wchar_t ch, int eventKey);
    void mouseClicked(int mouseX, int mouseY, int buttonNum);
    void focus(bool newFocus);
    void render();
    void setMaxLength(int maxLength);
    int getMaxLength();
};