#pragma once
#include <atomic>
#include <cstdint>
#include <deque>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "app/common/src/JavaEdition/JavaProtocolCodec.h"
#include "app/common/src/JavaEdition/JavaRawTcpClient.h"

enum class JavaConnectionState {
    Connecting,
    LoggingIn,
    Playing,
    Disconnected,
};

struct JavaTabListEntry {
    std::string uuid;
    std::wstring name;
    int ping = -1;
    std::string skinUrl;
};

enum class JavaConnectionEventType {
    StateChanged,
    Chat,
    SystemMessage,
    TabListReplaced,
    TabHeaderFooter,
    Disconnected,
    JoinGame,
    ChunkData,
    PlayerTeleport,
    SpawnMob,
    SpawnObject,
    SpawnPlayer,
    EntityDestroy,
    EntityMove,
    EntityTeleport,
    EntityLook,
    EntityHeadLook,
    EntityVelocity,
    EntityEquipment,
    EntityItemData,
    EntityMeta,
    Animation,
    EntityStatus,
    SoundEffect,
    Respawn,
    GameStateChange,
    PlayerAbilities,
    SetExperience,
    UpdateTime,
    BlockAction,
    Explosion,
    AttachEntity,
    UpdateSign,
    OpenSignEditor,
    CraftProgress,
    BlockChange,
    UpdateHealth,
    HeldItemChange,
    WindowItems,
    SetSlot,
    OpenWindow,
    CloseWindow,
    ConfirmTransaction,
    ScoreObjective,
    ScoreUpdate,
    ScoreDisplay,
    ScoreTeam,
    HudText,
};


struct JavaEnch {
    int16_t id = 0;
    int16_t lvl = 0;
};

struct JavaInvSlot {
    int16_t id = -1;
    uint8_t count = 0;
    int16_t damage = 0;
    std::wstring customName;
    std::vector<std::wstring> lore;
    std::vector<JavaEnch> ench;         
    std::vector<JavaEnch> storedEnch;   
    int32_t hideFlags = 0;              
};

struct JavaMetaEntry {
    uint8_t index = 0;
    uint8_t type = 0;
    int32_t intVal = 0;
    float floatVal = 0;
    std::wstring strVal;
};

struct JavaEntityInfo {
    int32_t id = 0;
    int entityType = 0;
    int rawJavaType = -1;
    int32_t x = 0, y = 0, z = 0;
    int8_t yaw = 0, pitch = 0;
    int8_t headYaw = 0;
    int8_t dx = 0, dy = 0, dz = 0;
    int16_t vx = 0, vy = 0, vz = 0;
    std::wstring playerName;
    std::string skinUrl;
    
    
    
    std::string uuid;
    int objectData = -1;
};

struct JavaConnectionEvent {
    JavaConnectionEventType type = JavaConnectionEventType::StateChanged;
    JavaConnectionState state = JavaConnectionState::Connecting;
    std::wstring text;
    std::vector<JavaTabListEntry> tabList;
    std::wstring tabHeader;
    std::wstring tabFooter;
    int chunkX = 0, chunkZ = 0;
    std::vector<uint8_t> chunkBuffer;
    double posX = 0, posY = 0, posZ = 0;
    float posYaw = 0, posPitch = 0;
    uint8_t posFlags = 0;
    JavaEntityInfo entity;
    std::vector<int32_t> destroyIds;
    int blockX = 0, blockY = 0, blockZ = 0;
    int blockId = 0;
    uint8_t blockMeta = 0;
    float healthVal = 20.0f;
    int foodVal = 20;
    float saturationVal = 5.0f;
    int8_t slotIdx = 0;
    int8_t animAction = 0;
    bool entityIsSelf = false;
    bool entityHasRot = false;
    uint8_t entityStatus = 0;
    std::string soundName;
    int soundX = 0, soundY = 0, soundZ = 0;
    float soundVolume = 1.0f;
    uint8_t soundPitch = 63;
    uint8_t respawnDifficulty = 1;
    uint8_t gameStateReason = 0;
    int gameStateGameMode = 0;
    uint8_t abilityFlags = 0;
    float flySpeed = 0.05f;
    float walkSpeed = 0.1f;
    float xpBar = 0.0f;
    int xpLevel = 0;
    int xpTotal = 0;
    int64_t timeAge = 0;
    int64_t timeDay = 0;
    uint8_t blockActionB0 = 0;
    uint8_t blockActionB1 = 0;
    double explodeX = 0, explodeY = 0, explodeZ = 0;
    float explodeRadius = 0.0f;
    std::vector<int8_t> explodeOffsets;
    float explodeMotX = 0, explodeMotY = 0, explodeMotZ = 0;
    int32_t vehicleId = -1;
    bool attachLeash = false;
    std::vector<std::wstring> signLines;
    int16_t equippedItemId = -1;
    int16_t equippedItemDamage = 0;
    uint8_t equippedItemCount = 1;
    int equippedSlot = 0;
    int joinGameMode = 0;
    int joinDimension = 0;
    int winId = 0;
    int slotIndex = 0;
    std::vector<JavaInvSlot> invSlots;
    int winType = 0;
    int winSlotCount = 0;
    bool winIsFurnace = false;
    int craftProperty = 0;
    int craftValue = 0;
    int winEntityId = 0;
    bool winAccepted = false;
    std::vector<JavaMetaEntry> metaEntries;
    std::wstring scoreObjective;
    std::wstring scoreDisplayName;
    std::wstring scoreOwner;
    int32_t scoreValue = 0;
    uint8_t scoreMode = 0;
    uint8_t scoreSlot = 0;
    std::wstring teamPrefix;
    std::wstring teamSuffix;
    std::vector<std::wstring> teamPlayers;
};

