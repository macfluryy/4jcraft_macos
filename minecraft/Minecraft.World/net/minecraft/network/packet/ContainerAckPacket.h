#pragma once

#include <iostream>
#include <memory>

#include "Packet.h"
#include "Minecraft.World/Header Files/stdafx.h"
#include "PacketListener.h"
#include "Minecraft.World/net/minecraft/network/packet/Packet.h"

class ContainerAckPacket
    : public Packet,
      public std::enable_shared_from_this<ContainerAckPacket> {
public:
    int containerId;
    short uid;
    bool accepted;

    ContainerAckPacket();
    ContainerAckPacket(int containerId, short uid, bool accepted);

    virtual void handle(PacketListener* listener);
    virtual void read(DataInputStream* dis);
    virtual void write(DataOutputStream* dos);
    virtual int getEstimatedSize();

public:
    static std::shared_ptr<Packet> create() {
        return std::shared_ptr<Packet>(new ContainerAckPacket());
    }
    virtual int getId() { return 106; }
};
