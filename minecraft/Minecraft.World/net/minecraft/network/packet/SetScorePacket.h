#pragma once

#include <memory>
#include <string>

#include "Packet.h"
#include "Minecraft.World/net/minecraft/network/packet/Packet.h"

class Score;

class SetScorePacket : public Packet,
                       public std::enable_shared_from_this<SetScorePacket> {
public:
    static const int METHOD_CHANGE = 0;
    static const int METHOD_REMOVE = 1;

    std::wstring owner;
    std::wstring objectiveName;
    int score;
    int method;

    SetScorePacket();
    SetScorePacket(Score* score, int method);
    SetScorePacket(const std::wstring& owner);

    void read(DataInputStream* dis);
    void write(DataOutputStream* dos);
    void handle(PacketListener* listener);
    int getEstimatedSize();

public:
    static std::shared_ptr<Packet> create() {
        return std::make_shared<SetScorePacket>();
    }
    virtual int getId() { return 207; }
};