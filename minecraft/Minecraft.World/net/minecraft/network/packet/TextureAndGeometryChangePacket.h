#pragma once
#include <cstdint>

#include "Packet.h"

class TextureAndGeometryChangePacket
    : public Packet,
      public std::enable_shared_from_this<TextureAndGeometryChangePacket> {
public:
    int id;
    std::wstring path;
    std::uint32_t dwSkinID;

    TextureAndGeometryChangePacket();
    TextureAndGeometryChangePacket(std::shared_ptr<Entity> e,
                                   const std::wstring& path);

    virtual void read(DataInputStream* dis);
    virtual void write(DataOutputStream* dos);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

public:
    static std::shared_ptr<Packet> create() {
        return std::shared_ptr<Packet>(new TextureAndGeometryChangePacket());
    }
    virtual int getId() { return 161; }
};
