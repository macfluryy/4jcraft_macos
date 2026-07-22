#include "UIBitmapFont.h"

#include "app/mac/Iggy/include/iggy.h"
#ifndef _ENABLEIGGY
#include "app/mac/Stubs/iggy_stubs.h"
#endif
#include "app/mac/Iggy/include/rrCore.h"
#include "app/include/BufferedImage.h"
#include "UIFontData.h"





UIAbstractBitmapFont::~UIAbstractBitmapFont() {
    if (m_registered)
        IggyFontRemoveUTF8(m_fontname.c_str(), -1, IGGY_FONTFLAG_none);
    delete m_bitmapFontProvider;
}

UIAbstractBitmapFont::UIAbstractBitmapFont(const std::string& fontname) {
    m_fontname = fontname;

    m_registered = false;

    m_bitmapFontProvider = new IggyBitmapFontProvider();
    m_bitmapFontProvider->get_font_metrics =
        &UIAbstractBitmapFont::GetFontMetrics_Callback;
    m_bitmapFontProvider->get_glyph_for_codepoint =
        &UIAbstractBitmapFont::GetCodepointGlyph_Callback;
    m_bitmapFontProvider->get_glyph_metrics =
        &UIAbstractBitmapFont::GetGlyphMetrics_Callback;
    m_bitmapFontProvider->is_empty =
        &UIAbstractBitmapFont::IsGlyphEmpty_Callback;
    m_bitmapFontProvider->get_kerning =
        &UIAbstractBitmapFont::GetKerningForGlyphPair_Callback;
    m_bitmapFontProvider->can_bitmap =
        &UIAbstractBitmapFont::CanProvideBitmap_Callback;
    m_bitmapFontProvider->get_bitmap =
        &UIAbstractBitmapFont::GetGlyphBitmap_Callback;
    m_bitmapFontProvider->free_bitmap =
        &UIAbstractBitmapFont::FreeGlyphBitmap_Callback;
    m_bitmapFontProvider->userdata = this;
}

void UIAbstractBitmapFont::registerFont() {
    if (!m_registered) {
        
        
        m_bitmapFontProvider->num_glyphs = m_numGlyphs;
        IggyFontInstallBitmapUTF8(m_bitmapFontProvider, m_fontname.c_str(), -1,
                                  IGGY_FONTFLAG_none);
        m_registered = true;
    }

    
    
    IggyFontSetIndirectUTF8(m_fontname.c_str(), -1, IGGY_FONTFLAG_all,
                            m_fontname.c_str(), -1, IGGY_FONTFLAG_none);
}

IggyFontMetrics* RADLINK UIAbstractBitmapFont::GetFontMetrics_Callback(
    void* user_context, IggyFontMetrics* metrics) {
    return ((UIAbstractBitmapFont*)user_context)->GetFontMetrics(metrics);
}

S32 RADLINK UIAbstractBitmapFont::GetCodepointGlyph_Callback(void* user_context,
                                                             U32 codepoint) {
    return ((UIAbstractBitmapFont*)user_context)->GetCodepointGlyph(codepoint);
}

IggyGlyphMetrics* RADLINK UIAbstractBitmapFont::GetGlyphMetrics_Callback(
    void* user_context, S32 glyph, IggyGlyphMetrics* metrics) {
    return ((UIAbstractBitmapFont*)user_context)
        ->GetGlyphMetrics(glyph, metrics);
}

rrbool RADLINK UIAbstractBitmapFont::IsGlyphEmpty_Callback(void* user_context,
                                                           S32 glyph) {
    return ((UIAbstractBitmapFont*)user_context)->IsGlyphEmpty(glyph);
}

F32 RADLINK UIAbstractBitmapFont::GetKerningForGlyphPair_Callback(
    void* user_context, S32 first_glyph, S32 second_glyph) {
    return ((UIAbstractBitmapFont*)user_context)
        ->GetKerningForGlyphPair(first_glyph, second_glyph);
}

rrbool RADLINK UIAbstractBitmapFont::CanProvideBitmap_Callback(
    void* user_context, S32 glyph, F32 pixel_scale) {
    return ((UIAbstractBitmapFont*)user_context)
        ->CanProvideBitmap(glyph, pixel_scale);
}

rrbool RADLINK UIAbstractBitmapFont::GetGlyphBitmap_Callback(
    void* user_context, S32 glyph, F32 pixel_scale,
    IggyBitmapCharacter* bitmap) {
    return ((UIAbstractBitmapFont*)user_context)
        ->GetGlyphBitmap(glyph, pixel_scale, bitmap);
}

void RADLINK UIAbstractBitmapFont::FreeGlyphBitmap_Callback(
    void* user_context, S32 glyph, F32 pixel_scale,
    IggyBitmapCharacter* bitmap) {
    return ((UIAbstractBitmapFont*)user_context)
        ->FreeGlyphBitmap(glyph, pixel_scale, bitmap);
}

UIBitmapFont::UIBitmapFont(SFontData& sfontdata)
    : UIAbstractBitmapFont(sfontdata.m_strFontName) {
    m_numGlyphs = sfontdata.m_uiGlyphCount;

    BufferedImage bimg(sfontdata.m_wstrFilename);
    int* bimgData = bimg.getData();

    m_cFontData = new CFontData(sfontdata, bimgData);

    
}

