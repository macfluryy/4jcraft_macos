#include "../../../../../../Minecraft.World/Header Files/stdafx.h"
#include "ContainerScreen.h"
#include "../../renderer/Textures.h"
#include "../../../../../../Minecraft.World/net/minecraft/world/inventory/net.minecraft.world.inventory.h"

ContainerScreen::ContainerScreen(std::shared_ptr<Container> inventory,
                                 std::shared_ptr<Container> container)
    : AbstractContainerScreen(new ContainerMenu(inventory, container)) {
    this->inventory = inventory;
    this->container = container;
    this->passEvents = false;

    int defaultHeight = 222;
    int noRowHeight = defaultHeight - 6 * 18;
    containerRows = container->getContainerSize() / 9;

    imageHeight = noRowHeight + containerRows * 18;
}

void ContainerScreen::renderLabels() {}

void ContainerScreen::renderBg(float a) {
    // 4J Unused
#if defined(ENABLE_JAVA_GUIS)
    int tex = minecraft->textures->loadTexture(TN_GUI_CONTAINER);
    glColor4f(1, 1, 1, 1);
    minecraft->textures->bind(tex);
    int xo = (width - imageWidth) / 2;
    int yo = (height - imageHeight) / 2;
    this->blit(xo, yo, 0, 0, imageWidth, containerRows * 18 + 17);
    this->blit(xo, yo + containerRows * 18 + 17, 0, 222 - 96, imageWidth, 96);
#endif
}