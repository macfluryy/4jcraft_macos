#include "BiomeOverrideLayer.h"

#include <string.h>

#include "app/linux/Linux_App.h"
#include "platform/PlatformServices.h"
#include "minecraft/world/level/newbiome/layer/Layer.h"
#if defined(__linux__)
#include "app/linux/Stubs/winapi_stubs.h"
#endif
#include "minecraft/world/level/biome/Biome.h"

BiomeOverrideLayer::BiomeOverrideLayer(int seedMixup) : Layer(seedMixup) {
    m_biomeOverride = std::vector<uint8_t>(width * height);

    {
        const char* path = "GameRules/biomemap.bin";
        auto result = PlatformFileIO.readFile(
            path, m_biomeOverride.data(), m_biomeOverride.size());
        if (result.status == IPlatformFileIO::ReadStatus::NotFound) {
            app.DebugPrintf("Biome override not found, using plains as default\n");
            memset(m_biomeOverride.data(), Biome::plains->id,
                   m_biomeOverride.size());
        } else if (result.status == IPlatformFileIO::ReadStatus::TooLarge) {
            app.DebugPrintf("Biomemap binary is too large!!\n");
            __debugbreak();
        } else if (result.status != IPlatformFileIO::ReadStatus::Ok) {
            app.FatalLoadError();
        }
    }
}

std::vector<int> BiomeOverrideLayer::getArea(int xo, int yo, int w, int h) {
    std::vector<int> result(w * h);

    int xOrigin = xo + width / 2;
    int yOrigin = yo + height / 2;
    if (xOrigin < 0) xOrigin = 0;
    if (xOrigin >= width) xOrigin = width - 1;
    if (yOrigin < 0) yOrigin = 0;
    if (yOrigin >= height) yOrigin = height - 1;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int curX = xOrigin + x;
            int curY = yOrigin + y;
            if (curX >= width) curX = width - 1;
            if (curY >= height) curY = height - 1;
            int index = curX + curY * width;

            unsigned char headerValue = m_biomeOverride[index];
            result[x + y * w] = headerValue;
        }
    }
    return result;
}
