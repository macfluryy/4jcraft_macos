#include "../../Platform/stdafx.h"
#include "TrapScreen.h"
#include "../../Player/LocalPlayer.h"
#include "../../Textures/Textures.h"
#include "../../../Minecraft.World/Headers/net.minecraft.world.inventory.h"
#include "../../../Minecraft.World/Blocks/TileEntities/DispenserTileEntity.h"
#include "../../../Minecraft.World/Headers/net.minecraft.world.h"

TrapScreen::TrapScreen(std::shared_ptr<Inventory> inventory,
                       std::shared_ptr<DispenserTileEntity> trap)
    : AbstractContainerScreen(new TrapMenu(inventory, trap)) {}

void TrapScreen::renderLabels() {
    font->draw(L"Dispenser", 16 + 4 + 40, 2 + 2 + 2, 0x404040);
    font->draw(L"Inventory", 8, imageHeight - 96 + 2, 0x404040);
}

void TrapScreen::renderBg(float a) {
    // 4J Unused
}