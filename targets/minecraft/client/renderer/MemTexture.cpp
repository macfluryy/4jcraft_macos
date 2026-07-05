#include "app/include/MemTexture.h"

#include <cstdint>
#include <string>

#include "app/include/BufferedImage.h"
#include "app/include/MemTextureProcessor.h"

MemTexture::MemTexture(const std::wstring& _url, std::uint8_t* pbData,
                       std::uint32_t dataBytes,
                       MemTextureProcessor* processor) {
    // 4J - added
    count = 1;
    id = -1;
    isLoaded = false;
    ticksSinceLastUse = 0;

    // load the texture, and process it
    // (Java original: loadedImage=processor.process(ImageIO.read(...)))
    loadedImage = new BufferedImage(pbData, dataBytes);
    if (processor != nullptr && loadedImage != nullptr) {
        // Processor may return the input unchanged (no conversion needed) or a
        // freshly allocated converted image, in which case we own both.
        BufferedImage* processed = processor->process(loadedImage);
        if (processed != nullptr && processed != loadedImage) {
            delete loadedImage;
            loadedImage = processed;
        }
    }
}

MemTexture::~MemTexture() { delete loadedImage; }
