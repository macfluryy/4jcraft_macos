#pragma once
#include <cstdint>
#include <format>
#include <memory>
#include <string>

#include "Minecraft.Client/include/SkinBox.h"
#include "Packet.h"
#include "minecraft/client/model/geom/Model.h"
#include "minecraft/network/packet/Packet.h"

class DLCSkinFile;

class TextureAndGeometryPacket
    : public Packet,
      public std::enable_shared_from_this<TextureAndGeometryPacket> {
public:
    std::wstring textureName;
    std::uint32_t dwSkinID;
    std::uint8_t* pbData;
    std::uint32_t dwTextureBytes;
    SKIN_BOX* BoxDataA;
    std::uint32_t dwBoxC;
    unsigned int uiAnimOverrideBitmask;

    TextureAndGeometryPacket();
    ~TextureAndGeometryPacket();
    TextureAndGeometryPacket(const std::wstring& textureName,
                             std::uint8_t* pbData, std::uint32_t dataBytes);
    TextureAndGeometryPacket(const std::wstring& textureName,
                             std::uint8_t* pbData, std::uint32_t dataBytes,
                             DLCSkinFile* pDLCSkinFile);
    TextureAndGeometryPacket(const std::wstring& textureName,
                             std::uint8_t* pbData, std::uint32_t dataBytes,
                             std::vector<SKIN_BOX*>* pvSkinBoxes,
                             unsigned int uiAnimOverrideBitmask);

    virtual void handle(PacketListener* listener);
    virtual void read(DataInputStream* dis);
    virtual void write(DataOutputStream* dos);
    virtual int getEstimatedSize();

public:
    static std::shared_ptr<Packet> create() {
        return std::make_shared<TextureAndGeometryPacket>();
    }
    virtual int getId() { return 160; }
};
