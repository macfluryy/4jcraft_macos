#pragma once

#include "AbstractContainerScreen.h"
#include "../../../../../../Minecraft.World/net/minecraft/world/inventory/BrewingStandMenu.h"
#include "../../../../../../Minecraft.World/net/minecraft/world/level/tile/entity/net.minecraft.world.level.tile.entity.h"

class BrewingStandScreen : public AbstractContainerScreen {
public:
    BrewingStandScreen(std::shared_ptr<Inventory> inventory,
                       std::shared_ptr<BrewingStandTileEntity> brewingStand);
    virtual ~BrewingStandScreen();

    void init() override;
    void removed() override;
    void renderLabels() override;
    void renderBg(float a) override;
    void render(int xm, int ym, float a) override;

private:
    std::shared_ptr<Inventory> inventory;
    std::shared_ptr<BrewingStandTileEntity> brewingStand;
    BrewingStandMenu* brewMenu;
};