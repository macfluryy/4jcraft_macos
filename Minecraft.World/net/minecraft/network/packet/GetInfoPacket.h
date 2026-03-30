#pragma once

#include "Packet.h"

class GetInfoPacket : public Packet,
                      public std::enable_shared_from_this<GetInfoPacket> {
public:
    virtual void read(DataInputStream* dis);
    virtual void write(DataOutputStream* dos);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

public:
    static std::shared_ptr<Packet> create() {
        return std::shared_ptr<Packet>(new GetInfoPacket());
    }
    virtual int getId() { return 254; }
};