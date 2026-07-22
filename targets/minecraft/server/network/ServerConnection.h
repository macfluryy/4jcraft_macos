#pragma once
#include <format>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

class PendingConnection;
class PlayerConnection;
class MinecraftServer;
class Socket;
class ServerSettingsChangedPacket;

class ServerConnection {
    

private:
    
    
public:
    volatile bool running;

private:
    int connectionCounter;

private:
    std::mutex pending_cs;  
    std::vector<std::shared_ptr<PendingConnection> > pending;
    std::mutex players_cs;  
                            
    std::vector<std::shared_ptr<PlayerConnection> > players;

    
    
    std::vector<std::wstring> m_pendingTextureRequests;

public:
    MinecraftServer* server;

public:
    ServerConnection(
        MinecraftServer*
            server);  
    ~ServerConnection();
    void NewIncomingSocket(Socket* socket);  

    void removeSpamProtection(Socket* socket) {
    }  
    void addPlayerConnection(std::shared_ptr<PlayerConnection> uc);

private:
    void handleConnection(std::shared_ptr<PendingConnection> uc);

public:
    void stop();
    void tick();

    
    bool addPendingTextureRequest(const std::wstring& textureName);
    void handleTextureReceived(const std::wstring& textureName);
    void handleTextureAndGeometryReceived(const std::wstring& textureName);
    void handleServerSettingsChanged(
        std::shared_ptr<ServerSettingsChangedPacket> packet);
    std::vector<std::shared_ptr<PlayerConnection> >* getPlayers();
};
