#pragma once

#include <string>
#include <unordered_map>



#define _DEBUG_BLOCK_CHARS 0


struct SFontData {
public:
    static const unsigned short FONTCOLS = 23;
    static const unsigned short FONTROWS = 20;

    static const unsigned short FONTSIZE = FONTCOLS * FONTROWS;

public:
    
    std::string m_strFontName;

    
    std::wstring m_wstrFilename;

    
    unsigned int m_uiGlyphCount;

    
    unsigned short* m_arrCodepoints;

    
    unsigned int m_uiGlyphMapX;

    
    unsigned int m_uiGlyphMapY;

    
    unsigned int m_uiGlyphMapCols;

    
    unsigned int m_uiGlyphMapRows;

    
    unsigned int m_uiGlyphWidth;

    
    unsigned int m_uiGlyphHeight;

    
    float m_fAscent;

    
    float m_fDescent;

    
    float m_fAdvPerPixel;

    
    unsigned int m_uiWhitespaceWidth;

public:
    static unsigned short Codepoints[FONTSIZE];
    static SFontData Mojangles_7;
    static SFontData Mojangles_11;
};


class CFontData {
public:
    CFontData();

    
    CFontData(SFontData& sFontData, int* pbRawImage);

    
    void release();

protected:
    
    SFontData* m_sFontData;

    
    std::unordered_map<unsigned int, unsigned short> m_unicodeMap;

    
    unsigned short* m_kerningTable;

    
    unsigned char* m_pbRawImage;

    
    float* m_pfAdvanceTable;

public:
    
    const std::string getFontName();

    
    SFontData* getFontData();

    
    unsigned short getGlyphId(unsigned int unicodepoint);

    
    unsigned int getUnicode(unsigned short glyphId);

    
    unsigned char* topLeftPixel(int row, int col);

    
    void getPos(unsigned short gyphId, int& row, int& col);

    
    float getAdvance(unsigned short glyphId);

    
    int getWidth(unsigned short glyphId);

    
    bool glyphIsWhitespace(unsigned short glyphId);

    
    bool unicodeIsWhitespace(unsigned int unicodepoint);

private:
    
    
    void moveCursor(unsigned char*& cursor, unsigned int dx, unsigned int dy);
};
