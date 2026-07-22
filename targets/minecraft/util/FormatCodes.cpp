#include "minecraft/util/FormatCodes.h"

#include <cwchar>

namespace {

constexpr wchar_t kSection = L'§';

int fc_hexVal(wchar_t c) {
    if (c >= L'0' && c <= L'9') return c - L'0';
    if (c >= L'a' && c <= L'f') return (c - L'a') + 10;
    if (c >= L'A' && c <= L'F') return (c - L'A') + 10;
    return -1;
}

int fc_nearestPaletteIndex(uint32_t rgb, const uint32_t palette[16]) {
    const long r = (rgb >> 16) & 0xFF;
    const long g = (rgb >> 8) & 0xFF;
    const long b = rgb & 0xFF;
    int best = 0;
    long bestDist = -1;
    for (int i = 0; i < 16; ++i) {
        const long dr = r - ((palette[i] >> 16) & 0xFF);
        const long dg = g - ((palette[i] >> 8) & 0xFF);
        const long db = b - (palette[i] & 0xFF);
        const long dist = dr * dr + dg * dg + db * db;
        if (bestDist < 0 || dist < bestDist) {
            bestDist = dist;
            best = i;
        }
    }
    return best;
}

void fc_appendColorOpen(std::wstring& out, uint32_t rgb) {
    wchar_t buf[24];
    swprintf(buf, sizeof(buf) / sizeof(buf[0]), L"<font color=\"#%06X\">",
             rgb & 0xFFFFFF);
    out += buf;
}

}  

std::wstring formatCodesToHtml(const std::wstring& escapedText,
                               uint32_t baseColorRgb,
                               const uint32_t palette[16]) {
    std::wstring out;
    out.reserve(escapedText.size() + 32);

    bool italic = false;
    fc_appendColorOpen(out, baseColorRgb);

    auto switchColor = [&](uint32_t rgb) {
        
        if (italic) {
            out += L"</i>";
            italic = false;
        }
        out += L"</font>";
        fc_appendColorOpen(out, rgb);
    };

    for (size_t i = 0; i < escapedText.size(); ++i) {
        const wchar_t c = escapedText[i];
        if (c != kSection) {
            out.push_back(c);
            continue;
        }
        if (i + 1 >= escapedText.size()) break;  
        const wchar_t code = escapedText[i + 1];
        const int hex = fc_hexVal(code);
        if (hex >= 0) {
            switchColor(palette[hex]);
            ++i;
        } else if (code == L'x' || code == L'X') {
            
            int digits[6];
            int got = 0;
            size_t j = i + 2;
            while (got < 6 && j + 1 < escapedText.size() &&
                   escapedText[j] == kSection &&
                   fc_hexVal(escapedText[j + 1]) >= 0) {
                digits[got++] = fc_hexVal(escapedText[j + 1]);
                j += 2;
            }
            if (got == 6) {
                const uint32_t rgb =
                    (static_cast<uint32_t>(digits[0] * 16 + digits[1]) << 16) |
                    (static_cast<uint32_t>(digits[2] * 16 + digits[3]) << 8) |
                    static_cast<uint32_t>(digits[4] * 16 + digits[5]);
                switchColor(palette[fc_nearestPaletteIndex(rgb, palette)]);
                i = j - 1;  
            } else {
                ++i;  
            }
        } else if (code == L'r' || code == L'R') {
            switchColor(baseColorRgb);
            ++i;
        } else if (code == L'o' || code == L'O') {
            if (!italic) {
                out += L"<i>";
                italic = true;
            }
            ++i;
        } else {
            
            
            ++i;
        }
    }

    if (italic) out += L"</i>";
    out += L"</font>";
    return out;
}
