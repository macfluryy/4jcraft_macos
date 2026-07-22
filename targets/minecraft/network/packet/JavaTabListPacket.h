#pragma once

#include <memory>
#include <string>

#include "Packet.h"
#include "minecraft/network/packet/Packet.h"












class JavaTabListPacket
    : public Packet,
      public std::enable_shared_from_this<JavaTabListPacket> {
public:
    static const int ACTION_ADD = 0;     
    static const int ACTION_REMOVE = 1;  
    static const int ACTION_CLEAR = 2;   
    static const int ACTION_BIND = 3;    

    static const int MAX_NAME_LENGTH = 64;
    static const int UUID_BYTES = 16;

    int action = ACTION_ADD;
    std::string uuid;  
    std::wstring name;
    int ping = 0;
    int entityId = 0;  

    JavaTabListPacket();

    void read(DataInputStream* dis);
    void write(DataOutputStream* dos);
    void handle(PacketListener* listener);
    int getEstimatedSize();

public:
    static std::shared_ptr<Packet> create() {
        return std::make_shared<JavaTabListPacket>();
    }
    virtual int getId() { return 211; }
};
