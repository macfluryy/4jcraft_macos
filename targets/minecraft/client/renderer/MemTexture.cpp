#include "app/include/MemTexture.h"

#include <cstdint>
#include <string>

#include "app/include/BufferedImage.h"
#include "app/include/MemTextureProcessor.h"

MemTexture::MemTexture(const std::wstring& _url, std::uint8_t* pbData,
                       std::uint32_t dataBytes,
                       MemTextureProcessor* processor) {
    
    count = 1;
    id = -1;
    isLoaded = false;
    ticksSinceLastUse = 0;

    
    
    loadedImage = new BufferedImage(pbData, dataBytes);
    if (processor != nullptr && loadedImage != nullptr) {
        
        
        BufferedImage* processed = processor->process(loadedImage);
        if (processed != nullptr && processed != loadedImage) {
            delete loadedImage;
            loadedImage = processed;
        }
    }
}

MemTexture::~MemTexture() { delete loadedImage; }
