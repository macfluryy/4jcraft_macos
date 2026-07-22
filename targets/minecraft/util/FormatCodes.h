#pragma once

#include <cstdint>
#include <string>











std::wstring formatCodesToHtml(const std::wstring& escapedText,
                               uint32_t baseColorRgb,
                               const uint32_t palette[16]);
