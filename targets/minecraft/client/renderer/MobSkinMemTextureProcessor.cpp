#include "app/include/MobSkinMemTextureProcessor.h"

#include <cstring>

#include "app/include/BufferedImage.h"












BufferedImage* MobSkinMemTextureProcessor::process(BufferedImage* in) {
    if (in == nullptr) return nullptr;
    if (in->getWidth() != 64 || in->getHeight() != 64) return in;

    width = 64;
    height = 32;

    BufferedImage* out =
        new BufferedImage(width, height, BufferedImage::TYPE_INT_ARGB);
    const int* src = in->getData();
    pixels = out->getData();

    
    memcpy(pixels, src, static_cast<size_t>(64 * 32) * sizeof(int));

    
    copyOverlay(src, 0, 32, 0, 16, 16, 16);    
    copyOverlay(src, 16, 32, 16, 16, 24, 16);  
    copyOverlay(src, 40, 32, 40, 16, 16, 16);  
    
    

    setNoAlpha(0, 0, 32, 16);
    setForceAlpha(32, 0, 64, 32);
    setNoAlpha(0, 16, 64, 32);

    return out;
}



void MobSkinMemTextureProcessor::copyOverlay(const int* src, int srcX, int srcY,
                                             int dstX, int dstY, int w, int h) {
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            const int pix = src[(srcX + x) + (srcY + y) * 64];
            if (((pix >> 24) & 0xff) >= 128)
                pixels[(dstX + x) + (dstY + y) * width] = pix;
        }
}

void MobSkinMemTextureProcessor::setForceAlpha(int x0, int y0, int x1, int y1) {
    if (hasAlpha(x0, y0, x1, y1)) return;

    for (int x = x0; x < x1; x++)
        for (int y = y0; y < y1; y++) {
            pixels[x + y * width] &= 0x00ffffff;
        }
}

void MobSkinMemTextureProcessor::setNoAlpha(int x0, int y0, int x1, int y1) {
    for (int x = x0; x < x1; x++)
        for (int y = y0; y < y1; y++) {
            pixels[x + y * width] |= 0xff000000;
        }
}

bool MobSkinMemTextureProcessor::hasAlpha(int x0, int y0, int x1, int y1) {
    for (int x = x0; x < x1; x++)
        for (int y = y0; y < y1; y++) {
            int pix = pixels[x + y * width];
            if (((pix >> 24) & 0xff) < 128) return true;
        }
    return false;
}
