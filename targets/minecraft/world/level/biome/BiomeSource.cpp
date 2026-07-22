#include "BiomeSource.h"

#include <assert.h>

#include <algorithm>

#include "IPlatformInput.h"
#include "app/common/src/Console_Debug_enum.h"
#include "app/mac/MacGame.h"
#include "java/Random.h"
#include "java/System.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/ProgressRenderer.h"
#include "minecraft/world/level/ChunkPos.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/TilePos.h"
#include "minecraft/world/level/LevelType.h"
#include "minecraft/world/level/biome/Biome.h"
#include "minecraft/world/level/biome/BiomeCache.h"
#include "minecraft/world/level/newbiome/layer/Layer.h"
#include "minecraft/world/level/storage/LevelData.h"
#include "platform/PlatformServices.h"
#include "strings.h"



void BiomeSource::_init() {
    layer = nullptr;
    zoomedLayer = nullptr;

    cache = new BiomeCache(this);

    playerSpawnBiomes.push_back(Biome::forest);
    playerSpawnBiomes.push_back(Biome::taiga);
    
    
    playerSpawnBiomes.push_back(Biome::plains);
    playerSpawnBiomes.push_back(Biome::taigaHills);
    playerSpawnBiomes.push_back(Biome::forestHills);
    playerSpawnBiomes.push_back(Biome::jungle);
    playerSpawnBiomes.push_back(Biome::jungleHills);
}

void BiomeSource::_init(int64_t seed, LevelType* generator) {
    _init();

    std::vector<std::shared_ptr<Layer>> layers =
        Layer::getDefaultLayers(seed, generator);
    layer = layers[0];
    zoomedLayer = layers[1];
}

BiomeSource::BiomeSource() { _init(); }


BiomeSource::BiomeSource(int64_t seed, LevelType* generator) {
    _init(seed, generator);
}



BiomeSource::BiomeSource(Level* level) {
    _init(level->getSeed(), level->getLevelData()->getGenerator());
}

BiomeSource::~BiomeSource() { delete cache; }

Biome* BiomeSource::getBiome(ChunkPos* cp) {
    return getBiome(cp->x << 4, cp->z << 4);
}

Biome* BiomeSource::getBiome(int x, int z) { return cache->getBiome(x, z); }

float BiomeSource::getDownfall(int x, int z) const {
    return cache->getDownfall(x, z);
}



std::vector<float> BiomeSource::getDownfallBlock(int x, int z, int w,
                                                 int h) const {
    std::vector<float> downfalls;
    getDownfallBlock(downfalls, x, z, w, h);
    return downfalls;
}




void BiomeSource::getDownfallBlock(std::vector<float>& downfalls, int x, int z,
                                   int w, int h) const {
    
    if (downfalls.empty() || downfalls.size() < (unsigned int)(w * h)) {
        downfalls = std::vector<float>(w * h);
    }

    std::vector<int> result = zoomedLayer->getArea(x, z, w, h);
    for (int i = 0; i < w * h; i++) {
        float d = (float)Biome::biomes[result[i]]->getDownfallInt() / 65536.0f;
        if (d > 1) d = 1;
        downfalls[i] = d;
    }
}

BiomeCache::Block* BiomeSource::getBlockAt(int x, int y) {
    return cache->getBlockAt(x, y);
}

float BiomeSource::getTemperature(int x, int y, int z) const {
    return scaleTemp(cache->getTemperature(x, z), y);
}


float BiomeSource::scaleTemp(float temp, int y) const { return temp; }

std::vector<float> BiomeSource::getTemperatureBlock(int x, int z, int w,
                                                    int h) const {
    std::vector<float> temperatures;
    getTemperatureBlock(temperatures, x, z, w, h);
    return temperatures;
}




void BiomeSource::getTemperatureBlock(std::vector<float>& temperatures, int x,
                                      int z, int w, int h) const {
    
    if (temperatures.empty() || temperatures.size() < (unsigned int)(w * h)) {
        temperatures = std::vector<float>(w * h);
    }

    std::vector<int> result = zoomedLayer->getArea(x, z, w, h);
    for (int i = 0; i < w * h; i++) {
        float t =
            (float)Biome::biomes[result[i]]->getTemperatureInt() / 65536.0f;
        if (t > 1) t = 1;
        temperatures[i] = t;
    }
}