UIBitmapFont::~UIBitmapFont() { m_cFontData->release(); }


IggyFontMetrics* UIBitmapFont::GetFontMetrics(IggyFontMetrics* metrics) {
    
    
    
    
    
    
    
    
    

    metrics->ascent = m_cFontData->getFontData()->m_fAscent;
    metrics->descent = m_cFontData->getFontData()->m_fDescent;

    metrics->average_glyph_width_for_tab_stops = 8.0f;

    
    
    metrics->largest_glyph_bbox_y1 = metrics->descent;

    

    return metrics;
}



S32 UIBitmapFont::GetCodepointGlyph(U32 codepoint) {
    
    if (codepoint == 0x2019) codepoint = 0x27;

    return m_cFontData->getGlyphId(codepoint);
}


IggyGlyphMetrics* UIBitmapFont::GetGlyphMetrics(S32 glyph,
                                                IggyGlyphMetrics* metrics) {
    
    
    
    

    
    
    
    
    
    

    













    float glyphAdvance = m_cFontData->getAdvance(glyph);

    
    
    metrics->x0 = 0.0f;

    if (m_cFontData->glyphIsWhitespace(glyph))
        metrics->x1 = 0.0f;
    else
        metrics->x1 = glyphAdvance;

    
    metrics->advance = glyphAdvance;

    
    

    
    metrics->y0 = 0.0f;
    metrics->y1 = 1.0f;

    return metrics;
}



rrbool UIBitmapFont::IsGlyphEmpty(S32 glyph) {
    if (m_cFontData->glyphIsWhitespace(glyph)) return true;
    return false;  
                   
}



F32 UIBitmapFont::GetKerningForGlyphPair(S32 first_glyph, S32 second_glyph) {
    
    
    

    
    
    return 0.0f;
}



rrbool UIBitmapFont::CanProvideBitmap(S32 glyph, F32 pixel_scale) {
    
    
    return true;
}








rrbool UIBitmapFont::GetGlyphBitmap(S32 glyph, F32 pixel_scale,
                                    IggyBitmapCharacter* bitmap) {
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    

    int row = 0, col = 0;
    m_cFontData->getPos(glyph, row, col);

    
    bitmap->pixels_one_per_byte = m_cFontData->topLeftPixel(row, col);

    
    float glyphScale = 1.0f,
          truePixelScale = 1.0f / m_cFontData->getFontData()->m_fAdvPerPixel;
    F32 targetPixelScale = pixel_scale;
    
    
    
    
    
    while ((0.5f + glyphScale) * truePixelScale < targetPixelScale)
        glyphScale++;

    
#if (!defined _CONTENT_PACKAGE) && (VERBOSE_FONT_OUTPUT > 0)

    struct DebugData {
        std::string name;
        long scale;
        long mul;

        bool operator==(const DebugData& dd) const {
            if (name.compare(dd.name) != 0)
                return false;
            else if (scale != dd.scale)
                return false;
            else if (mul != dd.mul)
                return false;
            else
                return true;
        }
    };

    static long long lastPrint = System::currentTimeMillis();
    static std::unordered_set<DebugData> debug_fontSizesRequested;

    {
        DebugData dData = {m_cFontData->getFontName(), (long)pixel_scale,
                           (long)glyphScale};
        debug_fontSizesRequested.insert(dData);

        if ((lastPrint - System::currentTimeMillis()) > VERBOSE_FONT_OUTPUT) {
            app.DebugPrintf("<UIBitmapFont> Requested font/sizes:\n");

            std::unordered_set<DebugData>::iterator itr;
            for (itr = debug_fontSizesRequested.begin();
                 itr != debug_fontSizesRequested.end(); itr++) {
                app.DebugPrintf("<UIBitmapFont>\t- %s:%i\t(x%i)\n",
                                itr->name.c_str(), itr->scale, itr->mul);
            }

            lastPrint = System::currentTimeMillis();
            debug_fontSizesRequested.clear();
        }
    }
#endif

    
    
    
    

    
    
    
    bitmap->width_in_pixels = m_cFontData->getFontData()->m_uiGlyphWidth;
    bitmap->height_in_pixels = m_cFontData->getFontData()->m_uiGlyphHeight;

    




    bitmap->top_left_x = 0;

    
    
    
    
    bitmap->top_left_y = -((S32)m_cFontData->getFontData()->m_uiGlyphHeight) *
                         m_cFontData->getFontData()->m_fAscent;

    bitmap->oversample = 0;
    bitmap->point_sample = true;

    
    
    
    
    
    
    
    

    
    bitmap->pixel_scale_correct = pixel_scale / glyphScale;
    bitmap->pixel_scale_max = 99.0f;
    bitmap->pixel_scale_min = 0.0f;

    







    
    
    bitmap->stride_in_bytes = m_cFontData->getFontData()->m_uiGlyphMapX;

    
    bitmap->user_context_for_free = nullptr;

    return true;
}


void UIBitmapFont::FreeGlyphBitmap(S32 glyph, F32 pixel_scale,
                                   IggyBitmapCharacter* bitmap) {
    
    
    
}
