#include "GuiComponent.h"

#include <GL/gl.h>
#include <math.h>
#include <algorithm>

#include "platform/sdl2/Render.h"
#include "app/include/stubs.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/gui/Font.h"
#include "minecraft/client/gui/Gui.h"
#include "minecraft/client/renderer/Tesselator.h"

void GuiComponent::hLine(int x0, int x1, int y, int col) {
    if (x1 < x0) {
        std::swap(x0, x1); // 4J macOS - use std::swap
    }
    fill(x0, y, x1 + 1, y + 1, col);
}

void GuiComponent::vLine(int x, int y0, int y1, int col) {
    if (y1 < y0) {
        std::swap(y0, y1); // 4J macOS - use std::swap
    }
    fill(x, y0 + 1, x + 1, y1, col);
}

void GuiComponent::fill(int x0, int y0, int x1, int y1, int col) {
    // 4J macOS - fix coordinate swapping logic
    if (x0 > x1) {
        std::swap(x0, x1);
    }
    if (y0 > y1) {
        std::swap(y0, y1);
    }
    
    float a = ((col >> 24) & 0xff) / 255.0f;
    float r = ((col >> 16) & 0xff) / 255.0f;
    float g = ((col >> 8) & 0xff) / 255.0f;
    float b = ((col) & 0xff) / 255.0f;
    
    Tesselator* t = Tesselator::getInstance();
    glEnable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(r, g, b, a);
    
    t->begin();
    t->vertex((float)(x0), (float)(y1), (float)(0));
    t->vertex((float)(x1), (float)(y1), (float)(0));
    t->vertex((float)(x1), (float)(y0), (float)(0));
    t->vertex((float)(x0), (float)(y0), (float)(0));
    t->end();
    
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

void GuiComponent::fillGradient(int x0, int y0, int x1, int y1, int col1,
                                int col2) {
    // 4J macOS - fix coordinate handling
    if (x0 > x1) std::swap(x0, x1);
    if (y0 > y1) std::swap(y0, y1);
    
    float a1 = ((col1 >> 24) & 0xff) / 255.0f;
    float r1 = ((col1 >> 16) & 0xff) / 255.0f;
    float g1 = ((col1 >> 8) & 0xff) / 255.0f;
    float b1 = ((col1) & 0xff) / 255.0f;

    float a2 = ((col2 >> 24) & 0xff) / 255.0f;
    float r2 = ((col2 >> 16) & 0xff) / 255.0f;
    float g2 = ((col2 >> 8) & 0xff) / 255.0f;
    float b2 = ((col2) & 0xff) / 255.0f;
    
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_SMOOTH);

    Tesselator* t = Tesselator::getInstance();
    t->begin();
    t->color(r1, g1, b1, a1);
    t->vertex((float)(x1), (float)(y0), blitOffset);
    t->vertex((float)(x0), (float)(y0), blitOffset);
    t->color(r2, g2, b2, a2);
    t->vertex((float)(x0), (float)(y1), blitOffset);
    t->vertex((float)(x1), (float)(y1), blitOffset);
    t->end();

    glShadeModel(GL_FLAT);
    glDisable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    glEnable(GL_TEXTURE_2D);
}

GuiComponent::GuiComponent() : blitOffset(0) { 
    // 4J macOS - initialize blitOffset
}

void GuiComponent::drawCenteredString(Font* font, const std::wstring& str,
                                      int x, int y, int color) {
    if (font == nullptr) return; // 4J macOS - null check
    font->drawShadow(str, x - (font->width(str)) / 2, y, color);
}

void GuiComponent::drawString(Font* font, const std::wstring& str, int x, int y,
                              int color) {
    if (font == nullptr) return; // 4J macOS - null check
    font->drawShadow(str, x, y, color);
}

void GuiComponent::blit(int x, int y, int sx, int sy, int w, int h) {
    if (w <= 0 || h <= 0) return; // 4J macOS - safety check
    
    float us = 1 / 256.0f;
    float vs = 1 / 256.0f;
    Tesselator* t = Tesselator::getInstance();
    t->begin();

    const float extraShift = 0.75f;

    float dx = (extraShift * (float)Minecraft::GetInstance()->width) /
               (float)Minecraft::GetInstance()->width_phys;
    dx /= Gui::currentGuiScaleFactor;
    float dy = extraShift / Gui::currentGuiScaleFactor;
    
    float fx = (floorf((float)x * Gui::currentGuiScaleFactor)) /
               Gui::currentGuiScaleFactor;
    float fy = (floorf((float)y * Gui::currentGuiScaleFactor)) /
               Gui::currentGuiScaleFactor;
    float fw = (floorf((float)w * Gui::currentGuiScaleFactor)) /
               Gui::currentGuiScaleFactor;
    float fh = (floorf((float)h * Gui::currentGuiScaleFactor)) /
               Gui::currentGuiScaleFactor;

    t->vertexUV(fx + 0 - dx, fy + fh - dy, (float)(blitOffset),
                (float)((sx + 0) * us), (float)((sy + h) * vs));
    t->vertexUV(fx + fw - dx, fy + fh - dy, (float)(blitOffset),
                (float)((sx + w) * us), (float)((sy + h) * vs));
    t->vertexUV(fx + fw - dx, fy + 0 - dy, (float)(blitOffset),
                (float)((sx + w) * us), (float)((sy + 0) * vs));
    t->vertexUV(fx + 0 - dx, fy + 0 - dy, (float)(blitOffset),
                (float)((sx + 0) * us), (float)((sy + 0) * vs));
    t->end();
}