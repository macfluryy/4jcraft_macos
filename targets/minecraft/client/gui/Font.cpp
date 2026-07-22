#include "Font.h"

#include <string.h>

#include <utility>
#include <vector>

#include "platform/sdl2/Render.h"
#include "app/include/BufferedImage.h"
#include "util/StringHelpers.h"
#include "java/Random.h"
#include "minecraft/SharedConstants.h"
#include "minecraft/client/Options.h"
#include "minecraft/client/renderer/Tesselator.h"
#include "minecraft/client/renderer/Textures.h"
#include "minecraft/client/resources/ResourceLocation.h"

Font::Font(Options* options, const std::wstring& name, Textures* textures,
           bool enforceUnicode, ResourceLocation* textureLocation, int cols,
           int rows, int charWidth, int charHeight,
           unsigned short charMap[] )
    : textures(textures) {
    int charC = cols * rows;  

    charWidths = new int[charC];

    
    memset(charWidths, 0, charC);

    enforceUnicodeSheet = false;
    bidirectional = false;
    xPos = yPos = 0.0f;

    
    m_cols = cols;
    m_rows = rows;
    m_charWidth = charWidth;
    m_charHeight = charHeight;
    m_textureLocation = textureLocation;

    
    if (charMap != nullptr) {
        for (int i = 0; i < charC; i++) {
            m_charMap.insert(std::make_pair(charMap[i], i));
        }
    }

    random = new Random();

    
    BufferedImage* img =
        textures->readImage(textureLocation->getTexture(), name);

    







    int w = img->getWidth();
    int h = img->getHeight();
    std::vector<int> rawPixels(w * h);
    img->getRGB(0, 0, w, h, rawPixels, 0, w);

    for (int i = 0; i < charC; i++) {
        int xt = i % m_cols;
        int yt = i / m_cols;

        int x = 7;
        for (; x >= 0; x--) {
            int xPixel = xt * 8 + x;
            bool emptyColumn = true;
            for (int y = 0; y < 8 && emptyColumn; y++) {
                int yPixel = (yt * 8 + y) * w;
                bool emptyPixel = (rawPixels[xPixel + yPixel] >> 24) ==
                                  0;  
                if (!emptyPixel) emptyColumn = false;
            }
            if (!emptyColumn) {
                break;
            }
        }

        if (i == ' ') x = 4 - 2;
        charWidths[i] = x + 2;
    }

    delete img;

    
    for (int colorN = 0; colorN < 32; ++colorN) {
        int var10 = (colorN >> 3 & 1) * 85;
        int red = (colorN >> 2 & 1) * 170 + var10;
        int green = (colorN >> 1 & 1) * 170 + var10;
        int blue = (colorN >> 0 & 1) * 170 + var10;

        if (colorN == 6) {
            red += 85;
        }

        if (options->anaglyph3d) {
            int tmpRed = (red * 30 + green * 59 + blue * 11) / 100;
            int tmpGreen = (red * 30 + green * 70) / 100;
            int tmpBlue = (red * 30 + blue * 70) / 100;
            red = tmpRed;
            green = tmpGreen;
            blue = tmpBlue;
        }

        if (colorN >= 16) {
            red /= 4;
            green /= 4;
            blue /= 4;
        }

        colors[colorN] = (red & 255) << 16 | (green & 255) << 8 | (blue & 255);
    }
}



Font::~Font() { delete[] charWidths; }

void Font::renderCharacter(wchar_t c) {
    float xOff = c % m_cols * m_charWidth;
    float yOff = c / m_cols * m_charWidth;

    float width = charWidths[c] - .01f;
    float height = m_charHeight - .01f;

    float fontWidth = m_cols * m_charWidth;
    float fontHeight = m_rows * m_charHeight;

    Tesselator* t = Tesselator::getInstance();
    
    t->begin();
    t->tex(xOff / fontWidth, (yOff + 7.99f) / fontHeight);
    t->vertex(xPos, yPos + height, 0.0f);

    t->tex((xOff + width) / fontWidth, (yOff + 7.99f) / fontHeight);
    t->vertex(xPos + width, yPos + height, 0.0f);

    t->tex((xOff + width) / fontWidth, yOff / fontHeight);
    t->vertex(xPos + width, yPos, 0.0f);

    t->tex(xOff / fontWidth, yOff / fontHeight);
    t->vertex(xPos, yPos, 0.0f);

    t->end();

    xPos += (float)charWidths[c];
}

