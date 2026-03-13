#pragma once

#include "../../Commands/CommandsEnum.h"
#include "Packet.h"

class GameCommandPacket
    : public Packet,
      public std::enable_shared_from_this<GameCommandPacket> {
public:
    EGameCommand command;
    int length;
    byteArray data;

    GameCommandPacket();
    GameCommandPacket(EGameCommand command, byteArray data);
    ~GameCommandPacket();

    virtual void read(DataInputStream* dis);
    virtual void write(DataOutputStream* dos);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

public:
    static std::shared_ptr<Packet> create() {
        return std::shared_ptr<Packet>(new GameCommandPacket());
    }
    virtual int getId() { return 167; }
};