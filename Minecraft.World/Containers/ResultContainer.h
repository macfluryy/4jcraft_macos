#pragma once

#include "Container.h"

class ResultContainer : public Container {
private:
    std::shared_ptr<ItemInstance> items[1];

public:
    // 4J Stu Added a ctor to init items
    ResultContainer();

    virtual unsigned int getContainerSize();
    virtual std::shared_ptr<ItemInstance> getItem(unsigned int slot);
    virtual std::wstring getName();
    virtual std::wstring getCustomName();
    virtual bool hasCustomName();
    virtual std::shared_ptr<ItemInstance> removeItem(unsigned int slot,
                                                     int count);
    virtual std::shared_ptr<ItemInstance> removeItemNoUpdate(int slot);
    virtual void setItem(unsigned int slot, std::shared_ptr<ItemInstance> item);
    virtual int getMaxStackSize();
    virtual void setChanged();
    virtual bool stillValid(std::shared_ptr<Player> player);
    virtual void startOpen() {}  // TODO Auto-generated method stub
    virtual void stopOpen() {}   // TODO Auto-generated method stub
    virtual bool canPlaceItem(int slot, std::shared_ptr<ItemInstance> item);
};