void Font::drawShadow(const std::wstring& str, int x, int y, int color) {
    draw(str, x + 1, y + 1, color, true);
    draw(str, x, y, color, false);
}

void Font::drawShadowWordWrap(const std::wstring& str, int x, int y, int w,
                              int color, int h) {
    drawWordWrapInternal(str, x + 1, y + 1, w, color, true, h);
    drawWordWrapInternal(str, x, y, w, color, h);
}

void Font::draw(const std::wstring& str, int x, int y, int color) {
    draw(str, x, y, color, false);
}

std::wstring Font::reorderBidi(const std::wstring& str) {
    
    return str;
}

void Font::draw(const std::wstring& str, bool dropShadow) {
    
    textures->bindTexture(m_textureLocation);

    bool noise = false;
    std::wstring cleanStr = sanitize(str);

    for (int i = 0; i < (int)cleanStr.length(); ++i) {
        
        wchar_t c = cleanStr.at(i);

        if (c == 167 && i + 1 < cleanStr.length()) {
            
            
            
            wchar_t ca = cleanStr[i + 1];
            int colorN = 16;
            if ((ca >= L'0') && (ca <= L'9'))
                colorN = ca - L'0';
            else if ((ca >= L'a') && (ca <= L'f'))
                colorN = (ca - L'a') + 10;
            else if ((ca >= L'A') && (ca <= L'F'))
                colorN = (ca - L'A') + 10;

            if (colorN == 16) {
                noise = true;
            } else {
                noise = false;
                if (colorN < 0 || colorN > 15) colorN = 15;

                if (dropShadow) colorN += 16;

                int color = colors[colorN];
                glColor3f((color >> 16) / 255.0F, ((color >> 8) & 255) / 255.0F,
                          (color & 255) / 255.0F);
            }

            i += 1;
            continue;
        }

        
        if (noise) {
            int newc;
            do {
                newc = random->nextInt(
                    SharedConstants::acceptableLetters.length());
            } while (charWidths[c + 32] != charWidths[newc + 32]);
            c = newc;
        }

        renderCharacter(c);
    }
}

void Font::draw(const std::wstring& str, int x, int y, int color,
                bool dropShadow) {
    if (!str.empty()) {
        if ((color & 0xFC000000) == 0) color |= 0xFF000000;  
        

        if (dropShadow)  
                         
                         
            color = (color & 0xfcfcfc) >> 2 | (color & (0xFFFFFFFF << 24));

        glColor4f((color >> 16 & 255) / 255.0F, (color >> 8 & 255) / 255.0F,
                  (color & 255) / 255.0F, (color >> 24 & 255) / 255.0F);

        xPos = x;
        yPos = y;
        draw(str, dropShadow);
    }
}

int Font::width(const std::wstring& str) {
    std::wstring cleanStr = sanitize(str);

    if (cleanStr == L"") return 0;  
    int len = 0;

    for (int i = 0; i < cleanStr.length(); ++i) {
        wchar_t c = cleanStr.at(i);

        if (c == 167) {
            
            ++i;
        } else {
            len += charWidths[c];
        }
    }

    return len;
}

std::wstring Font::sanitize(const std::wstring& str) {
    std::wstring sb = str;

    for (unsigned int i = 0; i < sb.length(); i++) {
        if (CharacterExists(sb[i])) {
            sb[i] = MapCharacter(sb[i]);
        } else {
            
            
            sb[i] = 0;
        }
    }
    return sb;
}

int Font::MapCharacter(wchar_t c) {
    if (!m_charMap.empty()) {
        
        return c == ' ' ? c : m_charMap[c];
    } else {
        return c;
    }
}

