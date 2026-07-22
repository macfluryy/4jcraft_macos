#include "HttpTexture.h"

HttpTexture::HttpTexture(const std::wstring& _url,
                         HttpTextureProcessor* processor) {
    
    count = 1;
    id = -1;
    isLoaded = false;

    
}