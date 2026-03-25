#include "../Platform/stdafx.h"
#include "../Headers/net.minecraft.world.entity.player.h"
#include "../Headers/net.minecraft.world.level.h"
#include "../Headers/net.minecraft.world.h"
#include "../Headers/net.minecraft.h"
#include "WorkbenchTile.h"

WorkbenchTile::WorkbenchTile(int id) : Tile(id, Material::wood) {
    iconTop = NULL;
    iconFront = NULL;
}

Icon* WorkbenchTile::getTexture(int face, int data) {
    if (face == Facing::UP) return iconTop;
    if (face == Facing::DOWN) return Tile::wood->getTexture(face);
    if (face == Facing::NORTH || face == Facing::WEST) return iconFront;
    return icon;
}

void WorkbenchTile::registerIcons(IconRegister* iconRegister) {
    icon = iconRegister->registerIcon(L"workbench_side");
    iconTop = iconRegister->registerIcon(L"workbench_top");
    iconFront = iconRegister->registerIcon(L"workbench_front");
}

// 4J-PB - Adding a TestUse for tooltip display
bool WorkbenchTile::TestUse() { return true; }

bool WorkbenchTile::use(Level* level, int x, int y, int z,
                        std::shared_ptr<Player> player, int clickedFace,
                        float clickX, float clickY, float clickZ,
                        bool soundOnly /*=false*/)  // 4J added soundOnly param
{
    if (soundOnly) return false;
    if (level->isClientSide) {
        return true;
    }
    player->startCrafting(x, y, z);
    // player->openFireworks(x, y, z);
    return true;
}