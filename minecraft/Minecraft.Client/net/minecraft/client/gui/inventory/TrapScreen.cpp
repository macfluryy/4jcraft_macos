#include <string>

#include "TrapScreen.h"
#include "minecraft/world/level/tile/entity/DispenserTileEntity.h"
#include "minecraft/world/entity/player/Inventory.h"
#include "minecraft/world/inventory/TrapMenu.h"
#include "Minecraft.Client/net/minecraft/client/gui/Font.h"
#include "Minecraft.Client/net/minecraft/client/gui/inventory/AbstractContainerScreen.h"

#ifdef ENABLE_JAVA_GUIS
ResourceLocation GUI_TRAP_LOCATION = ResourceLocation(TN_GUI_TRAP);
#endif

TrapScreen::TrapScreen(std::shared_ptr<Inventory> inventory,
                       std::shared_ptr<DispenserTileEntity> trap)
    : AbstractContainerScreen(new TrapMenu(inventory, trap)) {
    this->trap = trap;
    this->inventory = inventory;
}

void TrapScreen::renderLabels() {
    font->draw(trap->getName(), 16 + 4 + 40, 2 + 2 + 2, 0x404040);
    font->draw(inventory->getName(), 8, imageHeight - 96 + 2, 0x404040);
}

void TrapScreen::renderBg(float a) {
    // 4J Unused
}