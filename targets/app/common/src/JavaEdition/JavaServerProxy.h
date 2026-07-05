#pragma once
#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "app/common/src/JavaEdition/JavaConnection.h"

class JavaServerProxy {
public:
    JavaServerProxy();
    ~JavaServerProxy();

    JavaServerProxy(const JavaServerProxy&) = delete;
    JavaServerProxy& operator=(const JavaServerProxy&) = delete;

    int startListening();

    void startWorker(const std::string& javaHost, uint16_t javaPort,
                     const std::string& nicknameUtf8,
                     const std::wstring& displayName);

    void requestStop();
    bool isStopped() const { return m_stopped.load(); }

    int port() const { return m_listenPort; }

private:
    void runWorker();

    bool readAll(uint8_t* buf, size_t n);
    bool writeAll(const uint8_t* buf, size_t n);
    bool readU8(uint8_t& v);
    bool readU16(uint16_t& v);
    bool readI16(int16_t& v);
    bool readI32(int32_t& v);
    bool readI64(int64_t& v);
    bool readF32(float& v);
    bool readF64(double& v);
    bool readUtf16(std::wstring& out, int maxChars);
    bool sendPreLoginPacket(const std::wstring& username);
    bool sendLoginPacket(const std::wstring& username);
    bool sendBlockRegionUpdatePacket(int32_t chunkX, int32_t chunkZ,
                                     const std::vector<uint8_t>& lceBuf);
    bool sendChatPacket(const std::wstring& text);
    bool sendMovePlayerPosRot(double x, double y, double z, double yView,
                              float yRot, float xRot,
                              bool onGround, bool isFlying);
    bool sendAddMobPacket(int id, uint8_t type, int x, int y, int z,
                          int8_t yaw, int8_t pitch, int8_t headYaw);
    bool sendAddEntityPacket(int id, uint8_t type, int x, int y, int z,
                             int8_t yawByte, int8_t pitchByte, int32_t data,
                             int16_t vx, int16_t vy, int16_t vz);
    bool sendRemoveEntitiesPacket(const std::vector<int>& ids);
    bool sendTeleportEntityPacket(int id, int x, int y, int z,
                                  int8_t yaw, int8_t pitch);
    bool sendMoveEntityPosRot(int id, int8_t dx, int8_t dy, int8_t dz,
                              int8_t yaw, int8_t pitch);
    bool sendMoveEntityRot(int id, int8_t yaw, int8_t pitch);
    bool sendRotateHeadPacket(int id, int8_t yHeadRot);
    bool sendSetEntityMotionPacket(int id, int16_t vx, int16_t vy, int16_t vz);
    bool sendAnimatePacket(int entityId, uint8_t animation);
    bool sendEntityEventPacket(int entityId, uint8_t eventId);
    bool sendLevelSoundPacket(int soundId, double x, double y, double z,
                              float volume, float pitch);
    bool sendSetTimePacket(int64_t gameTime, int64_t dayTime);
    bool sendSetExperiencePacket(float progress, int16_t level, int16_t total);
    bool sendGameEventPacket(uint8_t event, uint8_t param);
    bool sendRespawnPacket(int dimension, int gameMode, uint8_t difficulty);
    bool sendTileEventPacket(int x, int y, int z, uint8_t b0, uint8_t b1,
                             int16_t lceBlock);
    bool sendExplodePacket(double x, double y, double z, float radius,
                           const std::vector<int8_t>& offsets,
                           float kbX, float kbY, float kbZ);
    bool sendSetEntityLinkPacket(int sourceId, int destId, uint8_t linkType);
    bool sendSignUpdatePacket(int x, int y, int z,
                              const std::vector<std::wstring>& lines);
    bool sendTileEditorOpenPacket(uint8_t editorType, int x, int y, int z);
    bool sendContainerSetDataPacket(uint8_t windowId, int16_t property,
                                    int16_t value);
    bool sendSetEquippedItemPacket(int entityId, int slot, int16_t itemId,
                                   uint8_t count, int16_t damage);
    bool sendAddPlayerPacket(int id, const std::wstring& name,
                             int x, int y, int z,
                             int8_t yaw, int8_t pitch, int8_t headYaw,
                             int16_t carriedItem);

    bool sendTileUpdatePacket(int x, int y, int z, uint8_t block, uint8_t meta);
    bool sendSetHealthPacket(float health, int16_t food, float saturation);
    bool sendSpawnPositionPacket(int x, int y, int z);
    bool sendPlayerAbilitiesPacket(bool invulnerable, bool flying,
                                   bool canFly, bool instabuild);
    void appendLceItem(std::vector<uint8_t>& buf, const JavaInvSlot& item);
    bool sendContainerSetContentPacket(uint8_t windowId,
                                       const std::vector<JavaInvSlot>& slots);
    bool sendContainerSetSlotPacket(uint8_t windowId, int16_t slot,
                                    const JavaInvSlot& item);
    bool sendContainerOpenPacket(uint8_t windowId, uint8_t type,
                                 uint8_t slotCount, int32_t entityId,
                                 const std::wstring& title);
    bool sendSetItemDataPacket(int entityId, const JavaInvSlot& item);
    bool sendContainerClosePacket(uint8_t windowId);
    bool sendContainerAckPacket(uint8_t windowId, int16_t action, bool accepted);
    bool sendDisconnectPacket(int reason);
    bool sendDisconnectWithTextPacket(const std::wstring& text);
    void sendGracefulDisconnect(const std::wstring& reason);
    bool runSpawnHandshake(const std::wstring& username,
                           std::vector<JavaConnectionEvent>& pendingChunks);
    bool handleLceMovePlayer(uint8_t pktId);
    bool handleLceChat();
    bool consumeLceBody(uint8_t pktId, bool& consumed);
    bool skipLceItem();
    bool skipNBytes(size_t n);

