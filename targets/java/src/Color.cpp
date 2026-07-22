#include "java/Color.h"

#include <cassert>
#include <cmath>







Color::Color(float r, float g, float b) {
    assert(r >= 0.0f && r <= 1.0f);
    assert(g >= 0.0f && g <= 1.0f);
    assert(b >= 0.0f && b <= 1.0f);

    
    colour = ((0xFF << 24) | ((int)(r * 255) << 16) | ((int)(g * 255) << 8) |
              ((int)(b * 255)));
}

Color::Color(int r, int g, int b) {
    colour =
        ((0xFF << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | ((b & 0xff)));
}














Color Color::getHSBColor(float hue, float saturation, float brightness) {
    int r = 0, g = 0, b = 0;
    if (saturation == 0) {
        r = g = b = (int)(brightness * 255.0f + 0.5f);
    } else {
        float h = (hue - (float)std::floor(hue)) * 6.0f;
        float f = h - (float)std::floor(h);
        float p = brightness * (1.0f - saturation);
        float q = brightness * (1.0f - saturation * f);
        float t = brightness * (1.0f - (saturation * (1.0f - f)));
        switch ((int)h) {
            case 0:
                r = (int)(brightness * 255.0f + 0.5f);
                g = (int)(t * 255.0f + 0.5f);
                b = (int)(p * 255.0f + 0.5f);
                break;
            case 1:
                r = (int)(q * 255.0f + 0.5f);
                g = (int)(brightness * 255.0f + 0.5f);
                b = (int)(p * 255.0f + 0.5f);
                break;
            case 2:
                r = (int)(p * 255.0f + 0.5f);
                g = (int)(brightness * 255.0f + 0.5f);
                b = (int)(t * 255.0f + 0.5f);
                break;
            case 3:
                r = (int)(p * 255.0f + 0.5f);
                g = (int)(q * 255.0f + 0.5f);
                b = (int)(brightness * 255.0f + 0.5f);
                break;
            case 4:
                r = (int)(t * 255.0f + 0.5f);
                g = (int)(p * 255.0f + 0.5f);
                b = (int)(brightness * 255.0f + 0.5f);
                break;
            case 5:
                r = (int)(brightness * 255.0f + 0.5f);
                g = (int)(p * 255.0f + 0.5f);
                b = (int)(q * 255.0f + 0.5f);
                break;
        }
    }

    return Color(r, g, b);
}

int Color::getRGB() { return colour; }