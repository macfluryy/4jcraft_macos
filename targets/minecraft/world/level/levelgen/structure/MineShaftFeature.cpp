
#include "minecraft/world/level/levelgen/structure/MineShaftFeature.h"

#include <stdlib.h>

#include <algorithm>
#include <string>
#include <unordered_map>
#include <utility>

#include "app/common/App_enums.h"
#include "app/common/src/GameRules/LevelGeneration/LevelGenerationOptions.h"
#include "app/linux/Linux_App.h"
#include "java/Random.h"
#include "minecraft/util/Mth.h"
#include "minecraft/world/level/levelgen/structure/MineShaftStart.h"

const std::wstring MineShaftFeature::OPTION_CHANCE = L"chance";

MineShaftFeature::MineShaftFeature() { chance = 0.01; }

std::wstring MineShaftFeature::getFeatureName() { return L"Mineshaft"; }

MineShaftFeature::MineShaftFeature(
    std::unordered_map<std::wstring, std::wstring> options) {
    chance = 0.01;

    for (auto it = options.begin(); it != options.end(); ++it) {
        if (it->first.compare(OPTION_CHANCE) == 0) {
            chance = Mth::getDouble(it->second, chance);
        }
    }
}

bool MineShaftFeature::isFeatureChunk(int x, int z, bool bIsSuperflat) {
    bool forcePlacement = false;
    LevelGenerationOptions* levelGenOptions = app.getLevelGenerationOptions();
    if (levelGenOptions != nullptr) {
        forcePlacement =
            levelGenOptions->isFeatureChunk(x, z, eFeature_Mineshaft);
    }

    return forcePlacement || (random->nextDouble() < chance &&
                              random->nextInt(80) < std::max(abs(x), abs(z)));
}

StructureStart* MineShaftFeature::createStructureStart(int x, int z) {
    // 4J added
    app.AddTerrainFeaturePosition(eTerrainFeature_Mineshaft, x, z);

    return new MineShaftStart(level, random, x, z);
}