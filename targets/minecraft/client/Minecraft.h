#pragma once
#include <stdint.h>

#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <unordered_map>

#include "platform/PlatformTypes.h"
#include "platform/C4JThread.h"
#include "java/File.h"
#include "minecraft/client/resources/ResourceLocation.h"
#include "minecraft/network/packet/DisconnectPacket.h"

class Timer;
class MultiPlayerLevel;
class LevelRenderer;
class MultiplayerLocalPlayer;
class Player;
class Mob;
class ParticleEngine;
class User;
class Canvas;
class Textures;
class Font;
class Screen;
class ProgressRenderer;
class GameRenderer;
class BackgroundDownloader;
class HumanoidModel;
class HitResult;
class Options;
class SoundEngine;
class MinecraftApplet;
class MouseHandler;
class TexturePackRepository;
class File;
class LevelStorageSource;
class StatsCounter;
class Component;
class Entity;
class AchievementPopup;
class WaterTexture;
class LavaTexture;
class Gui;
class ClientConnection;
class ConsoleSaveFile;
class ItemInHandRenderer;
class LevelSettings;
class ColourTable;
class MultiPlayerGameMode;
class LivingEntity;
class Level;
class ResourceLocation;


class Minecraft {
private:
    enum OS { linux_os, solaris, windows, macos, unknown, xbox };

    static ResourceLocation DEFAULT_FONT_LOCATION;
    static ResourceLocation ALT_FONT_LOCATION;

public:
    
    
    
    struct JavaTabEntry {
        std::wstring name;
        int ping = 0;
    };
    std::unordered_map<std::string, JavaTabEntry> m_javaTabList;
    std::vector<std::string> m_javaTabOrder;

    void javaTabAdd(const std::string& uuid, const std::wstring& name, int ping);
    void javaTabRemove(const std::string& uuid);
    void javaTabClear();
    
    bool isJavaTabListed(const std::string& uuid) const {
        return !uuid.empty() && m_javaTabList.count(uuid) != 0;
    }

    static const std::wstring VERSION_STRING;
    Minecraft(Component* mouseComponent, Canvas* parent,
              MinecraftApplet* minecraftApplet, int width, int height,
              bool fullscreen);
    void init();

    
    
    

private:
    static Minecraft* m_instance;

public:
    MultiPlayerGameMode* gameMode;

private:
    bool fullscreen;
    bool hasCrashed;

    C4JThread::EventQueue* levelTickEventQueue;

    static void levelTickUpdateFunc(void* pParam);
    static void levelTickThreadInitFunc();

public:
    int width, height;
    int width_phys, height_phys;  
    

private:
    Timer* timer;
    bool reloadTextures;

public:
    Level* oldLevel;  
                      
    
    
public:
    MultiPlayerLevel* level;
    LevelRenderer* levelRenderer;
    std::shared_ptr<MultiplayerLocalPlayer> player;

    int m_serverViewDistanceChunks = 0;

    std::vector<MultiPlayerLevel*> levels;

    std::shared_ptr<MultiplayerLocalPlayer> localplayers[XUSER_MAX_COUNT];
    MultiPlayerGameMode* localgameModes[XUSER_MAX_COUNT];
    int localPlayerIdx;
    ItemInHandRenderer* localitemInHandRenderers[XUSER_MAX_COUNT];
    
    unsigned int uiDebugOptionsA[XUSER_MAX_COUNT];

    
    bool m_connectionFailed[XUSER_MAX_COUNT];
    DisconnectPacket::eDisconnectReason
        m_connectionFailedReason[XUSER_MAX_COUNT];
    ClientConnection* m_pendingLocalConnections[XUSER_MAX_COUNT];

    bool addLocalPlayer(
        int idx);  
    void addPendingLocalConnection(int idx, ClientConnection* connection);
    void connectionDisconnected(int idx,
                                DisconnectPacket::eDisconnectReason reason) {
        m_connectionFailed[idx] = true;
        m_connectionFailedReason[idx] = reason;
    }

    std::shared_ptr<MultiplayerLocalPlayer> createExtraLocalPlayer(
        int idx, const std::wstring& name, int pad, int iDimension,
        ClientConnection* clientConnection = nullptr,
        MultiPlayerLevel* levelpassedin = nullptr);
    void createPrimaryLocalPlayer(int iPad);
    bool setLocalPlayerIdx(int idx);
    int getLocalPlayerIdx();
    void removeLocalPlayerIdx(int idx);
    void storeExtraLocalPlayer(int idx);
    void updatePlayerViewportAssignments();
    int unoccupiedQuadrant;  

    std::shared_ptr<LivingEntity> cameraTargetPlayer;
    std::shared_ptr<LivingEntity> crosshairPickMob;
    ParticleEngine* particleEngine;
    User* user;
    std::wstring serverDomain;
    Canvas* parent;
    bool appletMode;

    
    volatile bool pause;
    volatile bool exitingWorldRightNow;