std::vector<Biome*> BiomeSource::getRawBiomeBlock(int x, int z, int w,
                                                  int h) const {
    std::vector<Biome*> biomes;
    getRawBiomeBlock(biomes, x, z, w, h);
    return biomes;
}


void BiomeSource::getRawBiomeIndices(std::vector<int>& biomes, int x, int z,
                                     int w, int h) const {
    std::vector<int> result = layer->getArea(x, z, w, h);
    for (int i = 0; i < w * h; i++) {
        biomes[i] = result[i];
    }
}

void BiomeSource::getRawBiomeBlock(std::vector<Biome*>& biomes, int x, int z,
                                   int w, int h) const {
    
    if (biomes.empty() || biomes.size() < (unsigned int)(w * h)) {
        biomes = std::vector<Biome*>(w * h);
    }

    std::vector<int> result = layer->getArea(x, z, w, h);
    for (int i = 0; i < w * h; i++) {
        biomes[i] = Biome::biomes[result[i]];
#if !defined(_CONTENT_PACKAGE)
        if (biomes[i] == nullptr) {
            app.DebugPrintf("Tried to assign null biome %d\n", result[i]);
            assert(0);
        }
#endif
    }
}

std::vector<Biome*> BiomeSource::getBiomeBlock(int x, int z, int w,
                                               int h) const {
    if (w == 16 && h == 16 && (x & 0xf) == 0 && (z & 0xf) == 0) {
        return cache->getBiomeBlockAt(x, z);
    }
    std::vector<Biome*> biomes;
    getBiomeBlock(biomes, x, z, w, h, true);
    return biomes;
}


void BiomeSource::getBiomeBlock(std::vector<Biome*>& biomes, int x, int z,
                                int w, int h, bool useCache) const {
    
    if (biomes.empty() || biomes.size() < w * h) {
        biomes = std::vector<Biome*>(w * h);
    }

    if (useCache && w == 16 && h == 16 && (x & 0xf) == 0 && (z & 0xf) == 0) {
        std::vector<Biome*> tmp = cache->getBiomeBlockAt(x, z);
        std::copy(tmp.begin(), tmp.begin() + w * h, biomes.begin());
        
        
    }

    std::vector<int> result = zoomedLayer->getArea(x, z, w, h);
    for (int i = 0; i < w * h; i++) {
        biomes[i] = Biome::biomes[result[i]];
    }
}

std::vector<uint8_t> BiomeSource::getBiomeIndexBlock(int x, int z, int w,
                                                     int h) const {
    if (w == 16 && h == 16 && (x & 0xf) == 0 && (z & 0xf) == 0) {
        return cache->getBiomeIndexBlockAt(x, z);
    }
    std::vector<uint8_t> biomeIndices;
    getBiomeIndexBlock(biomeIndices, x, z, w, h, true);
    return biomeIndices;
}


void BiomeSource::getBiomeIndexBlock(std::vector<uint8_t>& biomeIndices, int x,
                                     int z, int w, int h, bool useCache) const {
    
    if (biomeIndices.empty() || biomeIndices.size() < w * h) {
        biomeIndices = std::vector<uint8_t>(w * h);
    }

    if (useCache && w == 16 && h == 16 && (x & 0xf) == 0 && (z & 0xf) == 0) {
        std::vector<uint8_t> tmp = cache->getBiomeIndexBlockAt(x, z);
        std::copy(tmp.begin(), tmp.begin() + w * h, biomeIndices.begin());
        
    }

    std::vector<int> result = zoomedLayer->getArea(x, z, w, h);
    for (int i = 0; i < w * h; i++) {
        biomeIndices[i] = (uint8_t)result[i];
    }
}








