#pragma once

#include "Packet.h"

class Objective;

class SetObjectivePacket
    : public Packet,
      public std::enable_shared_from_this<SetObjectivePacket> {
public:
    static const int METHOD_ADD = 0;
    static const int METHOD_REMOVE = 1;
    static const int METHOD_CHANGE = 2;

    std::wstring objectiveName;
    std::wstring displayName;
    int method;

    SetObjectivePacket();
    SetObjectivePacket(Objective* objective, int method);
    void read(DataInputStream* dis);
    void write(DataOutputStream* dos);
    void handle(PacketListener* listener);
    int getEstimatedSize();

public:
    static std::shared_ptr<Packet> create() {
        return std::shared_ptr<Packet>(new SetObjectivePacket());
    }
    virtual int getId() { return 206; }
};