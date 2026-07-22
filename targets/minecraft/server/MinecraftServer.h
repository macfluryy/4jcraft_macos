#pragma once
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "ConsoleInputSource.h"
#include "platform/C4JThread.h"
#include "util/Timer.h"
#include "minecraft/SharedConstants.h"
#include "minecraft/world/level/chunk/ChunkSource.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/FileHeader.h"

class ServerConnection;
class Settings;
class PlayerList;
class EntityTracker;
class ConsoleInput;
class ConsoleCommands;
class LevelStorageSource;
class INetworkPlayer;
class LevelRuleset;
class LevelType;
class ProgressRenderer;
class CommandDispatcher;
class LevelGenerationOptions;
class ServerLevel;
class File;
class Level;
class Player;
class Pos;










#define MINECRAFT_SERVER_SLOW_QUEUE_DELAY 25

typedef struct _LoadSaveDataThreadParam {
    void* data;
    int64_t fileSize;
    const std::wstring saveName;
    _LoadSaveDataThreadParam(void* data, int64_t filesize,
                             const std::wstring& saveName)
        : data(data), fileSize(filesize), saveName(saveName) {}
} LoadSaveDataThreadParam;

typedef struct _NetworkGameInitData {
    int64_t seed;
    LoadSaveDataThreadParam* saveData;
    std::uint32_t settings;
    LevelGenerationOptions* levelGen;
    std::uint32_t texturePackId;
    bool findSeed;
    unsigned int xzSize;
    unsigned char hellScale;
    ESavePlatform savePlatform;

    _NetworkGameInitData() {
        seed = 0;
        saveData = nullptr;
        settings = 0;
        levelGen = nullptr;
        texturePackId = 0;
        findSeed = false;
        xzSize = LEVEL_LEGACY_WIDTH;
        hellScale = HELL_LEVEL_LEGACY_SCALE;
        savePlatform = SAVE_FILE_PLATFORM_LOCAL;
    }
} NetworkGameInitData;




class MinecraftServer : public ConsoleInputSource {
public:
    static const std::wstring VERSION;
    static const int TICK_STATS_SPAN = SharedConstants::TICKS_PER_SECOND * 5;

    
    static std::unordered_map<std::wstring, int> ironTimers;

private:
    static const int DEFAULT_MINECRAFT_PORT = 25565;
    static const int MS_PER_TICK = 1000 / SharedConstants::TICKS_PER_SECOND;

    
    
    
public:
    ServerConnection* connection;
    Settings* settings;
    std::vector<ServerLevel*> levels;

private:
    PlayerList* players;

    
    
    
private:
    ConsoleCommands* commands;
    bool running;
    bool m_bLoaded;

public:
    bool stopped;
    int tickCount;

    
    static const int TPS_SAMPLE_COUNT = 100;
    int64_t m_tickTimesNs[TPS_SAMPLE_COUNT] = {0};
    int m_tickTimesIndex = 0;
    int m_tickTimesFilled = 0;

public:
    std::wstring progressStatus;
    int progress;

private:
    
    
    CommandDispatcher* commandDispatcher;
    std::vector<ConsoleInput*>
        consoleInput;  
public:
    bool onlineMode;
    bool animals;
    bool npcs;
    bool pvp;
    bool allowFlight;
    std::wstring motd;
    int maxBuildHeight;
    int playerIdleTimeout;
    bool forceGameType;

private:
    
    

public:
    
    
    
    std::uint32_t m_ugcPlayersVersion;

    
    
    std::uint32_t m_texturePackId;

public:
    MinecraftServer();
    ~MinecraftServer();

private:
    
    bool initServer(int64_t seed, NetworkGameInitData* initData,
                    std::uint32_t initSettings, bool findSeed);
    void postProcessTerminate(ProgressRenderer* mcprogress);
    bool loadLevel(LevelStorageSource* storageSource, const std::wstring& name,
                   int64_t levelSeed, LevelType* pLevelType,
                   NetworkGameInitData* initData);
    void setProgress(const std::wstring& status, int progress);
    void endProgress();
    void saveAllChunks();
    void saveGameRules();
    void stopServer(bool didInit);
#if defined(_LARGE_WORLDS)
    void overwriteBordersForNewWorldSize(ServerLevel* level);
    void overwriteHellBordersForNewWorldSize(ServerLevel* level,
                                             int oldHellSize);

#endif
public:
    void setMaxBuildHeight(int maxBuildHeight);
    int getMaxBuildHeight();
    PlayerList* getPlayers();
    void setPlayers(PlayerList* players);
    ServerConnection* getConnection();
    bool isAnimals();
    void setAnimals(bool animals);
    bool isNpcsEnabled();
    void setNpcsEnabled(bool npcs);
    bool isPvpAllowed();
    void setPvpAllowed(bool pvp);
    bool isFlightAllowed();
    void setFlightAllowed(bool allowFlight);
    bool isCommandBlockEnabled();
    bool isNetherEnabled();
    bool isHardcore();
    int getOperatorUserPermissionLevel();
    CommandDispatcher* getCommandDispatcher();
    Pos* getCommandSenderWorldPosition();
    Level* getCommandSenderWorld();
    int getSpawnProtectionRadius();
    bool isUnderSpawnProtection(Level* level, int x, int y, int z,
                                std::shared_ptr<Player> player);
    void setForceGameType(bool forceGameType);
    bool getForceGameType();
    static int64_t getCurrentTimeMillis();
    int getPlayerIdleTimeout();
    void setPlayerIdleTimeout(int playerIdleTimeout);

public:
    void halt();
    void run(int64_t seed, void* lpParameter);

