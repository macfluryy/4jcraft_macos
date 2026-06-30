#pragma once
#include <memory>
#include <string>

#include "minecraft/network/packet/DisconnectPacket.h"
#include "minecraft/network/packet/PacketListener.h"

class MinecraftServer;
class Socket;
class LoginPacket;
class Connection;
class Random;
class Packet;

class PendingConnection : public PacketListener {
private:
    static const int FAKE_LAG = 0;
    // 4J - 60 seconds at 20 TPS. Was 10 minutes which was effectively no DoS
    // protection: a half-open TCP connection that never sends PreLoginPacket
    // would sit in the pending list for that whole window.
    static const int MAX_TICKS_BEFORE_LOGIN = 20 * 60;

    //    public static Logger logger = Logger.getLogger("Minecraft");
    static Random* random;

public:
    Connection* connection;

public:
    bool done;

private:
    MinecraftServer* server;
    int _tick;
    std::wstring name;
    std::shared_ptr<LoginPacket> acceptedLogin;
    std::wstring loginKey;

public:
    PendingConnection(MinecraftServer* server, Socket* socket,
                      const std::wstring& id);
    ~PendingConnection();
    void tick();
    void disconnect(DisconnectPacket::eDisconnectReason reason);
    virtual void handlePreLogin(std::shared_ptr<PreLoginPacket> packet);
    virtual void handleLogin(std::shared_ptr<LoginPacket> packet);
    virtual void handleAcceptedLogin(std::shared_ptr<LoginPacket> packet);
    virtual void onDisconnect(DisconnectPacket::eDisconnectReason reason,
                              void* reasonObjects);
    virtual void handleGetInfo(std::shared_ptr<GetInfoPacket> packet);
    virtual void handleKeepAlive(std::shared_ptr<KeepAlivePacket> packet);
    virtual void onUnhandledPacket(std::shared_ptr<Packet> packet);
    void send(std::shared_ptr<Packet> packet);
    std::wstring getName();
    virtual bool isServerPacketListener();
    virtual bool isDisconnected();

private:
    void sendPreLoginResponse();
};