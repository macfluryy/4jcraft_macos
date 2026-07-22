#pragma once
#include <stdint.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "platform/PlatformTypes.h"
#include "java/JavaIntHash.h"
#include "minecraft/commands/CommandsEnum.h"
#include "minecraft/network/packet/DisconnectPacket.h"
#include "minecraft/network/packet/PacketListener.h"
#include "minecraft/server/ConsoleInputSource.h"

class MinecraftServer;
class Connection;
class ServerPlayer;
class INetworkPlayer;
class Packet;
class Random;
class ClientInformationPacket;

class PlayerConnection : public PacketListener, public ConsoleInputSource {
    

public:
    Connection* connection;
    bool done;
    std::mutex done_cs;

    
    PlayerUID m_offlineXUID, m_onlineXUID;
    bool m_friendsOnlyUGC;

private:
    MinecraftServer* server;
    std::shared_ptr<ServerPlayer> player;
    int tickCount;
    int aboveGroundTickCount;

    bool didTick;
    int lastKeepAliveId;
    int64_t lastKeepAliveTime;
    static Random random;
    int64_t lastKeepAliveTick;
    int chatSpamTickCount;
    int dropSpamTickCount;

    bool m_bHasClientTickedOnce;

public:
    PlayerConnection(MinecraftServer* server, Connection* connection,
                     std::shared_ptr<ServerPlayer> player);
    ~PlayerConnection();
    void tick();
    void disconnect(DisconnectPacket::eDisconnectReason reason);

private:
    double xLastOk, yLastOk, zLastOk;
    bool synched;

public:
    virtual void handlePlayerInput(std::shared_ptr<PlayerInputPacket> packet);
    virtual void handleMovePlayer(std::shared_ptr<MovePlayerPacket> packet);
    void teleport(double x, double y, double z, float yRot, float xRot,
                  bool sendPacket = true);  
    virtual void handlePlayerAction(std::shared_ptr<PlayerActionPacket> packet);
    virtual void handleUseItem(std::shared_ptr<UseItemPacket> packet);
    virtual void onDisconnect(DisconnectPacket::eDisconnectReason reason,
                              void* reasonObjects);
    virtual void onUnhandledPacket(std::shared_ptr<Packet> packet);
    void send(std::shared_ptr<Packet> packet);
    void queueSend(std::shared_ptr<Packet> packet);  
    virtual void handleSetCarriedItem(
        std::shared_ptr<SetCarriedItemPacket> packet);
    virtual void handleChat(std::shared_ptr<ChatPacket> packet);

private:
    void handleCommand(const std::wstring& message);
    
    
    EGameCommand parseCommandName(const std::wstring& cmdName);

public:
    virtual void handleAnimate(std::shared_ptr<AnimatePacket> packet);
    virtual void handlePlayerCommand(
        std::shared_ptr<PlayerCommandPacket> packet);
    virtual void handleDisconnect(std::shared_ptr<DisconnectPacket> packet);
    int countDelayedPackets();
    virtual void info(const std::wstring& string);
    virtual void warn(const std::wstring& string);
    virtual std::wstring getConsoleName();
    virtual void handleInteract(std::shared_ptr<InteractPacket> packet);
    bool canHandleAsyncPackets();
    virtual void handleClientCommand(
        std::shared_ptr<ClientCommandPacket> packet);
    virtual void handleRespawn(std::shared_ptr<RespawnPacket> packet);
    virtual void handleContainerClose(
        std::shared_ptr<ContainerClosePacket> packet);

private:
    std::unordered_map<int, short, IntKeyHash, IntKeyEq> expectedAcks;

public:
    
#ifndef _CONTENT_PACKAGE
    virtual void handleContainerSetSlot(
        std::shared_ptr<ContainerSetSlotPacket> packet);
#endif
    virtual void handleContainerClick(
        std::shared_ptr<ContainerClickPacket> packet);
    virtual void handleContainerButtonClick(
        std::shared_ptr<ContainerButtonClickPacket> packet);
    virtual void handleSetCreativeModeSlot(
        std::shared_ptr<SetCreativeModeSlotPacket> packet);
    virtual void handleContainerAck(std::shared_ptr<ContainerAckPacket> packet);
    virtual void handleSignUpdate(std::shared_ptr<SignUpdatePacket> packet);
    virtual void handleKeepAlive(std::shared_ptr<KeepAlivePacket> packet);
    virtual void handlePlayerInfo(
        std::shared_ptr<PlayerInfoPacket> packet);  
    virtual bool isServerPacketListener();
    virtual void handlePlayerAbilities(
        std::shared_ptr<PlayerAbilitiesPacket> playerAbilitiesPacket);
    virtual void handleCustomPayload(
        std::shared_ptr<CustomPayloadPacket> customPayloadPacket);
    
    
    
    virtual void handleClientInformation(
        std::shared_ptr<ClientInformationPacket> packet);
    virtual bool isDisconnected();

    
    virtual void handleCraftItem(std::shared_ptr<CraftItemPacket> packet);
    virtual void handleTradeItem(std::shared_ptr<TradeItemPacket> packet);
    virtual void handleDebugOptions(std::shared_ptr<DebugOptionsPacket> packet);
    virtual void handleTexture(std::shared_ptr<TexturePacket> packet);
    virtual void handleTextureAndGeometry(
        std::shared_ptr<TextureAndGeometryPacket> packet);
    virtual void handleTextureChange(
        std::shared_ptr<TextureChangePacket> packet);
    virtual void handleTextureAndGeometryChange(
        std::shared_ptr<TextureAndGeometryChangePacket> packet);
    virtual void handleServerSettingsChanged(
        std::shared_ptr<ServerSettingsChangedPacket> packet);
    virtual void handleKickPlayer(std::shared_ptr<KickPlayerPacket> packet);
    virtual void handleGameCommand(std::shared_ptr<GameCommandPacket> packet);

    INetworkPlayer* getNetworkPlayer();
    bool isLocal();
    bool isGuest();

    
    void setPlayer(std::shared_ptr<ServerPlayer> player) {
        this->player = player;
    }
    std::shared_ptr<ServerPlayer> getPlayer() { return player; }

    
    void closeOnTick() { m_bCloseOnTick = true; }

    
    
    void handleTextureReceived(const std::wstring& textureName);
    void handleTextureAndGeometryReceived(const std::wstring& textureName);

    void setShowOnMaps(bool bVal);

    void setWasKicked() { m_bWasKicked = true; }
    bool getWasKicked() { return m_bWasKicked; }

    
    bool hasClientTickedOnce() { return m_bHasClientTickedOnce; }

private:
    bool m_bCloseOnTick;
    std::vector<std::wstring> m_texturesRequested;

    bool m_bWasKicked;

    
    
    
    
    
    
    
    std::atomic<int> m_pendingClientViewDistance{-1};
};