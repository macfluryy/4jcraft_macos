#pragma once

#include "Packet.h"

class SetCreativeModeSlotPacket
    : public Packet,
      public std::enable_shared_from_this<SetCreativeModeSlotPacket> {
public:
    int slotNum;
    std::shared_ptr<ItemInstance> item;

    SetCreativeModeSlotPacket();
    SetCreativeModeSlotPacket(int slotNum, std::shared_ptr<ItemInstance> item);

    virtual void handle(PacketListener* listener);
    virtual void read(DataInputStream* dis);
    virtual void write(DataOutputStream* dos);
    virtual int getEstimatedSize();

public:
    static std::shared_ptr<Packet> create() {
        return std::shared_ptr<Packet>(new SetCreativeModeSlotPacket());
    }
    virtual int getId() { return 107; }
};