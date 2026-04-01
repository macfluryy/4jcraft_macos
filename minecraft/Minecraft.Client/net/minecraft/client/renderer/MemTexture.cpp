#include <cstdint>
#include <string>

#include "Minecraft.Client/include/MemTexture.h"
#include "Minecraft.Client/include/BufferedImage.h"

class MemTextureProcessor;

MemTexture::MemTexture(const std::wstring& _url, std::uint8_t* pbData,
                       std::uint32_t dataBytes,
                       MemTextureProcessor* processor) {
    // 4J - added
    count = 1;
    id = -1;
    isLoaded = false;
    ticksSinceLastUse = 0;

    // 4J - TODO - actually implement

    // load the texture, and process it
    // loadedImage=Textures::getTexture()
    // 4J - remember to add deletes in here for any created BufferedImages when
    // implemented
    loadedImage = new BufferedImage(pbData, dataBytes);
    if (processor == nullptr) {
    } else {
        // loadedImage=processor.process(ImageIO.read(huc.getInputStream()));
    }
}

MemTexture::~MemTexture() { delete loadedImage; }
