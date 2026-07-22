#include "minecraft/world/level/newbiome/layer/Layer.h"

#include <stdint.h>

#include <memory>
#include <vector>

#include "BiomeOverrideLayer.h"
#include "IPlatformInput.h"
#include "app/common/src/Console_Debug_enum.h"
#include "app/mac/MacGame.h"
#include "minecraft/world/level/LevelType.h"
#include "minecraft/world/level/newbiome/layer/AddIslandLayer.h"
#include "minecraft/world/level/newbiome/layer/AddMushroomIslandLayer.h"
#include "minecraft/world/level/newbiome/layer/AddSnowLayer.h"
#include "minecraft/world/level/newbiome/layer/BiomeInitLayer.h"
#include "minecraft/world/level/newbiome/layer/FuzzyZoomLayer.h"
#include "minecraft/world/level/newbiome/layer/GrowMushroomIslandLayer.h"
#include "minecraft/world/level/newbiome/layer/IslandLayer.h"
#include "minecraft/world/level/newbiome/layer/RegionHillsLayer.h"
#include "minecraft/world/level/newbiome/layer/RiverInitLayer.h"
#include "minecraft/world/level/newbiome/layer/RiverLayer.h"
#include "minecraft/world/level/newbiome/layer/RiverMixerLayer.h"
#include "minecraft/world/level/newbiome/layer/ShoreLayer.h"
#include "minecraft/world/level/newbiome/layer/SmoothLayer.h"
#include "minecraft/world/level/newbiome/layer/SwampRiversLayer.h"
#include "minecraft/world/level/newbiome/layer/VoronoiZoom.h"
#include "minecraft/world/level/newbiome/layer/ZoomLayer.h"
#include "platform/PlatformServices.h"

std::vector<std::shared_ptr<Layer>> Layer::getDefaultLayers(
    int64_t seed, LevelType* levelType) {
    
    
    
    
    
    std::shared_ptr<Layer> islandLayer = std::make_shared<IslandLayer>(1);
    islandLayer = std::make_shared<FuzzyZoomLayer>(2000, islandLayer);
    islandLayer = std::make_shared<AddIslandLayer>(1, islandLayer);
    islandLayer = std::make_shared<ZoomLayer>(2001, islandLayer);
    islandLayer = std::make_shared<AddIslandLayer>(2, islandLayer);
    islandLayer = std::make_shared<AddSnowLayer>(2, islandLayer);
    islandLayer = std::make_shared<ZoomLayer>(2002, islandLayer);
    islandLayer = std::make_shared<AddIslandLayer>(3, islandLayer);
    islandLayer = std::make_shared<ZoomLayer>(2003, islandLayer);
    islandLayer = std::make_shared<AddIslandLayer>(4, islandLayer);
    
    

    int zoomLevel = 4;
    if (levelType == LevelType::lvl_largeBiomes) {
        zoomLevel = 6;
    }

    std::shared_ptr<Layer> riverLayer = islandLayer;
    riverLayer = ZoomLayer::zoom(1000, riverLayer, 0);
    riverLayer = std::make_shared<RiverInitLayer>(100, riverLayer);
    riverLayer = ZoomLayer::zoom(1000, riverLayer, zoomLevel + 2);
    riverLayer = std::make_shared<RiverLayer>(1, riverLayer);
    riverLayer = std::make_shared<SmoothLayer>(1000, riverLayer);

    std::shared_ptr<Layer> biomeLayer = islandLayer;
    biomeLayer = ZoomLayer::zoom(1000, biomeLayer, 0);
    biomeLayer = std::make_shared<BiomeInitLayer>(200, biomeLayer, levelType);

    biomeLayer = ZoomLayer::zoom(1000, biomeLayer, 2);
    biomeLayer = std::make_shared<RegionHillsLayer>(1000, biomeLayer);

    for (int i = 0; i < zoomLevel; i++) {
        biomeLayer = std::make_shared<ZoomLayer>(1000 + i, biomeLayer);

        if (i == 0)
            biomeLayer = std::make_shared<AddIslandLayer>(3, biomeLayer);

        if (i == 0) {
            
            
            
            
            
            
            biomeLayer = std::shared_ptr<Layer>(
                new AddMushroomIslandLayer(5, biomeLayer));
        }

        if (i == 1) {
            
            
            
            
            
            
            
            biomeLayer = std::shared_ptr<Layer>(
                new GrowMushroomIslandLayer(5, biomeLayer));
            
            
            
            biomeLayer = std::make_shared<ShoreLayer>(1000, biomeLayer);

            biomeLayer = std::make_shared<SwampRiversLayer>(1000, biomeLayer);
        }
    }

    biomeLayer = std::make_shared<SmoothLayer>(1000, biomeLayer);

    biomeLayer = std::shared_ptr<Layer>(
        new RiverMixerLayer(100, biomeLayer, riverLayer));

#if !defined(_CONTENT_PACKAGE)
#if defined(_BIOME_OVERRIDE)
    if (app.DebugSettingsOn() &&
        app.GetGameSettingsDebugMask(PlatformInput.GetPrimaryPad()) &
            (1L << eDebugSetting_EnableBiomeOverride)) {
        biomeLayer = std::make_shared<BiomeOverrideLayer>(1);
    }
#endif
#endif

    std::shared_ptr<Layer> debugLayer = biomeLayer;

    std::shared_ptr<Layer> zoomedLayer =
        std::make_shared<VoronoiZoom>(10, biomeLayer);

    biomeLayer->init(seed);
    zoomedLayer->init(seed);

    std::vector<std::shared_ptr<Layer>> result(3);
    result[0] = biomeLayer;
    result[1] = zoomedLayer;
    result[2] = debugLayer;
    return result;
}

