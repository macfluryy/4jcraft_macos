#pragma once

#include <limits>
#include <format>
#include <memory>
#include <vector>

#include "Packet.h"
#include "Minecraft.World/net/minecraft/network/packet/Packet.h"

class RemoveEntitiesPacket
    : public Packet,
      public std::enable_shared_from_this<RemoveEntitiesPacket> {
public:
    static const int MAX_PER_PACKET = std::numeric_limits<char>::max();

    std::vector<int> ids;

    RemoveEntitiesPacket();
    RemoveEntitiesPacket(std::vector<int>& ids);
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
