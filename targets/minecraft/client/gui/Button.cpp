#include "Button.h"

#include <GL/gl.h>
#include "platform/sdl2/Render.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/renderer/Textures.h"
#include "minecraft/client/resources/ResourceLocation.h"

class Minecraft;

#ifdef ENABLE_JAVA_GUIS
ResourceLocation GUI_GUI_LOCATION = ResourceLocation(TN_GUI_GUI);
#endif

Button::Button(int id, int x, int y, const std::wstring& msg) {
    init(id, x, y, 200, 20, msg);
}

Button::Button(int id, int x, int y, int w, int h, const std::wstring& msg) {
    init(id, x, y, w, h, msg);
}

void Button::init(int id, int x, int y, int w, int h, const std::wstring& msg) {
    active = true;
    visible = true;
    hovered = false; // 4J macOS - initialize hover state

    this->id = id;
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
    this->msg = msg;
}

int Button::getYImage(bool hovered) {
    int res = 1;
    if (!active)
        res = 0;
    else if (hovered)
        res = 2;
    return res;
}

void Button::render(Minecraft* minecraft, int xm, int ym) {
#ifdef ENABLE_JAVA_GUIS
    if (!visible) return;

    Font* font = minecraft->font;

    minecraft->textures->bindTexture(&GUI_GUI_LOCATION);
    glColor4f(1, 1, 1, 1);

    // 4J macOS - improved hover detection with bounds checking
    hovered = isMouseInBounds(xm, ym);
    int yImage = getYImage(hovered);

    // Draw button texture parts
    blit(x, y, 0, 46 + yImage * 20, w / 2, h);
    blit(x + w / 2, y, 200 - w / 2, 46 + yImage * 20, w / 2, h);

    renderBg(minecraft, xm, ym);

    // 4J macOS - improved text rendering with color based on state
    int textColor = 0xe0e0e0; // default color
    
    if (!active) {
        textColor = 0xffa0a0a0; // disabled color
    } else if (hovered) {
        textColor = 0xffffa0; // hover color (yellow-ish)
    }
    
    drawCenteredString(font, msg, x + w / 2, y + (h - 8) / 2, textColor);
#endif
}

void Button::renderBg(Minecraft* minecraft, int xm, int ym) {}

void Button::released(int mx, int my) {}

bool Button::clicked(Minecraft* minecraft, int mx, int my) {
    // 4J macOS - improved click detection using bounds checking method
    return isMouseInBounds(mx, my);
}