Layer::Layer(int64_t seedMixup) {
    parent = nullptr;

    
    this->seedMixup = seedMixup;
    this->seedMixup *=
        (uint64_t)this->seedMixup * 6364136223846793005l + 1442695040888963407l;
    this->seedMixup = (uint64_t)this->seedMixup + seedMixup;
    this->seedMixup *=
        (uint64_t)this->seedMixup * 6364136223846793005l + 1442695040888963407l;
    this->seedMixup = (uint64_t)this->seedMixup + seedMixup;
    this->seedMixup *=
        (uint64_t)this->seedMixup * 6364136223846793005l + 1442695040888963407l;
    this->seedMixup = (uint64_t)this->seedMixup + seedMixup;
}

void Layer::init(int64_t seed) {
    this->seed = seed;
    if (parent != nullptr) parent->init(seed);
    
    this->seed *=
        (uint64_t)this->seed * 6364136223846793005l + 1442695040888963407l;
    this->seed = (uint64_t)this->seed + seedMixup;
    this->seed *=
        (uint64_t)this->seed * 6364136223846793005l + 1442695040888963407l;
    this->seed = (uint64_t)this->seed + seedMixup;
    this->seed *=
        (uint64_t)this->seed * 6364136223846793005l + 1442695040888963407l;
    this->seed = (uint64_t)this->seed + seedMixup;
}

void Layer::initRandom(int64_t x, int64_t y) {
    rval = seed;
    
    rval *= (uint64_t)rval * 6364136223846793005l + 1442695040888963407l;
    rval += (uint64_t)x;
    rval *= (uint64_t)rval * 6364136223846793005l + 1442695040888963407l;
    rval += (uint64_t)y;
    rval *= (uint64_t)rval * 6364136223846793005l + 1442695040888963407l;
    rval += (uint64_t)x;
    rval *= (uint64_t)rval * 6364136223846793005l + 1442695040888963407l;
    rval += (uint64_t)y;
}

int Layer::nextRandom(int max) {
    int result = (int)((rval >> 24) % max);

    if (result < 0) result += max;
    
    rval *= (uint64_t)rval * 6364136223846793005l + 1442695040888963407l;
    rval += (uint64_t)seed;
    return result;
}
