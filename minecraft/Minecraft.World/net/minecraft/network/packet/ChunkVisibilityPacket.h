#pragma once

#include <iostream>
#include <memory>

#include "Packet.h"
#include "Minecraft.World/Header Files/stdafx.h"
#include "PacketListener.h"
#include "Minecraft.World/net/minecraft/network/packet/Packet.h"

class ChunkVisibilityPacket
    : public Packet,
      public std::enable_shared_from_this<ChunkVisibilityPacket> {
public:
    int x, z;
    bool visible;

    ChunkVisibilityPacket();
    ChunkVisibilityPacket(int x, int z, bool visible);

    virtual void read(DataInputStream* dis);
    virtual void write(DataOutputStream* dos);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

public:
    static std::shared_ptr<Packet> create() {
        return std::shared_ptr<Packet>(new ChunkVisibilityPacket());
    }
    virtual int getId() { return 50; }
};