bool BiomeSource::containsOnly(int x, int z, int r,
                               const std::vector<Biome*>& allowed) {
    int x0 = ((x - r) >> 2);
    int z0 = ((z - r) >> 2);
    int x1 = ((x + r) >> 2);
    int z1 = ((z + r) >> 2);

    int w = x1 - x0 + 1;
    int h = z1 - z0 + 1;

    std::vector<int> biomes = layer->getArea(x0, z0, w, h);
    for (int i = 0; i < w * h; i++) {
        Biome* b = Biome::biomes[biomes[i]];
        if (find(allowed.begin(), allowed.end(), b) == allowed.end())
            return false;
    }

    return true;
}








bool BiomeSource::containsOnly(int x, int z, int r, Biome* allowed) {
    int x0 = ((x - r) >> 2);
    int z0 = ((z - r) >> 2);
    int x1 = ((x + r) >> 2);
    int z1 = ((z + r) >> 2);

    int w = x1 - x0;
    int h = z1 - z0;
    int biomesCount = w * h;
    std::vector<int> biomes = layer->getArea(x0, z0, w, h);
    for (unsigned int i = 0; i < biomesCount; i++) {
        Biome* b = Biome::biomes[biomes[i]];
        if (allowed != b) return false;
    }

    return true;
}







TilePos* BiomeSource::findBiome(int x, int z, int r, Biome* toFind,
                                Random* random) {
    int x0 = ((x - r) >> 2);
    int z0 = ((z - r) >> 2);
    int x1 = ((x + r) >> 2);
    int z1 = ((z + r) >> 2);

    int w = x1 - x0 + 1;
    int h = z1 - z0 + 1;
    std::vector<int> biomes = layer->getArea(x0, z0, w, h);
    TilePos* res = nullptr;
    int found = 0;
    int biomesCount = w * h;
    for (unsigned int i = 0; i < biomesCount; i++) {
        int xx = x0 + i % w;
        int zz = z0 + i / w;
        Biome* b = Biome::biomes[biomes[i]];
        if (b == toFind) {
            if (res == nullptr || random->nextInt(found + 1) == 0) {
                res = new TilePos(xx, 0, zz);
                found++;
            }
        }
    }

    return res;
}







TilePos* BiomeSource::findBiome(int x, int z, int r,
                                const std::vector<Biome*>& allowed,
                                Random* random) {
    int x0 = ((x - r) >> 2);
    int z0 = ((z - r) >> 2);
    int x1 = ((x + r) >> 2);
    int z1 = ((z + r) >> 2);

    int w = x1 - x0 + 1;
    int h = z1 - z0 + 1;
    std::vector<int> biomes = layer->getArea(x0, z0, w, h);
    TilePos* res = nullptr;
    int found = 0;
    for (unsigned int i = 0; i < w * h; i++) {
        int xx = (x0 + i % w) << 2;
        int zz = (z0 + i / w) << 2;
        Biome* b = Biome::biomes[biomes[i]];
        if (find(allowed.begin(), allowed.end(), b) != allowed.end()) {
            if (res == nullptr || random->nextInt(found + 1) == 0) {
                delete res;
                res = new TilePos(xx, 0, zz);
                found++;
            }
        }
    }

    return res;
}

void BiomeSource::update() { cache->update(); }




