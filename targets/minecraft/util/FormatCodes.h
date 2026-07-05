#pragma once

#include <cstdint>
#include <string>

// Translates legacy Minecraft "§" formatting codes embedded in (already
// XML-escaped) text into the Flash-HTML subset the UI label renders:
//   §0-§9/§a-§f -> <font color="#RRGGBB"> using the supplied palette
//   §x§R§R§G§G§B§B (modern RGB) -> nearest palette colour
//   §r -> close all formatting, restore the base colour
//   §o -> <i>
//   §l/§k/§m/§n and unknown codes -> stripped
// Literal formatting codes never reach the output. `palette` is the 16-entry
// legacy colour table (index = code value); `baseColorRgb` is the colour the
// line starts with and the colour §r restores.
std::wstring formatCodesToHtml(const std::wstring& escapedText,
                               uint32_t baseColorRgb,
                               const uint32_t palette[16]);
