#pragma once

#include "../../Platform/stdafx.h"
#include "AbstractContainerScreen.h"
#include "../../../Minecraft.World/Containers/AnvilMenu.h"
#include "../../../Minecraft.World/Headers/net.minecraft.world.inventory.ContainerListener.h"

class EditBox;

class RepairScreen : public AbstractContainerScreen, public ContainerListener {
public:
    RepairScreen(std::shared_ptr<Inventory> inventory, Level* level, int x,
                 int y, int z);
    virtual ~RepairScreen();

    void init();
    void removed();
    void render(int xm, int ym, float a);
    void renderLabels();
    void renderBg(float a);
    void keyPressed(char ch, int eventKey);
    void mouseClicked(int mouseX, int mouseY, int buttonNum);

    // 4jcraft: these 3 are to implement Containerlistener (see
    // IUIScene_AnvilMenu and net.minecraft.world.inventory.ContainerListener)
    void refreshContainer(AbstractContainerMenu* container,
                          std::vector<std::shared_ptr<ItemInstance> >* items);
    void slotChanged(AbstractContainerMenu* container, int slotIndex,
                     std::shared_ptr<ItemInstance> item);
    void setContainerData(AbstractContainerMenu* container, int id, int value);

private:
    void updateItemName();

    std::shared_ptr<Inventory> inventory;
    Level* level;
    AnvilMenu* repairMenu;
    EditBox* editName;
};