enum class JavaConnectionCommandType {
    SendChat,
    Disconnect,
    SendPlayerPosition,
    SendBlockDig,
    SendBlockPlace,
    SendHeldItem,
    SendAnimation,
    SendUseEntity,
    SendEntityAction,
    SendClientSettings,
    SendBrand,
    SendCreativeSlot,
    SendWindowClick,
    SendCloseWindow,
    SendClientStatus,
};

struct JavaConnectionCommand {
    JavaConnectionCommandType type = JavaConnectionCommandType::SendChat;
    std::string chatUtf8;
    double posX = 0, posY = 0, posZ = 0;
    float posYaw = 0, posPitch = 0;
    bool onGround = false;
    int blockX = 0, blockY = 0, blockZ = 0;
    uint8_t digStatus = 0;
    uint8_t face = 0;
    uint8_t cursorX = 8, cursorY = 8, cursorZ = 8;
    int16_t heldItemId = -1;
    uint8_t heldItemCount = 0;
    int16_t heldItemDamage = 0;
    int16_t hotbarSlot = 0;
    int32_t targetEntityId = 0;
    uint8_t useEntityAction = 1;
    int32_t playerEntityId = 0;
    int32_t actionId = 0;
    int32_t actionParam = 0;
    int winId = 0;
    int16_t winSlot = 0;
    uint8_t winButton = 0;
    int16_t winActionNum = 0;
    uint8_t winMode = 0;
};

class JavaConnection {
public:
    JavaConnection(std::string host, uint16_t port, std::string nicknameUtf8);
    ~JavaConnection();

    void start();

    void sendChat(const std::string& utf8);

    void sendPlayerPosition(double x, double y, double z,
                            float yaw, float pitch, bool onGround);

    void sendBlockDig(int x, int y, int z, uint8_t status, uint8_t face);
    void sendBlockPlace(int x, int y, int z, uint8_t face,
                        int16_t itemId, uint8_t itemCount, int16_t itemDamage,
                        uint8_t cursorX, uint8_t cursorY, uint8_t cursorZ);
    void sendHeldItem(int16_t slot);
    void sendCreativeSlot(int16_t slot, int16_t itemId, uint8_t count,
                          int16_t damage);
    void sendWindowClick(int windowId, int16_t slot, uint8_t button,
                         int16_t actionNum, uint8_t mode,
                         int16_t itemId, uint8_t count, int16_t damage);
    void sendCloseWindow(int windowId);
    void sendClientStatus(int action);
    void sendAnimation();
    void sendUseEntity(int32_t targetId, uint8_t action);
    void sendEntityAction(int32_t playerId, int32_t action, int32_t param);
    void sendClientSettings();

    void sendBrand();

    void requestDisconnect();

    void drainEvents(std::vector<JavaConnectionEvent>& out);

    JavaConnectionState state() const { return m_observedState.load(); }
    bool isFinished() const { return m_workerFinished.load(); }

private:
    void runWorker();

    bool pumpRecv(int timeoutMs);

    JavaProtocolCodec::DecodeStatus nextFrame(JavaProtocolCodec::DecodedFrame& outFrame);

    bool sendFrame(int32_t packetId, const std::vector<uint8_t>& body);

    bool handleLoginFrame(const JavaProtocolCodec::DecodedFrame& frame);
    bool handlePlayFrame(const JavaProtocolCodec::DecodedFrame& frame);

    bool drainCommands();

    void pushEvent(JavaConnectionEvent event);

    void setState(JavaConnectionState s);

    const std::string m_host;
    const uint16_t m_port;
    const std::string m_nicknameUtf8;

    JavaRawTcpClient m_tcp;
    JavaProtocolCodec m_codec;
    std::vector<uint8_t> m_recvBuf;
    std::vector<JavaTabListEntry> m_tabList;

    double m_playerX = 0.0, m_playerY = 0.0, m_playerZ = 0.0;
    float m_playerYaw = 0.0f, m_playerPitch = 0.0f;
    bool m_havePlayerPos = false;

    int32_t m_dimension = 0;

    bool m_forwardingActive = false;
    bool m_sawBungeeKick = false;

    std::mutex m_eventMutex;
    std::deque<JavaConnectionEvent> m_eventQueue;
    std::mutex m_commandMutex;
    std::deque<JavaConnectionCommand> m_commandQueue;

    std::atomic<JavaConnectionState> m_observedState{JavaConnectionState::Connecting};
    std::atomic<bool> m_workerFinished{false};
    std::atomic<bool> m_shutdownRequested{false};

    std::thread m_worker;
    bool m_started = false;

    int32_t m_javaPlayerEntityId = -1;
};