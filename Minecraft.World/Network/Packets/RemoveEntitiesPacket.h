#pragma once

#include <limits>
#include "Packet.h"

class RemoveEntitiesPacket
    : public Packet,
      public std::enable_shared_from_this<RemoveEntitiesPacket> {
public:
    static const int MAX_PER_PACKET = std::numeric_limits<char>::max();

    intArray ids;

    RemoveEntitiesPacket();
    RemoveEntitiesPacket(intArray ids);
    ~RemoveEntitiesPacket();

    virtual void read(DataInputStream* dis);
    virtual void write(DataOutputStream* dos);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

public:
    static std::shared_ptr<Packet> create() {
        return std::shared_ptr<Packet>(new RemoveEntitiesPacket());
    }
    virtual int getId() { return 29; }
};
