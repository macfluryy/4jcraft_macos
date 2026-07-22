#pragma once

#include <vector>

namespace JavaKeyInput {

extern std::vector<int> pressedKeys;
extern std::vector<wchar_t> typedChars;
extern bool keysCurrent[512];
extern bool keysPrev[512];

}  
