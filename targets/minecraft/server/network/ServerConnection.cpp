#include "ServerConnection.h"

#include <algorithm>

#include "app/mac/MacGame.h"
#include "PendingConnection.h"
#include "PlayerConnection.h"
#include "util/StringHelpers.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/multiplayer/MultiPlayerLevel.h"
#include "minecraft/network/Connection.h"
#include "minecraft/network/packet/DisconnectPacket.h"
#include "minecraft/network/packet/ServerSettingsChangedPacket.h"
#include "minecraft/server/level/ServerPlayer.h"

ServerConnection::ServerConnection(MinecraftServer* server) {
    
    connectionCounter = 0;

    this->server = server;
}

ServerConnection::~ServerConnection() {}



void ServerConnection::NewIncomingSocket(Socket* socket) {
    std::shared_ptr<PendingConnection> unconnectedClient =
        std::make_shared<PendingConnection>(
            server, socket,
            L"Connection #" + toWString<int>(connectionCounter++));
    handleConnection(unconnectedClient);
}

void ServerConnection::addPlayerConnection(
    std::shared_ptr<PlayerConnection> uc) {
    std::lock_guard<std::mutex> lock(players_cs);
    players.push_back(uc);
}

void ServerConnection::handleConnection(std::shared_ptr<PendingConnection> uc) {
    {
        std::lock_guard<std::mutex> lock(pending_cs);
        pending.push_back(uc);
    }
}

void ServerConnection::stop() {
    {
        std::lock_guard<std::mutex> lock(pending_cs);
        for (unsigned int i = 0; i < pending.size(); i++) {
            std::shared_ptr<PendingConnection> uc = pending[i];
            uc->connection->close(DisconnectPacket::eDisconnect_Closed);
        }
    }

    std::vector<std::shared_ptr<PlayerConnection> > playersSnapshot;
    {
        std::lock_guard<std::mutex> lock(players_cs);
        playersSnapshot = players;
    }
    for (unsigned int i = 0; i < playersSnapshot.size(); i++) {
        std::shared_ptr<PlayerConnection> player = playersSnapshot[i];
        player->connection->close(DisconnectPacket::eDisconnect_Closed);
    }
}

void ServerConnection::tick() {
    {
        
        
        std::vector<std::shared_ptr<PendingConnection> > tempPending;
        {
            std::lock_guard<std::mutex> lock(pending_cs);
            tempPending = pending;
        }

        for (unsigned int i = 0; i < tempPending.size(); i++) {
            std::shared_ptr<PendingConnection> uc = tempPending[i];
            
            uc->tick();
            
            
            
            
            
            if (uc->connection != nullptr) uc->connection->flush();
        }
    }

    
    {
        std::lock_guard<std::mutex> lock(pending_cs);
        for (unsigned int i = 0; i < pending.size(); i++)
            if (pending[i]->done) {
                pending.erase(pending.begin() + i);
                i--;
            }
    }

    
    
    
    
    std::vector<std::shared_ptr<PlayerConnection> > playersSnapshot;
    {
        std::lock_guard<std::mutex> lock(players_cs);
        playersSnapshot = players;
    }
    for (unsigned int i = 0; i < playersSnapshot.size(); i++) {
        std::shared_ptr<PlayerConnection> player = playersSnapshot[i];
        std::shared_ptr<ServerPlayer> serverPlayer = player->getPlayer();
        if (serverPlayer) {
            serverPlayer->updateFrameTick();
            serverPlayer->doChunkSendingTick(false);
        }
        player->tick();
        if (player->connection != nullptr) player->connection->flush();
    }
    
    {
        std::lock_guard<std::mutex> lock(players_cs);
        for (unsigned int i = 0; i < players.size();) {
            if (players[i]->done) {
                players.erase(players.begin() + i);
            } else {
                ++i;
            }
        }
    }
}

bool ServerConnection::addPendingTextureRequest(
    const std::wstring& textureName) {
    auto it = find(m_pendingTextureRequests.begin(),
                   m_pendingTextureRequests.end(), textureName);
    if (it == m_pendingTextureRequests.end()) {
        m_pendingTextureRequests.push_back(textureName);
        return true;
    }

    
    
    
    
    
    
    return true;
}

void ServerConnection::handleTextureReceived(const std::wstring& textureName) {
    auto it = find(m_pendingTextureRequests.begin(),
                   m_pendingTextureRequests.end(), textureName);
    if (it != m_pendingTextureRequests.end()) {
        m_pendingTextureRequests.erase(it);
    }
    std::vector<std::shared_ptr<PlayerConnection> > playersSnapshot;
    {
        std::lock_guard<std::mutex> lock(players_cs);
        playersSnapshot = players;
    }
    for (unsigned int i = 0; i < playersSnapshot.size(); i++) {
        std::shared_ptr<PlayerConnection> player = playersSnapshot[i];
        if (!player->done) {
            player->handleTextureReceived(textureName);
        }
    }
}

void ServerConnection::handleTextureAndGeometryReceived(
    const std::wstring& textureName) {
    auto it = find(m_pendingTextureRequests.begin(),
                   m_pendingTextureRequests.end(), textureName);
    if (it != m_pendingTextureRequests.end()) {
        m_pendingTextureRequests.erase(it);
    }
    std::vector<std::shared_ptr<PlayerConnection> > playersSnapshot;
    {
        std::lock_guard<std::mutex> lock(players_cs);
        playersSnapshot = players;
    }
    for (unsigned int i = 0; i < playersSnapshot.size(); i++) {
        std::shared_ptr<PlayerConnection> player = playersSnapshot[i];
        if (!player->done) {
            player->handleTextureAndGeometryReceived(textureName);
        }
    }
}

void ServerConnection::handleServerSettingsChanged(
    std::shared_ptr<ServerSettingsChangedPacket> packet) {
    Minecraft* pMinecraft = Minecraft::GetInstance();

    if (packet->action == ServerSettingsChangedPacket::HOST_DIFFICULTY) {
        for (unsigned int i = 0; i < pMinecraft->levels.size(); ++i) {
            if (pMinecraft->levels[i] != nullptr) {
                app.DebugPrintf(
                    "ClientConnection::handleServerSettingsChanged - "
                    "Difficulty = %d",
                    packet->data);
                pMinecraft->levels[i]->difficulty = packet->data;
            }
        }
    }
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
}

std::vector<std::shared_ptr<PlayerConnection> >*
ServerConnection::getPlayers() {
    return &players;
}