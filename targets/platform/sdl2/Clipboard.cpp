#include "Clipboard.h"

#include <SDL2/SDL_clipboard.h>
#include <SDL2/SDL_error.h>

#include <cstdint>
#include <cstdio>
#include <string>

namespace Clipboard {

namespace {

// Encode one Unicode code point into a UTF-8 byte sequence (1-4 bytes).
// We assume the platform wchar_t is 32-bit (true on macOS / Linux); if
// the input contains an unpaired surrogate we just emit the replacement
// character so the clipboard never receives invalid UTF-8.
void appendUtf8(std::string& out, uint32_t cp) {
    if (cp >= 0xD800 && cp <= 0xDFFF) {
        cp = 0xFFFD;  // surrogate -> replacement
    }
    if (cp <= 0x7F) {
        out.push_back(static_cast<char>(cp));
    } else if (cp <= 0x7FF) {
        out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF) {
        out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0x10FFFF) {
        out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        // Out of Unicode range; replace.
        appendUtf8(out, 0xFFFD);
    }
}

}  // namespace

bool SetText(const std::wstring& text) {
    std::string utf8;
    utf8.reserve(text.size() * 2);  // rough upper bound for ASCII-heavy input
    for (wchar_t wc : text) {
        appendUtf8(utf8, static_cast<uint32_t>(wc));
    }
    if (SDL_SetClipboardText(utf8.c_str()) != 0) {
        std::fprintf(stderr, "[Clipboard] SDL_SetClipboardText failed: %s\n",
                     SDL_GetError());
        return false;
    }
    return true;
}

}  // namespace Clipboard