bool Font::CharacterExists(wchar_t c) {
    if (!m_charMap.empty()) {
        return m_charMap.find(c) != m_charMap.end();
    } else {
        return c >= 0 && c <= m_rows * m_cols;
    }
}

void Font::drawWordWrap(const std::wstring& string, int x, int y, int w,
                        int col, int h) {
    
    
    
    
    drawWordWrapInternal(string, x, y, w, col, h);
}

void Font::drawWordWrapInternal(const std::wstring& string, int x, int y, int w,
                                int col, int h) {
    drawWordWrapInternal(string, x, y, w, col, false, h);
}

void Font::drawWordWrap(const std::wstring& string, int x, int y, int w,
                        int col, bool darken, int h) {
    
    
    
    
    drawWordWrapInternal(string, x, y, w, col, darken, h);
}

void Font::drawWordWrapInternal(const std::wstring& string, int x, int y, int w,
                                int col, bool darken, int h) {
    std::vector<std::wstring> lines = stringSplit(string, L'\n');
    if (lines.size() > 1) {
        auto itEnd = lines.end();
        for (auto it = lines.begin(); it != itEnd; it++) {
            
            
            if ((y + this->wordWrapHeight(*it, w)) > h) break;
            drawWordWrapInternal(*it, x, y, w, col, h);
            y += this->wordWrapHeight(*it, w);
        }
        return;
    }
    std::vector<std::wstring> words = stringSplit(string, L' ');
    unsigned int pos = 0;
    while (pos < words.size()) {
        std::wstring line = words[pos++] + L" ";
        while (pos < words.size() && width(line + words[pos]) < w) {
            line += words[pos++] + L" ";
        }
        while (width(line) > w) {
            int l = 0;
            while (width(line.substr(0, l + 1)) <= w) {
                l++;
            }
            if (trimString(line.substr(0, l)).length() > 0) {
                draw(line.substr(0, l), x, y, col);
                y += 8;
            }
            line = line.substr(l);

            
            
            if ((y + 8) > h) break;
        }
        
        
        if (trimString(line).length() > 0 && !((y + 8) > h)) {
            draw(line, x, y, col);
            y += 8;
        }
    }
}

int Font::wordWrapHeight(const std::wstring& string, int w) {
    std::vector<std::wstring> lines = stringSplit(string, L'\n');
    if (lines.size() > 1) {
        int h = 0;
        auto itEnd = lines.end();
        for (auto it = lines.begin(); it != itEnd; it++) {
            h += this->wordWrapHeight(*it, w);
        }
        return h;
    }
    std::vector<std::wstring> words = stringSplit(string, L' ');
    unsigned int pos = 0;
    int y = 0;
    while (pos < words.size()) {
        std::wstring line = words[pos++] + L" ";
        while (pos < words.size() && width(line + words[pos]) < w) {
            line += words[pos++] + L" ";
        }
        while (width(line) > w) {
            int l = 0;
            while (width(line.substr(0, l + 1)) <= w) {
                l++;
            }
            if (trimString(line.substr(0, l)).length() > 0) {
                y += 8;
            }
            line = line.substr(l);
        }
        if (trimString(line).length() > 0) {
            y += 8;
        }
    }
    if (y < 8) y += 8;
    return y;
}

void Font::setEnforceUnicodeSheet(bool enforceUnicodeSheet) {
    this->enforceUnicodeSheet = enforceUnicodeSheet;
}

void Font::setBidirectional(bool bidirectional) {
    this->bidirectional = bidirectional;
}

bool Font::AllCharactersValid(const std::wstring& str) {
    for (int i = 0; i < (int)str.length(); ++i) {
        wchar_t c = str.at(i);

        if (c == 167 && i + 1 < str.length()) {
            
            i += 1;
            continue;
        }

        int index = SharedConstants::acceptableLetters.find(c);

        if ((c != ' ') && !(index > 0 && !enforceUnicodeSheet)) {
            return false;
        }
    }
    return true;
}



































































































































