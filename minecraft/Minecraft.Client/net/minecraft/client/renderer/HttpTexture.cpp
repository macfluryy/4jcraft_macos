#include "Minecraft.World/Header Files/stdafx.h"
#include "HttpTexture.h"

HttpTexture::HttpTexture(const std::wstring& _url,
                         HttpTextureProcessor* processor) {
    // 4J - added
    count = 1;
    id = -1;
    isLoaded = false;

    // 4J - TODO - actually implement
}