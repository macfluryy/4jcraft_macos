#pragma once

#include <string>

#include "app/mac/Iggy/include/iggy.h"
#ifndef _ENABLEIGGY
#include "app/mac/Stubs/iggy_stubs.h"
#endif
#include "app/mac/Iggy/include/rrCore.h"

struct SFontData;
class CFontData;

#define VERBOSE_FONT_OUTPUT 0












class UIAbstractBitmapFont {
protected:
    std::string m_fontname;

    IggyBitmapFontProvider* m_bitmapFontProvider;

    bool m_registered;

    unsigned int m_numGlyphs;

public:
    UIAbstractBitmapFont(const std::string& fontname);
    ~UIAbstractBitmapFont();

    void registerFont();

    
    virtual IggyFontMetrics* GetFontMetrics(IggyFontMetrics* metrics) = 0;
    virtual S32 GetCodepointGlyph(U32 codepoint) = 0;
    virtual IggyGlyphMetrics* GetGlyphMetrics(S32 glyph,
                                              IggyGlyphMetrics* metrics) = 0;
    virtual rrbool IsGlyphEmpty(S32 glyph) = 0;
    virtual F32 GetKerningForGlyphPair(S32 first_glyph, S32 second_glyph) = 0;
    virtual rrbool CanProvideBitmap(S32 glyph, F32 pixel_scale) = 0;
    virtual rrbool GetGlyphBitmap(S32 glyph, F32 pixel_scale,
                                  IggyBitmapCharacter* bitmap) = 0;
    virtual void FreeGlyphBitmap(S32 glyph, F32 pixel_scale,
                                 IggyBitmapCharacter* bitmap) = 0;

    
    
    static IggyFontMetrics* RADLINK
    GetFontMetrics_Callback(void* user_context, IggyFontMetrics* metrics);
    static S32 RADLINK GetCodepointGlyph_Callback(void* user_context,
                                                  U32 codepoint);
    static IggyGlyphMetrics* RADLINK GetGlyphMetrics_Callback(
        void* user_context, S32 glyph, IggyGlyphMetrics* metrics);
    static rrbool RADLINK IsGlyphEmpty_Callback(void* user_context, S32 glyph);
    static F32 RADLINK GetKerningForGlyphPair_Callback(void* user_context,
                                                       S32 first_glyph,
                                                       S32 second_glyph);
    static rrbool RADLINK CanProvideBitmap_Callback(void* user_context,
                                                    S32 glyph, F32 pixel_scale);
    static rrbool RADLINK GetGlyphBitmap_Callback(void* user_context, S32 glyph,
                                                  F32 pixel_scale,
                                                  IggyBitmapCharacter* bitmap);
    static void RADLINK FreeGlyphBitmap_Callback(void* user_context, S32 glyph,
                                                 F32 pixel_scale,
                                                 IggyBitmapCharacter* bitmap);
};

class UIBitmapFont : public UIAbstractBitmapFont {
protected:
    CFontData* m_cFontData;

public:
    UIBitmapFont(SFontData& sfontdata);

    ~UIBitmapFont();

    virtual IggyFontMetrics* GetFontMetrics(IggyFontMetrics* metrics);
    virtual S32 GetCodepointGlyph(U32 codepoint);
    virtual IggyGlyphMetrics* GetGlyphMetrics(S32 glyph,
                                              IggyGlyphMetrics* metrics);
    virtual rrbool IsGlyphEmpty(S32 glyph);
    virtual F32 GetKerningForGlyphPair(S32 first_glyph, S32 second_glyph);
    virtual rrbool CanProvideBitmap(S32 glyph, F32 pixel_scale);
    virtual rrbool GetGlyphBitmap(S32 glyph, F32 pixel_scale,
                                  IggyBitmapCharacter* bitmap);
    virtual void FreeGlyphBitmap(S32 glyph, F32 pixel_scale,
                                 IggyBitmapCharacter* bitmap);
};