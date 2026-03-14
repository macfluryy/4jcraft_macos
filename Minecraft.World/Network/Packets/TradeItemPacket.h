#pragma once

// 4J ADDED PACKET

#include "Packet.h"

#include "../../Platform/stdafx.h"
#include <iostream>
#include "PacketListener.h"

class TradeItemPacket : public Packet,
                        public std::enable_shared_from_this<TradeItemPacket> {
public:
    int containerId;
    int offer;

    TradeItemPacket();
    TradeItemPacket(int containerId, int offer);

    virtual void handle(PacketListener* listener);
    virtual void read(DataInputStream* dis);
    virtual void write(DataOutputStream* dos);
    virtual int getEstimatedSize();

public:
    static std::shared_ptr<Packet> create() {
        return std::shared_ptr<Packet>(new TradeItemPacket());
    }
    virtual int getId() { return 151; }
};