    void broadcastStartSavingPacket();
    void broadcastStopSavingPacket();

private:
    void tick();

public:
    void handleConsoleInput(const std::wstring& msg,
                            ConsoleInputSource* source);
    void handleConsoleInputs();
    
    static void main(int64_t seed, void* lpParameter);
    static void HaltServer(bool bPrimaryPlayerSignedOut = false);

    File* getFile(const std::wstring& name);
    void info(const std::wstring& string);
    void warn(const std::wstring& string);
    std::wstring getConsoleName();
    ServerLevel* getLevel(int dimension);
    void setLevel(int dimension, ServerLevel* level);         
    static MinecraftServer* getInstance() { return server; }  
    static bool serverHalted() { return s_bServerHalted; }
    static bool saveOnExitAnswered() { return s_bSaveOnExitAnswered; }
    static void resetFlags() {
        s_bServerHalted = false;
        s_bSaveOnExitAnswered = false;
    }

    bool flagEntitiesToBeRemoved(unsigned int* flags);  
private:
    
    static MinecraftServer* server;

    static bool setTimeOfDayAtEndOfTick;
    static int64_t setTimeOfDay;
    static bool setTimeAtEndOfTick;
    static int64_t setTime;

    static bool
        m_bPrimaryPlayerSignedOut;  
                                    
                                    
                                    
    static bool s_bServerHalted;  
                                  
    static bool s_bSaveOnExitAnswered;  
                                        

    
    
    static int runPostUpdate(void* lpParam);
    C4JThread* m_postUpdateThread;
    bool m_postUpdateTerminate;
    class postProcessRequest {
    public:
        int x, z;
        ChunkSource* chunkSource;
        postProcessRequest(int x, int z, ChunkSource* chunkSource)
            : x(x), z(z), chunkSource(chunkSource) {}
    };
    std::vector<postProcessRequest> m_postProcessRequests;
    std::mutex m_postProcessCS;

public:
    void addPostProcessRequest(ChunkSource* chunkSource, int x, int z);

public:
    static PlayerList* getPlayerList() {
        if (server != nullptr)
            return server->players;
        else
            return nullptr;
    }
    static void SetTimeOfDay(int64_t time) {
        setTimeOfDayAtEndOfTick = true;
        setTimeOfDay = time;
    }
    static void SetTime(int64_t time) {
        setTimeAtEndOfTick = true;
        setTime = time;
    }

    C4JThread::Event* m_serverPausedEvent;

private:
    
    bool m_isServerPaused;

    
    
#if defined(_ACK_CHUNK_SEND_THROTTLING)
    static bool s_hasSentEnoughPackets;
    static int64_t s_tickStartTime;
    static std::vector<INetworkPlayer*> s_sentTo;
    static const int MAX_TICK_TIME_FOR_PACKET_SENDS = 35;
#else
    static int s_slowQueuePlayerIndex;
    static time_util::time_point s_slowQueueLastTime;
    static bool s_slowQueuePacketSent;
#endif

    bool IsServerPaused() { return m_isServerPaused; }

private:
    
    bool m_saveOnExit;
    bool m_suspending;

public:
    static bool chunkPacketManagement_CanSendTo(INetworkPlayer* player);
    static void chunkPacketManagement_DidSendTo(INetworkPlayer* player);
#if !defined(_ACK_CHUNK_SEND_THROTTLING)
    static void cycleSlowQueueIndex();
#endif

    void chunkPacketManagement_PreTick();
    void chunkPacketManagement_PostTick();

    void setSaveOnExit(bool save) {
        m_saveOnExit = save;
        s_bSaveOnExitAnswered = true;
    }

    
    
    
    
    
    
    void forceShutdownSave();
    void Suspend();
    bool IsSuspending();

    
    
};