// 4J macOS - thin wrapper around SDL_SetClipboardText so the rest of the
// codebase doesn't need to pull in SDL2 headers just to copy a chat
// line. We accept std::wstring at the call site (the engine speaks
// wide strings everywhere) and convert to UTF-8 here.
#pragma once

#include <string>

namespace Clipboard {

// Copy the given text to the system clipboard. Returns true on success.
// Wide chars outside the BMP are encoded as surrogate-free 4-byte UTF-8
// sequences. On failure we print to stderr and return false.
bool SetText(const std::wstring& text);

}  // namespace Clipboard
