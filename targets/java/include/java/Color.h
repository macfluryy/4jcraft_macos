#pragma once

class Color {
private:
    int colour;

public:
    
    
    Color(float r, float g, float b);
    Color(int r, int g, int b);

    static Color getHSBColor(float h, float s, float b);
    int getRGB();
};