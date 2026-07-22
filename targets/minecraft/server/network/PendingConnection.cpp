#include "PendingConnection.h"

#include <stdio.h>

#include <cstdint>
#include <vector>

#include "platform/PlatformTypes.h"
#include "platform/sdl2/Storage.h"
#include "app/common/App_enums.h"
#include "app/common/src/BuildVer/BuildVer.h"
#include "app/common/src/Network/NetworkPlayerInterface.h"
#include "platform/IPlatformNetwork.h"
#include "app/mac/MacGame.h"
#include "app/mac/Stubs/winapi_stubs.h"
#include "app/include/NetTypes.h"
#include "PlayerConnection.h"
#include "ServerConnection.h"
#include "java/Random.h"
#include "minecraft/SharedConstants.h"
#include "minecraft/network/Connection.h"
#include "minecraft/network/packet/DisconnectPacket.h"
#include "minecraft/network/packet/LoginPacket.h"
#include "minecraft/network/packet/PreLoginPacket.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/server/PlayerList.h"
#include "minecraft/server/level/ServerPlayer.h"

class Packet;




Random* PendingConnection::random = new Random();

PendingConnection::PendingConnection(MinecraftServer* server, Socket* socket,
                                     const std::wstring& id) {
    
    done = false;
    _tick = 0;
    name = L"";
    acceptedLogin = nullptr;
    loginKey = L"";

    this->server = server;
    connection = new Connection(socket, id, this);
    connection->fakeLag = FAKE_LAG;
}

PendingConnection::~PendingConnection() { delete connection; }

void PendingConnection::tick() {
    if (acceptedLogin != nullptr) {
        this->handleAcceptedLogin(acceptedLogin);
        acceptedLogin = nullptr;
    }
    if (_tick++ == MAX_TICKS_BEFORE_LOGIN) {
        disconnect(DisconnectPacket::eDisconnect_LoginTooLong);
    } else {
        connection->tick();
    }
}

void PendingConnection::disconnect(DisconnectPacket::eDisconnectReason reason) {
    
    
    fprintf(stderr, "[PENDING] disconnect called with reason=%d at tick=%d\n",
            reason, _tick);
    app.DebugPrintf("Pending connection disconnect: %d\n", reason);
    connection->send(std::make_shared<DisconnectPacket>(reason));
    connection->sendAndQuit();
    done = true;
    
    
    
}

void PendingConnection::handlePreLogin(std::shared_ptr<PreLoginPacket> packet) {
    if (packet->m_netcodeVersion != MINECRAFT_NET_VERSION) {
        app.DebugPrintf("Netcode version is %d not equal to %d\n",
                        packet->m_netcodeVersion, MINECRAFT_NET_VERSION);
        if (packet->m_netcodeVersion > MINECRAFT_NET_VERSION) {
            disconnect(DisconnectPacket::eDisconnect_OutdatedServer);
        } else {
            disconnect(DisconnectPacket::eDisconnect_OutdatedClient);
        }
        return;
    }
    
    name =
        packet->loginKey;  
                           
    sendPreLoginResponse();
}

