#pragma once
#include <string>

#include "StitchedTexture.h"



class SimpleIcon : public StitchedTexture {
public:
    SimpleIcon(const std::wstring& name, const std::wstring& filename, float u0,
               float v0, float u1, float v1);
};