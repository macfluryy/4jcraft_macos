#pragma once

#include <iostream>
#include <memory>

#include "Packet.h"
#include "PacketListener.h"
#include "minecraft/network/packet/Packet.h"

class ClientInformationPacket
    : public Packet,
      public std::enable_shared_from_this<ClientInformationPacket> {
public:
    int viewDistance;  // in chunks

    ClientInformationPacket();
    ClientInformationPacket(int viewDistance);

    virtual void read(DataInputStream* dis);
    virtual void write(DataOutputStream* dos);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

public:
    static std::shared_ptr<Packet> create() {
        return std::make_shared<ClientInformationPacket>();
    }
    virtual int getId() { return 204; }
};
