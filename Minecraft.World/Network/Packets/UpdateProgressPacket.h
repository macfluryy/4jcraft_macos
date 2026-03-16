#pragma once

#include "Packet.h"

// 4J Added packet to update clients on the time for the host to finish doing
// something

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
        return std::shared_ptr<Packet>(new UpdateProgressPacket());
    }
    virtual int getId() { return 156; }
};