void PendingConnection::sendPreLoginResponse() {
    
    PlayerUID* ugcXuids = new PlayerUID[MINECRAFT_NET_MAX_PLAYERS];
    std::uint8_t ugcXuidCount = 0;
    std::uint8_t hostIndex = 0;
    std::uint8_t ugcFriendsOnlyBits = 0;
    char szUniqueMapName[14];

    StorageManager.GetSaveUniqueFilename(szUniqueMapName);

    PlayerList* playerList = MinecraftServer::getInstance()->getPlayers();
    for (auto it = playerList->players.begin(); it != playerList->players.end();
         ++it) {
        std::shared_ptr<ServerPlayer> player = *it;
        
        
        
        

        
        

        if (player != nullptr &&
            player->connection->m_offlineXUID != INVALID_XUID &&
            player->connection->m_onlineXUID != INVALID_XUID) {
            if (player->connection->m_friendsOnlyUGC) {
                ugcFriendsOnlyBits |= (1 << ugcXuidCount);
            }
            
            
            ugcXuids[ugcXuidCount] = player->connection->m_onlineXUID;

            if (player->connection->getNetworkPlayer() != nullptr &&
                player->connection->getNetworkPlayer()->IsHost())
                hostIndex = ugcXuidCount;

            ++ugcXuidCount;
        }
    }

    {
        connection->send(std::shared_ptr<PreLoginPacket>(
            new PreLoginPacket(L"-", ugcXuids, ugcXuidCount, ugcFriendsOnlyBits,
                               server->m_ugcPlayersVersion, szUniqueMapName,
                               app.GetGameHostOption(eGameHostOption_All),
                               hostIndex, server->m_texturePackId)));
    }
}

void PendingConnection::handleLogin(std::shared_ptr<LoginPacket> packet) {
    fprintf(stderr, "[LOGIN-SRV] handleLogin called! clientVersion=%d\n",
            packet->clientVersion);
    
    if (packet->clientVersion != SharedConstants::NETWORK_PROTOCOL_VERSION) {
        app.DebugPrintf("Client version is %d not equal to %d\n",
                        packet->clientVersion,
                        SharedConstants::NETWORK_PROTOCOL_VERSION);
        if (packet->clientVersion > SharedConstants::NETWORK_PROTOCOL_VERSION) {
            disconnect(DisconnectPacket::eDisconnect_OutdatedServer);
        } else {
            disconnect(DisconnectPacket::eDisconnect_OutdatedClient);
        }
        return;
    }

    
    bool sentDisconnect = false;

    if (sentDisconnect) {
        
    } else if (server->getPlayers()->isXuidBanned(packet->m_onlineXuid)) {
        disconnect(DisconnectPacket::eDisconnect_Banned);
    } else {
        handleAcceptedLogin(packet);
    }
    
    {
        
    }
}

void PendingConnection::handleAcceptedLogin(
    std::shared_ptr<LoginPacket> packet) {
    if (packet->m_ugcPlayersVersion != server->m_ugcPlayersVersion) {
        
        sendPreLoginResponse();
        return;
    }

    
    PlayerUID playerXuid = packet->m_offlineXuid;
    if (playerXuid == INVALID_XUID) playerXuid = packet->m_onlineXuid;

    std::shared_ptr<ServerPlayer> playerEntity =
        server->getPlayers()->getPlayerForLogin(this, name, playerXuid,
                                                packet->m_onlineXuid);
    if (playerEntity != nullptr) {
        server->getPlayers()->placeNewPlayer(connection, playerEntity, packet);
        connection = nullptr;  
                               
                               
    }
    done = true;
}

void PendingConnection::onDisconnect(DisconnectPacket::eDisconnectReason reason,
                                     void* reasonObjects) {
    
    done = true;
}

void PendingConnection::handleGetInfo(std::shared_ptr<GetInfoPacket> packet) {
    
    
    
    
    connection->send(std::shared_ptr<DisconnectPacket>(
        new DisconnectPacket(DisconnectPacket::eDisconnect_ServerFull)));
    connection->sendAndQuit();
    server->connection->removeSpamProtection(connection->getSocket());
    done = true;
    
    
    
}

void PendingConnection::handleKeepAlive(
    std::shared_ptr<KeepAlivePacket> packet) {
    
}

void PendingConnection::onUnhandledPacket(std::shared_ptr<Packet> packet) {
    disconnect(DisconnectPacket::eDisconnect_UnexpectedPacket);
}

void PendingConnection::send(std::shared_ptr<Packet> packet) {
    connection->send(packet);
}

std::wstring PendingConnection::getName() {
    return L"Unimplemented";
    
    
    
}

bool PendingConnection::isServerPacketListener() { return true; }

bool PendingConnection::isDisconnected() { return done; }