#pragma once

#include "Packet.h"

class ContainerSetContentPacket
    : public Packet,
      public std::enable_shared_from_this<ContainerSetContentPacket> {
public:
    int containerId;
    arrayWithLength<std::shared_ptr<ItemInstance>> items;

    ContainerSetContentPacket();
    ~ContainerSetContentPacket();
    ContainerSetContentPacket(
        int containerId, std::vector<std::shared_ptr<ItemInstance> >* newItems);

    virtual void read(DataInputStream* dis);
    virtual void write(DataOutputStream* dos);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

public:
    static std::shared_ptr<Packet> create() {
        return std::shared_ptr<Packet>(new ContainerSetContentPacket());
    }
    virtual int getId() { return 104; }
};
