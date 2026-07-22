#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Packet.h"
#include "minecraft/network/packet/Packet.h"

class ChatPacket : public Packet,
                   public std::enable_shared_from_this<ChatPacket> {
    
private:
    static const unsigned int MAX_LENGTH;

public:
    
    
    
    
    enum EChatPacketMessage {
        e_ChatCustom = 0,  
        e_ChatBedOccupied,
        e_ChatBedNoSleep,
        e_ChatBedNotValid,
        e_ChatBedNotSafe,
        e_ChatBedPlayerSleep,
        e_ChatBedMeSleep,
        e_ChatPlayerLeftGame,
        e_ChatPlayerJoinedGame,
        e_ChatPlayerKickedFromGame,
        e_ChatCannotPlaceLava,

        e_ChatDeathInFire,
        e_ChatDeathOnFire,
        e_ChatDeathLava,
        e_ChatDeathInWall,
        e_ChatDeathDrown,
        e_ChatDeathStarve,
        e_ChatDeathCactus,
        e_ChatDeathFall,
        e_ChatDeathOutOfWorld,
        e_ChatDeathGeneric,
        e_ChatDeathExplosion,
        e_ChatDeathMagic,
        e_ChatDeathMob,
        e_ChatDeathPlayer,
        e_ChatDeathArrow,
        e_ChatDeathFireball,
        e_ChatDeathThrown,
        e_ChatDeathIndirectMagic,
        e_ChatDeathDragonBreath,
        e_ChatDeathAnvil,
        e_ChatDeathFallingBlock,
        e_ChatDeathThorns,

        e_ChatDeathFellAccidentLadder,
        e_ChatDeathFellAccidentVines,
        e_ChatDeathFellAccidentWater,
        e_ChatDeathFellAccidentGeneric,
        e_ChatDeathFellKiller,
        e_ChatDeathFellAssist,
        e_ChatDeathFellAssistItem,
        e_ChatDeathFellFinish,
        e_ChatDeathFellFinishItem,
        e_ChatDeathInFirePlayer,
        e_ChatDeathOnFirePlayer,
        e_ChatDeathLavaPlayer,
        e_ChatDeathDrownPlayer,
        e_ChatDeathCactusPlayer,
        e_ChatDeathExplosionPlayer,
        e_ChatDeathWither,
        e_ChatDeathPlayerItem,
        e_ChatDeathArrowItem,
        e_ChatDeathFireballItem,
        e_ChatDeathThrownItem,
        e_ChatDeathIndirectMagicItem,

        e_ChatPlayerEnteredEnd,
        e_ChatPlayerLeftEnd,

        e_ChatPlayerMaxPigsSheepCows,  
                                       
        e_ChatPlayerMaxChickens,  
                                  
        e_ChatPlayerMaxSquid,  
        e_ChatPlayerMaxMooshrooms,  
                                    
        e_ChatPlayerMaxWolves,  
        e_ChatPlayerMaxAnimals,    
                                   
        e_ChatPlayerMaxEnemies,    
                                   
        e_ChatPlayerMaxVillagers,  
                                   
        e_ChatPlayerMaxHangingEntities,   
                                          
        e_ChatPlayerCantSpawnInPeaceful,  
                                          
        e_ChatPlayerMaxBredAnimals,       
                                     
                                     
        e_ChatPlayerMaxBredPigsSheepCows,  
                                           
                                           
        e_ChatPlayerMaxBredChickens,  
                                      
                                      
        e_ChatPlayerMaxBredMooshrooms,  
                                        
                                        
        e_ChatPlayerMaxBredWolves,  
                                    
                                    
        e_ChatPlayerCantShearMooshroom,  
                                         
        e_ChatPlayerMaxBoats,
        e_ChatPlayerMaxBats,

        e_ChatCommandTeleportSuccess,
        e_ChatCommandTeleportMe,
        e_ChatCommandTeleportToMe,

    };

public:
    std::vector<std::wstring> m_stringArgs;
    std::vector<int> m_intArgs;
    EChatPacketMessage m_messageType;

    ChatPacket();

    
    
    
    ChatPacket(const std::wstring& message,
               EChatPacketMessage type = e_ChatCustom, int customData = -1);
    ChatPacket(const std::wstring& message, EChatPacketMessage type,
               int sourceEntityType, const std::wstring& sourceName);
    ChatPacket(const std::wstring& message, EChatPacketMessage type,
               int sourceEntityType, const std::wstring& sourceName,
               const std::wstring& itemName);

    virtual void read(DataInputStream* dis);
    virtual void write(DataOutputStream* dos);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

public:
    static std::shared_ptr<Packet> create() {
        return std::make_shared<ChatPacket>();
    }
    virtual int getId() { return 3; }
};
