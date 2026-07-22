#pragma once

#include <memory>

#include "Packet.h"
#include "minecraft/network/packet/Packet.h"




class UpdateProgressPacket
    : public Packet,
      public std::enable_shared_from_this<UpdateProgressPacket> {
public:
    int m_percentage;

    UpdateProgressPacket();
    UpdateProgressPacket(int percentage);

    virtual void read(DataInputStream* dis);
    virtual void write(DataOutputStream* dos);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

public:
    static std::shared_ptr<Packet> create() {
        return std::make_shared<UpdateProgressPacket>();
    }
    virtual int getId() { return 156; }
};