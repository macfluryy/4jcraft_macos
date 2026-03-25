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
    virtual std::shared_ptr<ItemInstance> getItem(unsigned int slot);
    virtual std::shared_ptr<ItemInstance> removeItem(unsigned int slot,
                                                     int count);
    virtual std::shared_ptr<ItemInstance> removeItemNoUpdate(int slot);
    virtual void setItem(unsigned int slot, std::shared_ptr<ItemInstance> item);
    virtual unsigned int getContainerSize();
    virtual std::wstring getName();
    virtual std::wstring getCustomName();
    virtual bool hasCustomName();
    virtual void setCustomName(const std::wstring& name);
    virtual int getMaxStackSize();
    virtual void setChanged();
    virtual bool stillValid(std::shared_ptr<Player> player);
    virtual void startOpen() {}  // TODO Auto-generated method stub
    virtual void stopOpen() {}   // TODO Auto-generated method stub
    virtual bool canPlaceItem(int slot, std::shared_ptr<ItemInstance> item);
};