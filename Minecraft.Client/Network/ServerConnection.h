#pragma once
class PendingConnection;
class PlayerConnection;
class MinecraftServer;
class Socket;
class ServerSettingsChangedPacket;

class ServerConnection {
    //    public static Logger logger = Logger.getLogger("Minecraft");

private:
    //	ServerSocket serverSocket;
    //    private Thread listenThread;
public:
    volatile bool running;

private:
    int connectionCounter;

private:
    CRITICAL_SECTION pending_cs;  // 4J added
    std::vector<std::shared_ptr<PendingConnection> > pending;
    std::vector<std::shared_ptr<PlayerConnection> > players;

    // 4J - When the server requests a texture, it should add it to here while
    // we are waiting for it
    std::vector<std::wstring> m_pendingTextureRequests;

public:
    MinecraftServer* server;

public:
    ServerConnection(
        MinecraftServer*
            server);  // 4J - removed params InetAddress address, int port);
    ~ServerConnection();
    void NewIncomingSocket(Socket* socket);  // 4J - added

    void removeSpamProtection(Socket* socket) {
    }  // 4J Stu - Not implemented as not required
    void addPlayerConnection(std::shared_ptr<PlayerConnection> uc);

private:
    void handleConnection(std::shared_ptr<PendingConnection> uc);

public:
    void stop();
    void tick();

    // 4J Added
    bool addPendingTextureRequest(const std::wstring& textureName);
    void handleTextureReceived(const std::wstring& textureName);
    void handleTextureAndGeometryReceived(const std::wstring& textureName);
    void handleServerSettingsChanged(
        std::shared_ptr<ServerSettingsChangedPacket> packet);
    std::vector<std::shared_ptr<PlayerConnection> >* getPlayers();
};