    int allocateLceEntityId(int32_t javaId);
    int lookupLceEntityId(int32_t javaId);
    void releaseJavaEntityId(int32_t javaId);
    void emitFlatFallback();

    struct LceMetaItem {
        uint8_t type = 0;
        uint8_t index = 0;
        int32_t i = 0;
        float f = 0;
        std::wstring s;
        bool operator==(const LceMetaItem& o) const {
            return type == o.type && index == o.index && i == o.i &&
                   f == o.f && s == o.s;
        }
    };
    void handleEntityMeta(const JavaConnectionEvent& ev);
    bool sendEntityDataPacket(int lceId,
                              const std::vector<LceMetaItem>& items);
    struct JavaEntityKind {
        int javaType = -1;
        bool spawned = false;
    };
    std::unordered_map<int32_t, JavaEntityKind> m_javaEntityKind;
    std::unordered_map<int32_t, std::unordered_map<uint8_t, LceMetaItem>>
        m_metaCache;

    void handleScoreEvent(const JavaConnectionEvent& ev);
    bool sendSetObjectivePacket(const std::wstring& name,
                                const std::wstring& displayName,
                                uint8_t method);
    bool sendSetScorePacket(const std::wstring& owner, uint8_t method,
                            const std::wstring& objective, int32_t value);
    bool sendSetDisplayObjectivePacket(uint8_t slot,
                                       const std::wstring& name);
    std::unordered_map<std::wstring, std::wstring> m_scoreObjectives;
    std::unordered_map<std::wstring,
                       std::unordered_map<std::wstring, int32_t>>
        m_scoreValues;
    std::wstring m_sidebarObjective;
    bool m_sidebarSet = false;

    // ---- Team translation (Java 0x3E -> LCE 209). Same incremental-state
    // pattern: duplicate/idempotent packets are suppressed.
    struct TeamState {
        std::wstring displayName;  // translated
        std::wstring prefix;       // translated
        std::wstring suffix;       // translated
        int options = 0;
        std::unordered_set<std::wstring> players;
    };
    void handleTeamEvent(const JavaConnectionEvent& ev);
    bool sendSetPlayerTeamPacket(const std::wstring& name, uint8_t method,
                                 const TeamState* info,
                                 const std::vector<std::wstring>& players);
    std::unordered_map<std::wstring, TeamState> m_teams;

    int m_listenFd = -1;
    int m_listenPort = -1;
    int m_clientFd = -1;
    std::mutex m_sendMutex;
    std::mutex m_skinMutex;
    std::unordered_map<std::string, std::vector<uint8_t>> m_skinCache;
    std::vector<std::thread> m_skinThreads;

    void requestSkinDownload(int lceEntityId, const std::string& url);
    void deliverSkin(int lceEntityId, const std::wstring& texName,
                     const std::vector<uint8_t>& png);
    bool sendTexturePacket(const std::wstring& name,
                           const std::vector<uint8_t>& png);
    bool sendTextureChangePacket(int lceEntityId, const std::wstring& name);
    static std::wstring skinTexNameForUrl(const std::string& url);
    static std::vector<uint8_t> httpGetSkin(const std::string& url);

    std::string m_javaHost;
    uint16_t m_javaPort = 0;
    std::string m_nicknameUtf8;
    std::wstring m_displayName;
    std::wstring m_pendingDisconnectReason;
    std::unique_ptr<JavaConnection> m_java;
    std::thread m_worker;
    std::atomic<bool> m_stopRequested{false};
    std::atomic<bool> m_stopped{false};
    std::vector<std::pair<int32_t, int>> m_entityMap;
    int m_nextLceEntityId = 1000;

    struct EntityState {
        int32_t x = 0, y = 0, z = 0;
        int8_t yaw = 0, pitch = 0;
        int8_t headYaw = 0;
    };
    std::unordered_map<int32_t, EntityState> m_entityState;
    std::atomic<bool> m_loginSent{false};

    int m_javaGameMode = 0;
    int m_javaDimension = 0;
    int m_openWindowId = -1;
    bool m_openWindowIsFurnace = false;
    bool m_haveSpawn = false;
    double m_spawnX = 0.0, m_spawnY = 64.0, m_spawnZ = 0.0;
    float m_spawnYaw = 0.0f, m_spawnPitch = 0.0f;

    double m_lastFeetX = 0.0;
    double m_lastFeetY = 0.0;
    double m_lastFeetZ = 0.0;
    float m_lastYaw = 0.0f;
    float m_lastPitch = 0.0f;
    bool m_lastOnGround = true;
    bool m_haveLastPos = false;

    std::atomic<int> m_chunksSent{0};
    std::vector<std::wstring> m_chatBacklog;
    static constexpr int kChunksBeforeChat = 8;

    bool m_realChunkSeen = false;
    bool m_flatFallbackSent = false;
    std::chrono::steady_clock::time_point m_spawnDoneTime;
    static constexpr int kFlatFallbackMs = 3000;
};

extern JavaServerProxy* g_activeJavaProxy;