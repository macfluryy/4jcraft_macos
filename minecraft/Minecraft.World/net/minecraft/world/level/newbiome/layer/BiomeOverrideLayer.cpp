#include <string.h>
#include <string>
#include <fstream>
#include <filesystem>

#include "BiomeOverrideLayer.h"
#include "Minecraft.Client/Linux/Linux_App.h"
#if defined(__linux__)
#include "Minecraft.Client/Linux/Stubs/winapi_stubs.h"
#endif
#include "Minecraft.World/net/minecraft/world/level/biome/Biome.h"
#include "Minecraft.World/net/minecraft/world/level/newbiome/layer/Layer.h"

BiomeOverrideLayer::BiomeOverrideLayer(int seedMixup) : Layer(seedMixup) {
    m_biomeOverride = std::vector<uint8_t>(width * height);

    std::filesystem::path path = "GameRules/biomemap.bin";
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        // assert(false);
        app.DebugPrintf("Biome override not found, using plains as default\n");

        memset(m_biomeOverride.data(), Biome::plains->id, m_biomeOverride.size());
    } else {
        auto fileSize = std::filesystem::file_size(path);
        if (fileSize > m_biomeOverride.size()) {
            app.DebugPrintf("Biomemap binary is too large!!\n");
            __debugbreak();
        }
        file.read(reinterpret_cast<char*>(m_biomeOverride.data()), static_cast<std::streamsize>(fileSize));

        if (!file) {
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
