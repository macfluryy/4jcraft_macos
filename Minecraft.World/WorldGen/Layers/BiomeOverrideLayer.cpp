#include "../../Platform/stdafx.h"
#include "../../Util/PortableFileIO.h"
#include "../../Headers/net.minecraft.world.level.biome.h"
#include "../../Headers/net.minecraft.world.level.newbiome.layer.h"
#include "../../Headers/net.minecraft.world.level.h"
#include "BiomeOverrideLayer.h"

BiomeOverrideLayer::BiomeOverrideLayer(int seedMixup) : Layer(seedMixup) {
    m_biomeOverride = byteArray(width * height);

#ifdef _WINDOWS64
    const std::wstring path = L"GameRules\\biomemap.bin";
#else
    const std::wstring path = L"GAME:\\GameRules\\biomemap.bin";
#endif
    const PortableFileIO::BinaryReadResult readResult =
        PortableFileIO::ReadBinaryFile(path, m_biomeOverride.data,
                                       m_biomeOverride.length);
    if (readResult.status == PortableFileIO::BinaryReadStatus::not_found) {
        app.DebugPrintf("Biome override not found, using plains as default\n");
        memset(m_biomeOverride.data, Biome::plains->id, m_biomeOverride.length);
    } else if (readResult.status ==
               PortableFileIO::BinaryReadStatus::too_large) {
        app.DebugPrintf("Biomemap binary is too large!!\n");
        __debugbreak();
    } else if (readResult.status != PortableFileIO::BinaryReadStatus::ok) {
        app.FatalLoadError();
    }
}

intArray BiomeOverrideLayer::getArea(int xo, int yo, int w, int h) {
    intArray result = IntCache::allocate(w * h);

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
