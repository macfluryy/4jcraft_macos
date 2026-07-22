#pragma once

#include <map>
#include <string>

class IntBuffer;
class Options;
class Textures;
class ResourceLocation;
class Random;

class Font {
private:
    int* charWidths;

public:
    int fontTexture;
    Random* random;

private:
    int colors[32];  

    Textures* textures;

    float xPos;
    float yPos;

    bool enforceUnicodeSheet;  
    bool bidirectional;        

    int m_cols;                           
    int m_rows;                           
    int m_charWidth;                      
    int m_charHeight;                     
    ResourceLocation* m_textureLocation;  
    std::map<int, int> m_charMap;

public:
    Font(Options* options, const std::wstring& name, Textures* textures,
         bool enforceUnicode, ResourceLocation* textureLocation, int cols,
         int rows, int charWidth, int charHeight,
         unsigned short charMap[] = nullptr);
    
    
    ~Font();
    void renderFakeCB(IntBuffer* cb);  

private:
    void renderCharacter(wchar_t c);  

public:
    void drawShadow(const std::wstring& str, int x, int y, int color);
    void drawShadowWordWrap(const std::wstring& str, int x, int y, int w,
                            int color, int h);  
    void draw(const std::wstring& str, int x, int y, int color);
    






private:
    std::wstring reorderBidi(const std::wstring& str);

    void draw(const std::wstring& str, bool dropShadow);
    void draw(const std::wstring& str, int x, int y, int color,
              bool dropShadow);
    int MapCharacter(wchar_t c);      
    bool CharacterExists(wchar_t c);  

public:
    int width(const std::wstring& str);
    std::wstring sanitize(const std::wstring& str);
    void drawWordWrap(const std::wstring& string, int x, int y, int w, int col,
                      int h);  

private:
    void drawWordWrapInternal(const std::wstring& string, int x, int y, int w,
                              int col, int h);  

public:
    void drawWordWrap(const std::wstring& string, int x, int y, int w, int col,
                      bool darken, int h);  

private:
    void drawWordWrapInternal(const std::wstring& string, int x, int y, int w,
                              int col, bool darken, int h);  

public:
    int wordWrapHeight(const std::wstring& string, int w);
    void setEnforceUnicodeSheet(bool enforceUnicodeSheet);
    void setBidirectional(bool bidirectional);

    
    bool AllCharactersValid(const std::wstring& str);
};
