#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "platform/PlatformTypes.h"
#include "Packet.h"
#include "minecraft/network/packet/Packet.h"

class PreLoginPacket : public Packet,
                       public std::enable_shared_from_this<PreLoginPacket> {
    
public:
    static const int m_iSaveNameLen = 14;
    
    
    
    
    
    PlayerUID* m_playerXuids;
    std::uint8_t m_dwPlayerCount;
    std::uint8_t m_friendsOnlyBits;
    std::uint32_t m_ugcPlayersVersion;
    char m_szUniqueSaveName[m_iSaveNameLen];  
                                              
    std::uint32_t
        m_serverSettings;  
                           
    std::uint8_t
        m_hostIndex;  
                      
    std::uint32_t m_texturePackId;
    std::int16_t m_netcodeVersion;

    std::wstring loginKey;

    PreLoginPacket();
    PreLoginPacket(std::wstring userName);
    PreLoginPacket(std::wstring userName, PlayerUID* playerXuids,
                   std::uint8_t playerCount, std::uint8_t friendsOnlyBits,
                   std::uint32_t ugcPlayersVersion,
                   const char* pszUniqueSaveName, std::uint32_t serverSettings,
                   std::uint8_t hostIndex, std::uint32_t texturePackId);
    ~PreLoginPacket();

    virtual void read(DataInputStream* dis);
    virtual void write(DataOutputStream* dos);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

public:
    static std::shared_ptr<Packet> create() {
        return std::make_shared<PreLoginPacket>();
    }
    virtual int getId() { return 2; }
};
