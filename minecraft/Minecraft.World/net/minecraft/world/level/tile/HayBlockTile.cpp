#include <string>

#include "HayBlockTile.h"
#include "Minecraft.World/net/minecraft/world/IconRegister.h"
#include "Minecraft.World/net/minecraft/world/level/material/Material.h"
#include "Minecraft.World/net/minecraft/world/level/tile/RotatedPillarTile.h"

HayBlockTile::HayBlockTile(int id) : RotatedPillarTile(id, Material::grass) {}

int HayBlockTile::getRenderShape() { return SHAPE_TREE; }

Icon* HayBlockTile::getTypeTexture(int type) { return icon; }

void HayBlockTile::registerIcons(IconRegister* iconRegister) {
    iconTop = iconRegister->registerIcon(getIconName() + L"_top");
    icon = iconRegister->registerIcon(getIconName() + L"_side");
}