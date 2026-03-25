#pragma once
#include "../Platform/stdafx.h"
#include "../Textures/Stitching/StitchedTexture.h"

// 4J Added this class to store the uv data that we have pre-calculated and
// loaded from a file
class SimpleIcon : public StitchedTexture {
public:
    SimpleIcon(const std::wstring& name, const std::wstring& filename, float u0,
               float v0, float u1, float v1);
};