    Textures* textures;
    Font *font, *altFont;
    Screen* screen;
    ProgressRenderer* progressRenderer;
    GameRenderer* gameRenderer;

private:
    BackgroundDownloader* bgLoader;

    int ticks;
    

    

    int orgWidth, orgHeight;

public:
    AchievementPopup* achievementPopup;

public:
    Gui* gui;
    
    bool noRender;

    HumanoidModel* humanoidModel;
    HitResult* hitResult;
    Options* options;

protected:
    MinecraftApplet* minecraftApplet;

public:
    SoundEngine* soundEngine;
    MouseHandler* mouseHandler;

public:
    TexturePackRepository* skins;
    File workingDirectory;

private:
    LevelStorageSource* levelSource;

public:
    static const int frameTimes_length = 512;
    static int64_t frameTimes[frameTimes_length];
    static const int tickTimes_length = 512;
    static int64_t tickTimes[tickTimes_length];
    static int frameTimePos;
    static int64_t warezTime;

private:
    int rightClickDelay;

public:
    
    StatsCounter* stats[4];

private:
    std::wstring connectToIp;
    int connectToPort;

public:
    void clearConnectionFailed();
    void connectTo(const std::wstring& server, int port);

private:
    void renderLoadingScreen();

public:
    void blit(int x, int y, int sx, int sy, int w, int h);

private:
    static File workDir;

public:
    static File getWorkingDirectory();
    static File getWorkingDirectory(const std::wstring& applicationName);
    static File getSavesDirectory();

public:
    LevelStorageSource* getLevelSource();
    void setScreen(Screen* screen);

private:
    void checkGlError(const std::wstring& string);

public:
    void destroy();
    volatile bool running;
    std::wstring fpsString;
    void run();
    
    
    static Minecraft* GetInstance();
    void run_middle();
    void run_end();

    void emergencySave();

    
    
private:
    
    
    

    
    int64_t lastTimer;

    void renderFpsMeter(int64_t tickTime);

public:
    void stop();
    
    
    
    
    
    
    

    void pauseGame();
    
    bool pollResize();

private:
    void resize(int width, int height);

public:
    
    

    
    

private:
    
    int recheckPlayerIn;
    void verify();

public:
    
    
    
    
    void tick(bool bFirst, bool bUpdateTextures);

private:
    void reloadSound();

public:
    bool isClientSide();
    void selectLevel(ConsoleSaveFile* saveFile, const std::wstring& levelId,
                     const std::wstring& levelName,
                     LevelSettings* levelSettings);
    
    bool saveSlot(int slot, const std::wstring& name);
    bool loadSlot(const std::wstring& userName, int slot);
    void releaseLevel(int message);
    
    
    
    
    void setLevel(MultiPlayerLevel* level, int message = -1,
                  std::shared_ptr<Player> forceInsertPlayer = nullptr,
                  bool doForceStatsSave = true,
                  bool bPrimaryPlayerSignedOut = false);
    
    
    void forceaddLevel(MultiPlayerLevel* level);
    void prepareLevel(int title);  
    void fileDownloaded(const std::wstring& name, File* file);
    

    std::wstring gatherStats1();
    std::wstring gatherStats2();
    std::wstring gatherStats3();
    std::wstring gatherStats4();

    void respawnPlayer(int iPad, int dimension, int newEntityId);
    static void start(const std::wstring& name, const std::wstring& sid);
    static void startAndConnectTo(const std::wstring& name,
                                  const std::wstring& sid,
                                  const std::wstring& url);
    ClientConnection* getConnection(int iPad);  
    static void main();
    static bool renderNames();
    static bool useFancyGraphics();
    static bool useAmbientOcclusion();
    static bool renderDebug();
    bool handleClientSideCommand(const std::wstring& chatMessage);

    static int maxSupportedTextureSize();
    void delayTextureReload();
    static int64_t currentTimeMillis();

    static int InGame_SignInReturned(void* pParam, bool bContinue, int iPad);
    
    Screen* getScreen();

    
    void forceStatsSave(int idx);

    std::recursive_mutex m_setLevelCS;

private:
    
    
    uint8_t m_inFullTutorialBits;

public:
    bool isTutorial();
    void playerStartedTutorial(int iPad);
    void playerLeftTutorial(int iPad);

    
    MultiPlayerLevel* getLevel(int dimension);

    void tickAllConnections();

    Level* animateTickLevel;  

    
    
    std::vector<std::wstring> m_pendingTextureRequests;
    std::vector<std::wstring>
        m_pendingGeometryRequests;  

    
    bool addPendingClientTextureRequest(const std::wstring& textureName);
    void handleClientTextureReceived(const std::wstring& textureName);
    void clearPendingClientTextureRequests() {
        m_pendingTextureRequests.clear();
    }
    bool addPendingClientGeometryRequest(const std::wstring& textureName);
    void handleClientGeometryReceived(const std::wstring& textureName);
    void clearPendingClientGeometryRequests() {
        m_pendingGeometryRequests.clear();
    }

    unsigned int getCurrentTexturePackId();
    ColourTable* getColourTable();
};