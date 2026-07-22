#pragma once

#include <cstdint>
#include <format>
#include <memory>
#include <string>
#include <vector>

#include "platform/PlatformTypes.h"
#include "Packet.h"
#include "minecraft/network/packet/Packet.h"
#include "minecraft/world/entity/SyncedEntityData.h"

class Player;

class AddPlayerPacket : public Packet,
                        public std::enable_shared_from_this<AddPlayerPacket> {
private:
    std::shared_ptr<SynchedEntityData> entityData;
    std::vector<std::shared_ptr<SynchedEntityData::DataItem> >* unpack;

public:
    int id;
    std::wstring name;
    int x, y, z;
    char yRot, xRot;
    int carriedItem;
    PlayerUID xuid;                   
    PlayerUID OnlineXuid;             
    std::uint8_t m_playerIndex;       
    std::uint32_t m_skinId;           
    std::uint32_t m_capeId;           
    unsigned int m_uiGamePrivileges;  
    std::uint8_t yHeadRot;            

    AddPlayerPacket();
    ~AddPlayerPacket();
    AddPlayerPacket(std::shared_ptr<Player> player, PlayerUID xuid,
                    PlayerUID OnlineXuid, int xp, int yp, int zp, int yRotp,
                    int xRotp, int yHeadRotp);

    virtual void read(DataInputStream* dis);
    virtual void write(DataOutputStream* dos);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    std::vector<std::shared_ptr<SynchedEntityData::DataItem> >*
    getUnpackedData();

public:
    static std::shared_ptr<Packet> create() {
        return std::make_shared<AddPlayerPacket>();
    }
    virtual int getId() { return 20; }
};