int64_t BiomeSource::findSeed(LevelType* generator) {
    int64_t bestSeed = 0;

    ProgressRenderer* mcprogress = Minecraft::GetInstance()->progressRenderer;
    mcprogress->progressStage(IDS_PROGRESS_NEW_WORLD_SEED);

    
    
    
    
    
    
    if (generator == LevelType::lvl_largeBiomes) {
        Random rand(System::nanoTime());
        return rand.nextLong();
    }

#if !defined(_CONTENT_PACKAGE)
    if (app.DebugSettingsOn() &&
        app.GetGameSettingsDebugMask(PlatformInput.GetPrimaryPad()) &
            (1L << eDebugSetting_EnableBiomeOverride)) {
        
    } else
#endif
    {
#if defined(DEBUG_SEEDS)
        for (int k = 0; k < DEBUG_SEEDS; k++)
#endif
        {
            
            Random* pr = new Random(System::nanoTime());

            
            
            
            
            
            static const int biomeWidth =
                (54 * 4) - 16;  
            static const int biomeOffset = -(biomeWidth / 2);

            
            std::vector<int> indices =
                std::vector<int>(biomeWidth * biomeWidth);

            
            
            float toCompare[Biome::BIOME_COUNT];

            bool matchFound = false;
            int tryCount = 0;

            
            
            do {
                int64_t seed = pr->nextLong();
                BiomeSource* biomeSource = new BiomeSource(seed, generator);

                biomeSource->getRawBiomeIndices(
                    indices, biomeOffset, biomeOffset, biomeWidth, biomeWidth);
                getFracs(indices, toCompare);

                matchFound = getIsMatch(toCompare);

                if (matchFound) bestSeed = seed;

                delete biomeSource;
                tryCount++;

                mcprogress->progressStagePercentage(tryCount % 100);
            } while (!matchFound);

            
            delete pr;

#if defined(DEBUG_SEEDS)
            app.DebugPrintf("%d: %d tries taken, seed used is %lld\n", k,
                            tryCount, bestSeed);

            BiomeSource* biomeSource = new BiomeSource(bestSeed);
            std::vector<Biome*> biomes = biomeSource->getBiomeBlock(
                -27 * 16, -27 * 16, 54 * 16, 54 * 16);

            unsigned int* pixels = new unsigned int[54 * 16 * 54 * 16];
            for (int i = 0; i < 54 * 16 * 54 * 16; i++) {
                int id = biomes[i]->id;

                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                

                if (id == 1)
                    id = 14;
                else if (id == 14)
                    id = 1;
                else if (id == 9)
                    id = 15;
                else if (id == 15)
                    id = 9;
                pixels[i] = 0xff000000;
                if (id & 1) pixels[i] |= 0x00ff0000;
                if (id & 2) pixels[i] |= 0x0000ff00;
                if (id & 4) pixels[i] |= 0x000000ff;
                if (id & 8) pixels[i] |= 0x00808080;
            }
            D3DXIMAGE_INFO srcInfo;
            srcInfo.Format = D3DFMT_LIN_A8R8G8B8;
            srcInfo.ImageFileFormat = D3DXIFF_BMP;
            srcInfo.Width = 54 * 16;
            srcInfo.Height = 54 * 16;

            char buf[256];
            sprintf(buf, "GAME:\\BiomeTest%d.bmp", k);
            PlatformRender.SaveTextureData(buf, &srcInfo, (int*)pixels);

            delete[] pixels;
            delete biomeSource;
#endif
        }
    }

    return bestSeed;
}


void BiomeSource::getFracs(std::vector<int>& indices, float* fracs) {
    for (int i = 0; i < Biome::BIOME_COUNT; i++) {
        fracs[i] = 0.0f;
    }

    for (int i = 0; i < indices.size(); i++) {
        fracs[indices[i]] += 1.0f;
    }

    for (int i = 0; i < Biome::BIOME_COUNT; i++) {
        fracs[i] /= (float)(indices.size());
    }
}



bool BiomeSource::getIsMatch(float* frac) {
    
    
    static const bool critical[Biome::BIOME_COUNT] = {
        true,   
        true,   
        true,   
        false,  
        true,   
        true,   
        true,   
        false,  
        false,  
        false,  
        false,  
        false,  
        false,  
        false,  
        true,   
        false,  
        false,  
        false,  
        false,  
        false,  
        false,  
        true,   
        false,  
    };

    
    if (frac[0] > 0.15f) {
        return false;
    }

    
    frac[14] = ((frac[15] > frac[14]) ? frac[15] : frac[14]);

    
    frac[2] = ((frac[17] > frac[2]) ? frac[17] : frac[2]);

    
    frac[4] = ((frac[18] > frac[4]) ? frac[18] : frac[4]);

    
    frac[5] = ((frac[19] > frac[5]) ? frac[19] : frac[5]);

    
    frac[21] = ((frac[22] > frac[21]) ? frac[22] : frac[21]);

    
    
    

    int typeCount = 0;
    for (int i = 0; i < Biome::BIOME_COUNT; i++) {
        
        if (i == 15 || i == 17 || i == 18 || i == 19 || i == 22) continue;

        
        
        if (frac[i] > 0.001f) {
            typeCount++;
        } else {
            
            if (critical[i]) {
                return false;
            }
        }
    }

    
    
    
    return (typeCount >= 9);
}
