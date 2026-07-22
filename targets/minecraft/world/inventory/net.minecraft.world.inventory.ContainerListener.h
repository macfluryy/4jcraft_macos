#pragma once

class AbstractContainerMenu;







namespace net_minecraft_world_inventory {
class ContainerListener {
public:
    virtual void refreshContainer(
        AbstractContainerMenu* container,
        std::vector<std::shared_ptr<ItemInstance> >* items) = 0;

    virtual void slotChanged(AbstractContainerMenu* container, int slotIndex,
                             std::shared_ptr<ItemInstance> item) = 0;

    virtual void setContainerData(AbstractContainerMenu* container, int id,
                                  int value) = 0;
};
}  