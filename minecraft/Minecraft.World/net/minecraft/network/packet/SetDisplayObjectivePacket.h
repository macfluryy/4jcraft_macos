#pragma once

#include <memory>
#include <string>

#include "Packet.h"
#include "Minecraft.World/net/minecraft/network/packet/Packet.h"

class Objective;

class SetDisplayObjectivePacket
    : public Packet,
      public std::enable_shared_from_this<SetDisplayObjectivePacket> {
public:
    int slot;
    std::wstring objectiveName;

    SetDisplayObjectivePacket();
    SetDisplayObjectivePacket(int slot, Objective* objective);

    void read(DataInputStream* dis);
    void write(DataOutputStream* dos);
    void handle(PacketListener* listener);
    int getEstimatedSize();

public:
    static std::shared_ptr<Packet> create() {
        return std::make_shared<SetDisplayObjectivePacket>();
    }
    virtual int getId() { return 208; }
};