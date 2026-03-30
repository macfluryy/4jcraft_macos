#include "../../../../../Header Files/stdafx.h"
#include "../../../net.minecraft.h"
#include "../dimension/net.minecraft.world.level.dimension.h"
#include "../net.minecraft.world.level.h"
#include "../redstone/net.minecraft.world.level.redstone.h"
#include "entity/net.minecraft.world.level.tile.entity.h"
#include "../../net.minecraft.world.h"
#include "java/JavaMath.h"
#include "DaylightDetectorTile.h"

DaylightDetectorTile::DaylightDetectorTile(int id)
    : BaseEntityTile(id, Material::wood, false) {
    updateDefaultShape();
}

void DaylightDetectorTile::updateDefaultShape() {
    setShape(0, 0, 0, 1, 6.0f / 16.0f, 1);
}

void DaylightDetectorTile::updateShape(
    LevelSource* level, int x, int y, int z, int forceData,
    std::shared_ptr<TileEntity> forceEntity) {
    setShape(0, 0, 0, 1, 6.0f / 16.0f, 1);
}

int DaylightDetectorTile::getSignal(LevelSource* level, int x, int y, int z,
                                    int dir) {
    return level->getData(x, y, z);
}

void DaylightDetectorTile::tick(Level* level, int x, int y, int z,
                                Random* random) {
    //        updateSignalStrength(level, x, y, z);
}

void DaylightDetectorTile::neighborChanged(Level* level, int x, int y, int z,
                                           int type) {
    //        level.addToTickNextTick(x, y, z, id, getTickDelay());
}

void DaylightDetectorTile::onPlace(Level* level, int x, int y, int z) {
    //        level.addToTickNextTick(x, y, z, id, getTickDelay());
}

void DaylightDetectorTile::updateSignalStrength(Level* level, int x, int y,
                                                int z) {
    if (level->dimension->hasCeiling) return;

    int current = level->getData(x, y, z);
    int target =
        level->getBrightness(LightLayer::Sky, x, y, z) - level->skyDarken;
    float sunAngle = level->getSunAngle(1);

    // tilt sunAngle towards zenith (to make the transition to night
    // smoother)
    if (sunAngle < M_PI) {
        sunAngle = sunAngle + (0 - sunAngle) * .2f;
    } else {
        sunAngle = sunAngle + (M_PI * 2.0f - sunAngle) * .2f;
    }

    target = Math::round((float)target * Mth::cos(sunAngle));
    if (target < 0) {
        target = 0;
    }
    if (target > Redstone::SIGNAL_MAX) {
        target = Redstone::SIGNAL_MAX;
    }

    if (current != target) {
        level->setData(x, y, z, target, UPDATE_ALL);
    }
}

bool DaylightDetectorTile::isCubeShaped() { return false; }

bool DaylightDetectorTile::isSolidRender(bool isServerLevel) { return false; }

bool DaylightDetectorTile::isSignalSource() { return true; }

std::shared_ptr<TileEntity> DaylightDetectorTile::newTileEntity(Level* level) {
    return std::shared_ptr<DaylightDetectorTileEntity>(
        new DaylightDetectorTileEntity());
}

Icon* DaylightDetectorTile::getTexture(int face, int data) {
    if (face == Facing::UP) {
        return icons[0];
    }
    return icons[1];
}

void DaylightDetectorTile::registerIcons(IconRegister* iconRegister) {
    icons[0] = iconRegister->registerIcon(getIconName() + L"_top");
    icons[1] = iconRegister->registerIcon(getIconName() + L"_side");
}