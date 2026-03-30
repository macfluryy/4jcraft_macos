#pragma once

#include "inventory/AbstractContainerScreen.h"
#include "../../../../../Minecraft.World/net/minecraft/world/inventory/MerchantMenu.h"
#include "../../../../../Minecraft.World/net/minecraft/world/item/trading/net.minecraft.world.item.trading.h"

class TradeSwitchButton;

class MerchantScreen : public AbstractContainerScreen {
public:
    MerchantScreen(std::shared_ptr<Inventory> inventory,
                   std::shared_ptr<Merchant> merchant, Level* level);
    virtual ~MerchantScreen();

    void init() override;
    void removed() override;
    void renderLabels() override;
    void renderBg(float a) override;
    void render(int xm, int ym, float a) override;
    void tick() override;
    void buttonClicked(Button* button) override;

    std::shared_ptr<Merchant> getMerchant() { return merchant; }

private:
    std::shared_ptr<Inventory> inventory;
    std::shared_ptr<Merchant> merchant;
    MerchantMenu* merchantMenu;
    TradeSwitchButton* nextRecipeButton;
    TradeSwitchButton* prevRecipeButton;
    int currentRecipeIndex;
};