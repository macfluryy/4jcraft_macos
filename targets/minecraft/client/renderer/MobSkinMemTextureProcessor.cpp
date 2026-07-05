#include "app/include/MobSkinMemTextureProcessor.h"

#include <cstring>

#include "app/include/BufferedImage.h"

// 4jcraft: implemented as a direct pixel-buffer transform. The original Java
// version drew through Graphics, but BufferedImage::getGraphics() is a stub
// (returns nullptr) in this port, so we copy pixels directly.
//
// Modern Java skins are 64x64: the top half is exactly the legacy 64x32
// layout (head+hat, body, right arm, right leg), the bottom half holds the
// separate left limbs plus second-layer overlays. The legacy humanoid model
// mirrors the right limbs for the left side and has no overlay slots for
// body/limbs, so we convert by copying the top half and baking the overlays
// that have legacy equivalents onto their base regions. Anything that is not
// 64x64 (legacy/DLC 64x32 skins in particular) passes through untouched.
BufferedImage* MobSkinMemTextureProcessor::process(BufferedImage* in) {
    if (in == nullptr) return nullptr;
    if (in->getWidth() != 64 || in->getHeight() != 64) return in;

    width = 64;
    height = 32;

    BufferedImage* out =
        new BufferedImage(width, height, BufferedImage::TYPE_INT_ARGB);
    const int* src = in->getData();
    pixels = out->getData();

    // Top half of a modern 64x64 skin is the legacy 64x32 layout verbatim.
    memcpy(pixels, src, static_cast<size_t>(64 * 32) * sizeof(int));

    // Bake second-layer overlays onto their legacy base regions.
    copyOverlay(src, 0, 32, 0, 16, 16, 16);    // right pant   -> right leg
    copyOverlay(src, 16, 32, 16, 16, 24, 16);  // jacket       -> body
    copyOverlay(src, 40, 32, 40, 16, 16, 16);  // right sleeve -> right arm
    // Left-limb data (rows 48-63) has no legacy destination: the legacy model
    // mirrors the right limbs. Dropped rather than distorted.

    setNoAlpha(0, 0, 32, 16);
    setForceAlpha(32, 0, 64, 32);
    setNoAlpha(0, 16, 64, 32);

    return out;
}

// Copies a w*h overlay region from the 64x64 source onto the destination
// region of the 64x32 output, keeping only sufficiently opaque texels.
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
