#pragma once
#include <string>

static constexpr size_t kMaxChatJsonBytes = 256u * 1024u;
static constexpr int kMaxChatJsonDepth = 64;

std::wstring flattenChatComponent(const std::string& json);
std::wstring normalizeLegacyFormatting(const std::wstring& text);