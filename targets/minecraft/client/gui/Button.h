#pragma once
#include <string>
#include <memory>

#include "GuiComponent.h"

class Minecraft;

class Button : public GuiComponent {
protected:
    int w;
    int h;

public:
    int x, y;
    std::wstring msg;
    int id;
    bool active;
    bool visible;
    bool hovered; 

    Button(int id, int x, int y, const std::wstring& msg);
    Button(int id, int x, int y, int w, int h, const std::wstring& msg);
    
    void init(int id, int x, int y, int w, int h, const std::wstring& msg);
    
    
    int getWidth() const { return w; }
    int getHeight() const { return h; }
    
    bool isMouseInBounds(int mx, int my) const {
        return active && visible && 
               mx >= x && my >= y && 
               mx < x + w && my < y + h;
    }

protected:
    virtual int getYImage(bool hovered);

public:
    virtual void render(Minecraft* minecraft, int xm, int ym);

protected:
    virtual void renderBg(Minecraft* minecraft, int xm, int ym);

public:
    virtual void released(int mx, int my);
    virtual bool clicked(Minecraft* minecraft, int mx, int my);
};