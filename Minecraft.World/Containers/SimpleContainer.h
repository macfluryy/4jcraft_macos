#pragma once

#include "Container.h"
#include "../Headers/net.minecraft.world.ContainerListener.h"

class SimpleContainer : public Container {
private:
    int name;
    std::wstring stringName;
    int size;
    ItemInstanceArray* items;
    std::vector<net_minecraft_world::ContainerListener*>* listeners;
    bool customName;

public:
    SimpleContainer(int name, std::wstring stringName, bool customName,
                    int size);

    virtual void addListener(net_minecraft_world::ContainerListener* listener);
    virtual void removeListener(
        net_minecraft_world::ContainerListener* listener);
    virtual std::shared_ptr<ItemInstance> getItem(unsigned int slot) override;
    virtual std::shared_ptr<ItemInstance> removeItem(unsigned int slot,
                                                     int count) override;
    virtual std::shared_ptr<ItemInstance> removeItemNoUpdate(int slot) override;
    virtual void setItem(unsigned int slot,
                         std::shared_ptr<ItemInstance> item) override;
    virtual unsigned int getContainerSize() override;
    virtual std::wstring getName() override;
    virtual std::wstring getCustomName() override;
    virtual bool hasCustomName() override;
    virtual void setCustomName(const std::wstring& name);
    virtual int getMaxStackSize() override;
    virtual void setChanged() override;
    virtual bool stillValid(std::shared_ptr<Player> player) override;
    virtual void startOpen() override {}  // TODO Auto-generated method stub
    virtual void stopOpen() override {}   // TODO Auto-generated method stub
    virtual bool canPlaceItem(int slot,
                              std::shared_ptr<ItemInstance> item) override;
};