#pragma once
#include <string>

class Font;

class GuiComponent {
protected:
    float blitOffset;

protected:
    // 4J macOS - improved line drawing
    void hLine(int x0, int x1, int y, int col);
    void vLine(int x, int y0, int y1, int col);
    
    // 4J macOS - improved fill methods
    void fill(int x0, int y0, int x1, int y1, int col);
    void fillGradient(int x0, int y0, int x1, int y1, int col1, int col2);

public:
    GuiComponent();
    
    // 4J macOS - text rendering with null checks
    void drawCenteredString(Font* font, const std::wstring& str, int x, int y,
                            int color);
    void drawString(Font* font, const std::wstring& str, int x, int y,
                    int color);
    
    // 4J macOS - improved blitting with safety checks
    void blit(int x, int y, int sx, int sy, int w, int h);
    
    // 4J macOS - utility getters
    float getBlitOffset() const { return blitOffset; }
    void setBlitOffset(float offset) { blitOffset = offset; }
};