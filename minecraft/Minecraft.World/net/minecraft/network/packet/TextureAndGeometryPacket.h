#pragma once
#include <cstdint>

#include "Packet.h"
#include "Minecraft.Client/net/minecraft/client/model/geom/Model.h"
#include "Minecraft.Client/Header Files/SkinBox.h"

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
        return std::shared_ptr<Packet>(new TextureAndGeometryPacket());
    }
    virtual int getId() { return 160; }
};
