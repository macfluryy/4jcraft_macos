#pragma once

#include <memory>
#include <string>

#include "Packet.h"
#include "minecraft/network/packet/Packet.h"




class HudOverlayPacket
    : public Packet,
      public std::enable_shared_from_this<HudOverlayPacket> {
public:
    static const int ACTION_ACTIONBAR = 0;
    static const int ACTION_TITLE = 1;
    static const int ACTION_SUBTITLE = 2;
    static const int ACTION_TIMES = 3;
    static const int ACTION_CLEAR = 4;
    static const int ACTION_RESET = 5;

    static const int MAX_TEXT_LENGTH = 128;

    int action = ACTION_ACTIONBAR;
    std::wstring text;   
    int fadeIn = 0;      
    int stay = 0;
    int fadeOut = 0;

    HudOverlayPacket();
    HudOverlayPacket(int action, const std::wstring& text);

    void read(DataInputStream* dis);
    void write(DataOutputStream* dos);
    void handle(PacketListener* listener);
    int getEstimatedSize();

public:
    static std::shared_ptr<Packet> create() {
        return std::make_shared<HudOverlayPacket>();
    }
    virtual int getId() { return 210; }
};
