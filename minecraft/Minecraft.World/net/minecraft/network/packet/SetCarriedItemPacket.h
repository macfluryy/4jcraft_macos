#pragma once

#include <memory>

#include "Packet.h"
#include "Minecraft.World/net/minecraft/network/packet/Packet.h"

class SetCarriedItemPacket
    : public Packet,
      public std::enable_shared_from_this<SetCarriedItemPacket> {
public:
    int slot;

    SetCarriedItemPacket();
    SetCarriedItemPacket(int slot);

    virtual void read(DataInputStream* dis);
    virtual void write(DataOutputStream* dos);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual bool canBeInvalidated();
    virtual bool isInvalidatedBy(std::shared_ptr<Packet> packet);

public:
    static std::shared_ptr<Packet> create() {
        return std::shared_ptr<Packet>(new SetCarriedItemPacket());
    }
    virtual int getId() { return 16; }
};