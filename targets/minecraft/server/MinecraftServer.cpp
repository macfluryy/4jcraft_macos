#include "MinecraftServer.h"

#include <assert.h>
#include <wchar.h>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <utility>

#include "platform/PlatformTypes.h"
#include "platform/sdl2/Profile.h"
#include "platform/sdl2/Storage.h"
#include "ConsoleInput.h"
#include "DispenserBootstrap.h"
#include "app/common/App_Defines.h"
#include "app/common/App_enums.h"
#include "app/common/src/GameRules/GameRuleManager.h"
#include "app/common/src/GameRules/LevelGeneration/LevelGenerationOptions.h"
#include "app/common/src/Network/GameNetworkManager.h"
#include "app/common/src/Network/LanDiscovery.h"
#include "app/common/src/Network/NetworkPlayerInterface.h"
#include "app/common/src/BuildVer/BuildVer.h"
#include "app/mac/MacGame.h"
#include "PlayerList.h"
#include "Settings.h"
#include "util/Timer.h"
#include "java/Class.h"
#include "java/File.h"
#include "java/InputOutputStream/DataOutputStream.h"
#include "java/InputOutputStream/FileOutputStream.h"
#include "java/Random.h"
#include "java/System.h"
#include "minecraft/Pos.h"
#include "minecraft/client/Options.h"
#include "minecraft/commands/Command.h"
#include "minecraft/network/packet/GameEventPacket.h"
#include "minecraft/network/packet/ServerSettingsChangedPacket.h"
#include "minecraft/network/packet/SetTimePacket.h"
#include "minecraft/network/packet/UpdateProgressPacket.h"
#include "minecraft/server/level/DerivedServerLevel.h"
#include "minecraft/server/level/EntityTracker.h"
#include "minecraft/server/level/ServerChunkCache.h"
#include "minecraft/server/level/ServerLevel.h"
#include "minecraft/server/network/ServerConnection.h"
#include "minecraft/world/entity/Entity.h"
#include "minecraft/world/entity/EntityIO.h"
#include "minecraft/world/entity/Mob.h"
#include "minecraft/world/entity/player/Player.h"
#include "minecraft/world/item/ItemInstance.h"
#include "minecraft/world/level/GameRules.h"
#include "minecraft/world/level/LevelSettings.h"
#include "minecraft/world/level/LevelType.h"
#include "minecraft/world/level/chunk/ChunkSource.h"
#include "minecraft/world/level/dimension/Dimension.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/ConsoleSaveFile.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/ConsoleSavePath.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/FileHeader.h"
#include "minecraft/world/level/storage/LevelData.h"
#include "minecraft/world/level/storage/LevelStorage.h"
#include "minecraft/world/level/storage/McRegionLevelStorage.h"
#include "minecraft/world/level/storage/McRegionLevelStorageSource.h"
#include "minecraft/world/level/tile/Tile.h"
#include "strings.h"
#if defined(SPLIT_SAVES)
#include "minecraft/world/level/storage/ConsoleSaveFileIO/ConsoleSaveFileSplit.h"
#endif
#include "platform/sdl2/Input.h"
#include "platform/ShutdownManager.h"
#include "app/common/src/Console_Debug_enum.h"
#include "app/common/src/GameRules/LevelGeneration/ConsoleSchematicFile.h"
#include "app/common/src/Network/Socket.h"
#include "app/common/src/UI/All Platforms/UIStructs.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/compression.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/ProgressRenderer.h"
#include "minecraft/client/renderer/GameRenderer.h"
#include "minecraft/server/commands/ServerCommandDispatcher.h"
#include "minecraft/server/level/ServerPlayer.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/biome/BiomeSource.h"
#include "minecraft/world/level/chunk/CompressedTileStorage.h"
#include "minecraft/world/level/chunk/SparseDataStorage.h"
#include "minecraft/world/level/chunk/SparseLightStorage.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/ConsoleSaveFileOriginal.h"

class ConsoleInputSource;

#define DEBUG_SERVER_DONT_SPAWN_MOBS 0


MinecraftServer* MinecraftServer::server = nullptr;
bool MinecraftServer::setTimeAtEndOfTick = false;
int64_t MinecraftServer::setTime = 0;
bool MinecraftServer::setTimeOfDayAtEndOfTick = false;
int64_t MinecraftServer::setTimeOfDay = 0;
bool MinecraftServer::m_bPrimaryPlayerSignedOut = false;
bool MinecraftServer::s_bServerHalted = false;
bool MinecraftServer::s_bSaveOnExitAnswered = false;
#if defined(_ACK_CHUNK_SEND_THROTTLING)
bool MinecraftServer::s_hasSentEnoughPackets = false;
int64_t MinecraftServer::s_tickStartTime = 0;
std::vector<INetworkPlayer*> MinecraftServer::s_sentTo;
#else
int MinecraftServer::s_slowQueuePlayerIndex = 0;
time_util::time_point MinecraftServer::s_slowQueueLastTime = {};
bool MinecraftServer::s_slowQueuePacketSent = false;
#endif

std::unordered_map<std::wstring, int> MinecraftServer::ironTimers;

MinecraftServer::MinecraftServer() {
    
    connection = nullptr;
    settings = nullptr;
    players = nullptr;
    commands = nullptr;
    running = true;
    m_bLoaded = false;
    stopped = false;
    tickCount = 0;
    std::wstring progressStatus;
    progress = 0;
    motd = L"";

    m_isServerPaused = false;
    m_serverPausedEvent = new C4JThread::Event;

    m_saveOnExit = false;
    m_suspending = false;

    m_ugcPlayersVersion = 0;
    m_texturePackId = 0;
    maxBuildHeight = Level::maxBuildHeight;
    playerIdleTimeout = 0;
    m_postUpdateThread = nullptr;
    forceGameType = false;

    commandDispatcher = new ServerCommandDispatcher();

    DispenserBootstrap::bootStrap();
}

MinecraftServer::~MinecraftServer() {}

bool MinecraftServer::initServer(int64_t seed, NetworkGameInitData* initData,
                                 std::uint32_t initSettings, bool findSeed) {
    
    settings = new Settings(new File(L"server.properties"));

    app.DebugPrintf("\n*** SERVER SETTINGS ***\n");
    app.DebugPrintf(
        "ServerSettings: host-friends-only is %s\n",
        (app.GetGameHostOption(eGameHostOption_FriendsOfFriends) > 0) ? "on"
                                                                      : "off");
    app.DebugPrintf("ServerSettings: game-type is %s\n",
                    (app.GetGameHostOption(eGameHostOption_GameType) == 0)
                        ? "Survival Mode"
                        : "Creative Mode");
    app.DebugPrintf(
        "ServerSettings: pvp is %s\n",
        (app.GetGameHostOption(eGameHostOption_PvP) > 0) ? "on" : "off");
    app.DebugPrintf("ServerSettings: fire spreads is %s\n",
                    (app.GetGameHostOption(eGameHostOption_FireSpreads) > 0)
                        ? "on"
                        : "off");
    app.DebugPrintf(
        "ServerSettings: tnt explodes is %s\n",
        (app.GetGameHostOption(eGameHostOption_TNT) > 0) ? "on" : "off");
    app.DebugPrintf("\n");

    
    
    

    
    
    
    
    

    setAnimals(settings->getBoolean(L"spawn-animals", true));
    setNpcsEnabled(settings->getBoolean(L"spawn-npcs", true));
    setPvpAllowed(app.GetGameHostOption(eGameHostOption_PvP) > 0
                      ? true
                      : false);  

    
    
    
    
    setFlightAllowed(true);  

    
#if defined(_DEBUG_MENUS_ENABLED)
    setFlightAllowed(true);
#endif

    connection = new ServerConnection(this);
    Socket::Initialise(connection);  
    setPlayers(new PlayerList(this));

    
    while (app.getLevelGenerationOptions() != nullptr &&
           !app.getLevelGenerationOptions()->hasLoadedData())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    if (app.getLevelGenerationOptions() != nullptr &&
        !app.getLevelGenerationOptions()->ready()) {
        
    }

    int64_t levelNanoTime = System::nanoTime();

    std::wstring levelName = settings->getString(L"level-name", L"world");
    {
        const std::wstring& storageTitle = StorageManager.GetSaveTitle();
        if (!storageTitle.empty()) {
            levelName = storageTitle;
        }
    }
    std::wstring levelTypeString;

    bool gameRuleUseFlatWorld = false;
    if (app.getLevelGenerationOptions() != nullptr) {
        gameRuleUseFlatWorld =
            app.getLevelGenerationOptions()->getuseFlatWorld();
    }
    std::wstring defaultLevelType = L"default";
    unsigned int levelTypeOption =
        app.GetGameHostOption(eGameHostOption_LevelType);
    if (gameRuleUseFlatWorld) {
        defaultLevelType = L"flat";
    } else if (levelTypeOption == e_levelType_Superflat) {
        defaultLevelType = L"flat";
    } else if (levelTypeOption == e_levelType_LargeBiomes) {
        defaultLevelType = L"largeBiomes";
    } else if (levelTypeOption == e_levelType_Amplified) {
        defaultLevelType = L"amplified";
    } else if (levelTypeOption == e_levelType_Triple) {
        
        
        defaultLevelType = L"triple";
    }
    levelTypeString = settings->getString(L"level-type", defaultLevelType);

    LevelType* pLevelType = LevelType::getLevelType(levelTypeString);
    if (pLevelType == nullptr) {
        pLevelType = LevelType::lvl_normal;
    }

    ProgressRenderer* mcprogress = Minecraft::GetInstance()->progressRenderer;
    mcprogress->progressStart(IDS_PROGRESS_INITIALISING_SERVER);

    if (findSeed) {
        seed = BiomeSource::findSeed(pLevelType);
    }

    setMaxBuildHeight(
        settings->getInt(L"max-build-height", Level::maxBuildHeight));
    setMaxBuildHeight(((getMaxBuildHeight() + 8) / 16) * 16);
    setMaxBuildHeight(
        std::clamp(getMaxBuildHeight(), 64, Level::maxBuildHeight));
    

    
    m_bLoaded = loadLevel(new McRegionLevelStorageSource(File(L".")), levelName,
                          seed, pLevelType, initData);
    
    

    
    
    
    if (initData->saveData) {
        delete[] reinterpret_cast<std::uint8_t*>(initData->saveData->data);
        initData->saveData->data = 0;
        initData->saveData->fileSize = 0;
    }

    g_NetworkManager.ServerReady();  
    return m_bLoaded;
}



int MinecraftServer::runPostUpdate(void* lpParam) {
    ShutdownManager::HasStarted(ShutdownManager::ePostProcessThread);

    MinecraftServer* server = (MinecraftServer*)lpParam;
    Entity::useSmallIds();  
                            
    Compression::UseDefaultThreadStorage();
    Level::enableLightingCache();
    Tile::CreateNewThreadStorage();

    
    do {
        {
            std::unique_lock<std::mutex> lock(server->m_postProcessCS);
            if (server->m_postProcessRequests.size()) {
                MinecraftServer::postProcessRequest request =
                    server->m_postProcessRequests.back();
                server->m_postProcessRequests.pop_back();
                lock.unlock();
                static int count = 0;
                request.chunkSource->postProcess(request.chunkSource, request.x,
                                                 request.z);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    } while (!server->m_postUpdateTerminate &&
             ShutdownManager::ShouldRun(ShutdownManager::ePostProcessThread));
    
    
    {
        std::unique_lock<std::mutex> lock(server->m_postProcessCS);
        int maxRequests = server->m_postProcessRequests.size();
        while (
            server->m_postProcessRequests.size() &&
            ShutdownManager::ShouldRun(ShutdownManager::ePostProcessThread)) {
            MinecraftServer::postProcessRequest request =
                server->m_postProcessRequests.back();
            server->m_postProcessRequests.pop_back();
            lock.unlock();
            request.chunkSource->postProcess(request.chunkSource, request.x,
                                             request.z);
            lock.lock();
        }
    }
    
    Tile::ReleaseThreadStorage();
    Level::destroyLightingCache();

    ShutdownManager::HasFinished(ShutdownManager::ePostProcessThread);

    return 0;
}

void MinecraftServer::addPostProcessRequest(ChunkSource* chunkSource, int x,
                                            int z) {
    {
        std::lock_guard<std::mutex> lock(m_postProcessCS);
        m_postProcessRequests.push_back(
            MinecraftServer::postProcessRequest(x, z, chunkSource));
    }
}

void MinecraftServer::postProcessTerminate(ProgressRenderer* mcprogress) {
    std::uint32_t status = 0;
    size_t postProcessItemCount = 0;
    size_t postProcessItemRemaining = 0;

    {
        std::lock_guard<std::mutex> lock(server->m_postProcessCS);
        postProcessItemCount = server->m_postProcessRequests.size();
    }

    do {
        status = m_postUpdateThread->waitForCompletion(50);
        if (status == C4JThread::WaitResult::Timeout) {
            {
                std::lock_guard<std::mutex> lock(server->m_postProcessCS);
                postProcessItemRemaining = server->m_postProcessRequests.size();
            }

            if (postProcessItemCount) {
                mcprogress->progressStagePercentage(
                    (postProcessItemCount - postProcessItemRemaining) * 100 /
                    postProcessItemCount);
            }
            CompressedTileStorage::tick();
            SparseLightStorage::tick();
            SparseDataStorage::tick();
        }
    } while (status == C4JThread::WaitResult::Timeout);
    delete m_postUpdateThread;
    m_postUpdateThread = nullptr;
}

bool MinecraftServer::loadLevel(LevelStorageSource* storageSource,
                                const std::wstring& name, int64_t levelSeed,
                                LevelType* pLevelType,
                                NetworkGameInitData* initData) {
    
    
    
    
    
    ProgressRenderer* mcprogress = Minecraft::GetInstance()->progressRenderer;

    
    levels = std::vector<ServerLevel*>(3);

    int gameTypeId = settings->getInt(
        L"gamemode",
        app.GetGameHostOption(
            eGameHostOption_GameType));  
    GameType* gameType = LevelSettings::validateGameType(gameTypeId);
    app.DebugPrintf("Default game type: %d\n", gameTypeId);

    LevelSettings* levelSettings = new LevelSettings(
        levelSeed, gameType,
        app.GetGameHostOption(eGameHostOption_Structures) > 0 ? true : false,
        isHardcore(), true, pLevelType, initData->xzSize, initData->hellScale);
    if (app.GetGameHostOption(eGameHostOption_BonusChest))
        levelSettings->enableStartingBonusItems();

    
    std::shared_ptr<McRegionLevelStorage> storage = nullptr;
    bool levelChunksNeedConverted = false;
    if (initData->saveData != nullptr) {
        

#if defined(SPLIT_SAVES)
        ConsoleSaveFileOriginal oldFormatSave(
            initData->saveData->saveName, initData->saveData->data,
            initData->saveData->fileSize, false, initData->savePlatform);
        ConsoleSaveFile* pSave = new ConsoleSaveFileSplit(&oldFormatSave);

        
        
        
#else
        ConsoleSaveFile* pSave = new ConsoleSaveFileOriginal(
            initData->saveData->saveName, initData->saveData->data,
            initData->saveData->fileSize, false, initData->savePlatform);
#endif
        if (pSave->isSaveEndianDifferent()) levelChunksNeedConverted = true;
        pSave->ConvertToLocalPlatform();  
                                          

        storage = std::shared_ptr<McRegionLevelStorage>(
            new McRegionLevelStorage(pSave, File(L"."), name, true));
    } else {
        
#if defined(SPLIT_SAVES)
        bool bLevelGenBaseSave = false;
        LevelGenerationOptions* levelGen = app.getLevelGenerationOptions();
        if (levelGen != nullptr && levelGen->requiresBaseSave()) {
            unsigned int fileSize = 0;
            std::uint8_t* pvSaveData = levelGen->getBaseSaveData(fileSize);
            if (pvSaveData && fileSize != 0) bLevelGenBaseSave = true;
        }
        ConsoleSaveFileSplit* newFormatSave = nullptr;
        if (bLevelGenBaseSave) {
            ConsoleSaveFileOriginal oldFormatSave(L"");
            newFormatSave = new ConsoleSaveFileSplit(&oldFormatSave);
        } else {
            newFormatSave = new ConsoleSaveFileSplit(name);
        }

        if (!bLevelGenBaseSave) {
            newFormatSave->ReadEntriesFromFolderOnDisk(name);
        }

        storage = std::shared_ptr<McRegionLevelStorage>(
            new McRegionLevelStorage(newFormatSave, File(L"."), name, true));
#else
        storage = std::make_shared<McRegionLevelStorage>(
            new ConsoleSaveFileOriginal(L""), File(L"."), name, true);
#endif
    }

    
    
    
    
    for (unsigned int i = 0; i < levels.size(); i++) {
        if (s_bServerHalted || !g_NetworkManager.IsInSession()) {
            return false;
        }

        
        
        int dimension = 0;
        if (i == 1) dimension = -1;
        if (i == 2) dimension = 1;
        if (i == 0) {
            levels[i] =
                new ServerLevel(this, storage, name, dimension, levelSettings);
            if (app.getLevelGenerationOptions() != nullptr) {
                LevelGenerationOptions* mapOptions =
                    app.getLevelGenerationOptions();
                Pos* spawnPos = mapOptions->getSpawnPos();
                if (spawnPos != nullptr) {
                    levels[i]->setSpawnPos(spawnPos);
                }

                levels[i]->getLevelData()->setHasBeenInCreative(
                    mapOptions->isFromDLC());
            }
        } else
            levels[i] = new DerivedServerLevel(this, storage, name, dimension,
                                               levelSettings, levels[0]);
        
        
        
        

        
        
        
        Minecraft* pMinecraft = Minecraft::GetInstance();
        
        levels[i]->difficulty = app.GetGameHostOption(
            eGameHostOption_Difficulty);  
        app.DebugPrintf("MinecraftServer::loadLevel - Difficulty = %d\n",
                        levels[i]->difficulty);

#if DEBUG_SERVER_DONT_SPAWN_MOBS
        levels[i]->setSpawnSettings(false, false);
#else
        levels[i]->setSpawnSettings(
            settings->getBoolean(L"spawn-monsters", true), animals);
#endif
        levels[i]->getLevelData()->setGameType(gameType);

        if (app.getLevelGenerationOptions() != nullptr) {
            LevelGenerationOptions* mapOptions =
                app.getLevelGenerationOptions();
            levels[i]->getLevelData()->setHasBeenInCreative(
                mapOptions->getLevelHasBeenInCreative());
        }

        players->setLevel(levels);
    }

    if (levels[0]->isNew) {
        mcprogress->progressStage(IDS_PROGRESS_GENERATING_SPAWN_AREA);
    } else {
        mcprogress->progressStage(IDS_PROGRESS_LOADING_SPAWN_AREA);
    }
    app.SetGameHostOption(
        eGameHostOption_HasBeenInCreative,
        gameType == GameType::CREATIVE || levels[0]->getHasBeenInCreative());
    app.SetGameHostOption(eGameHostOption_Structures,
                          levels[0]->isGenerateMapFeatures());

    
    
    
    
    
    
    if (!levels[0]->isNew) {
        unsigned int persisted = levels[0]->getLevelData()->getGameHostSettings();
        if (persisted != 0) {
            unsigned int curGameType =
                app.GetGameHostOption(eGameHostOption_GameType);
            unsigned int curDifficulty =
                app.GetGameHostOption(eGameHostOption_Difficulty);
            app.SetGameHostOption(eGameHostOption_All, persisted);
            
            
            app.SetGameHostOption(eGameHostOption_GameType, curGameType);
            app.SetGameHostOption(eGameHostOption_Difficulty, curDifficulty);
            
            setPvpAllowed(app.GetGameHostOption(eGameHostOption_PvP) > 0);
            app.DebugPrintf(
                "[world] Restored persisted host settings 0x%08x\n",
                app.GetGameHostOption(eGameHostOption_All));
        }
    }
    
    
    for (unsigned int li = 0; li < levels.size(); ++li) {
        if (levels[li] != nullptr && levels[li]->getLevelData() != nullptr) {
            levels[li]->getLevelData()->setGameHostSettings(
                app.GetGameHostOption(eGameHostOption_All));
        }
    }

    if (s_bServerHalted || !g_NetworkManager.IsInSession()) return false;

    

    
    
    
    

    m_postUpdateThread =
        new C4JThread(runPostUpdate, this, "Post processing", 256 * 1024);

    m_postUpdateTerminate = false;
    m_postUpdateThread->setPriority(C4JThread::ThreadPriority::AboveNormal);
    m_postUpdateThread->run();

    int64_t startTime = System::currentTimeMillis();

    
    int r = 196;

    
    ConsoleSavePath filepath(GAME_RULE_SAVENAME);
    ConsoleSaveFile* csf = getLevel(0)->getLevelStorage()->getSaveFile();
    if (csf->doesFileExist(filepath)) {
        unsigned int numberOfBytesRead;
        std::vector<uint8_t> ba_gameRules;

        FileEntry* fe = csf->createFile(filepath);

        ba_gameRules.resize(fe->getFileSize());

        csf->setFilePointer(fe, 0, SaveFileSeekOrigin::Begin);
        csf->readFile(fe, ba_gameRules.data(), ba_gameRules.size(),
                      &numberOfBytesRead);
        assert(numberOfBytesRead == ba_gameRules.size());

        app.m_gameRules.loadGameRules(ba_gameRules.data(), ba_gameRules.size());
        csf->closeHandle(fe);
    }

    int64_t lastTime = System::currentTimeMillis();
#if defined(_LARGE_WORLDS)
    if (app.GetGameNewWorldSize() > levels[0]->getLevelData()->getXZSizeOld()) {
        if (!app.GetGameNewWorldSizeUseMoat())  
                                                
                                                
        {
            overwriteBordersForNewWorldSize(levels[0]);
        }
        
        int oldHellSize = levels[0]->getLevelData()->getXZHellSizeOld();
        overwriteHellBordersForNewWorldSize(levels[1], oldHellSize);
    }
#endif

    
    
    int i = 0;
    for (int i = 0; i < levels.size(); i++) {
        
        if (i == 0 || settings->getBoolean(L"allow-nether", true)) {
            ServerLevel* level = levels[i];
            if (levelChunksNeedConverted) {
                
            }

            int64_t lastStorageTickTime = System::currentTimeMillis();
            Pos* spawnPos = level->getSharedSpawnPos();

            int twoRPlusOne = r * 2 + 1;
            int total = twoRPlusOne * twoRPlusOne;
            for (int x = -r; x <= r && running; x += 16) {
                for (int z = -r; z <= r && running; z += 16) {
                    if (s_bServerHalted || !g_NetworkManager.IsInSession()) {
                        delete spawnPos;
                        m_postUpdateTerminate = true;
                        postProcessTerminate(mcprogress);
                        return false;
                    }
                    
                    
                    
                    
                    
                    
                    {
                        int pos = (x + r) * twoRPlusOne + (z + 1);
                        
                        
                        mcprogress->progressStagePercentage((pos + r) * 100 /
                                                            total);
                        
                    }
                    static int count = 0;
                    level->cache->create((spawnPos->x + x) >> 4,
                                         (spawnPos->z + z) >> 4,
                                         true);  
                                                 

                    
                    
                    
                    if (System::currentTimeMillis() - lastStorageTickTime >
                        50) {
                        CompressedTileStorage::tick();
                        SparseLightStorage::tick();
                        SparseDataStorage::tick();
                        lastStorageTickTime = System::currentTimeMillis();
                    }
                }
            }

            
            
            

            delete spawnPos;
        }
    }
    
    

    
    
    m_postUpdateTerminate = true;

    postProcessTerminate(mcprogress);

    
    if (levels[0]->dimension->id == 0) {
        app.DebugPrintf("===================================\n");

        if (!levels[0]->getLevelData()->getHasStronghold()) {
            int x, z;
            if (app.GetTerrainFeaturePosition(eTerrainFeature_Stronghold, &x,
                                              &z)) {
                levels[0]->getLevelData()->setXStronghold(x);
                levels[0]->getLevelData()->setZStronghold(z);
                levels[0]->getLevelData()->setHasStronghold();

                app.DebugPrintf(
                    "=== FOUND stronghold in terrain features list\n");

            } else {
                
                
                app.DebugPrintf(
                    "=== Can't find stronghold in terrain features list\n");
            }
        } else {
            app.DebugPrintf("=== Leveldata has stronghold position\n");
        }
        app.DebugPrintf("===================================\n");
    }

    
    

    
    

    if (s_bServerHalted || !g_NetworkManager.IsInSession()) return false;

    if (levels[1]->isNew) {
        levels[1]->save(true, mcprogress);
    }

    if (s_bServerHalted || !g_NetworkManager.IsInSession()) return false;

    if (levels[2]->isNew) {
        levels[2]->save(true, mcprogress);
    }

    if (s_bServerHalted || !g_NetworkManager.IsInSession()) return false;

    
    
    if (levels[0]->isNew) saveGameRules();

    if (levels[0]->isNew) {
        levels[0]->save(true, mcprogress);
    }

    if (s_bServerHalted || !g_NetworkManager.IsInSession()) return false;

    if (levels[0]->isNew || levels[1]->isNew || levels[2]->isNew) {
        levels[0]->saveToDisc(mcprogress, false);
    }

    if (s_bServerHalted || !g_NetworkManager.IsInSession()) return false;

    







    endProgress();

    return true;
}

#if defined(_LARGE_WORLDS)
void MinecraftServer::overwriteBordersForNewWorldSize(ServerLevel* level) {
    
    
    app.DebugPrintf("Expanding level size\n");
    int oldSize = level->getLevelData()->getXZSizeOld();
    
    int minVal = -oldSize / 2;
    int maxVal = (oldSize / 2) - 1;
    for (int xVal = minVal; xVal <= maxVal; xVal++) {
        int zVal = minVal;
        level->cache->overwriteLevelChunkFromSource(xVal, zVal);
        level->cache->overwriteLevelChunkFromSource(xVal, zVal + 1);
    }
    
    for (int xVal = minVal; xVal <= maxVal; xVal++) {
        int zVal = maxVal;
        level->cache->overwriteLevelChunkFromSource(xVal, zVal);
        level->cache->overwriteLevelChunkFromSource(xVal, zVal - 1);
    }
    
    for (int zVal = minVal; zVal <= maxVal; zVal++) {
        int xVal = minVal;
        level->cache->overwriteLevelChunkFromSource(xVal, zVal);
        level->cache->overwriteLevelChunkFromSource(xVal + 1, zVal);
    }
    
    for (int zVal = minVal; zVal <= maxVal; zVal++) {
        int xVal = maxVal;
        level->cache->overwriteLevelChunkFromSource(xVal, zVal);
        level->cache->overwriteLevelChunkFromSource(xVal - 1, zVal);
    }
}

void MinecraftServer::overwriteHellBordersForNewWorldSize(ServerLevel* level,
                                                          int oldHellSize) {
    
    
    app.DebugPrintf("Expanding level size\n");
    
    int minVal = -oldHellSize / 2;
    int maxVal = (oldHellSize / 2) - 1;
    for (int xVal = minVal; xVal <= maxVal; xVal++) {
        int zVal = minVal;
        level->cache->overwriteHellLevelChunkFromSource(xVal, zVal, minVal,
                                                        maxVal);
    }
    
    for (int xVal = minVal; xVal <= maxVal; xVal++) {
        int zVal = maxVal;
        level->cache->overwriteHellLevelChunkFromSource(xVal, zVal, minVal,
                                                        maxVal);
    }
    
    for (int zVal = minVal; zVal <= maxVal; zVal++) {
        int xVal = minVal;
        level->cache->overwriteHellLevelChunkFromSource(xVal, zVal, minVal,
                                                        maxVal);
    }
    
    for (int zVal = minVal; zVal <= maxVal; zVal++) {
        int xVal = maxVal;
        level->cache->overwriteHellLevelChunkFromSource(xVal, zVal, minVal,
                                                        maxVal);
    }
}

#endif

void MinecraftServer::setProgress(const std::wstring& status, int progress) {
    progressStatus = status;
    this->progress = progress;
    
}

void MinecraftServer::endProgress() {
    progressStatus = L"";
    this->progress = 0;
}

void MinecraftServer::saveAllChunks() {
    
    for (unsigned int i = 0; i < levels.size(); i++) {
        
        
        if (m_bPrimaryPlayerSignedOut) break;
        
        
        
        
        ServerLevel* level = levels[levels.size() - 1 - i];
        if (level)  
                    
        {
            level->save(true, Minecraft::GetInstance()->progressRenderer);

            
            
            
            if (i == (levels.size() - 1)) {
                level->closeLevelStorage();
            }
        }
    }
}


void MinecraftServer::saveGameRules() {
#if !defined(_CONTENT_PACKAGE)
    if (app.DebugSettingsOn() &&
        app.GetGameSettingsDebugMask(InputManager.GetPrimaryPad()) &
            (1L << eDebugSetting_DistributableSave)) {
        
    } else
#endif
    {
        uint8_t* baPtr = nullptr;
        unsigned int baSize = 0;
        app.m_gameRules.saveGameRules(&baPtr, &baSize);

        if (baPtr != nullptr) {
            std::vector<uint8_t> ba(baPtr, baPtr + baSize);
            ConsoleSaveFile* csf =
                getLevel(0)->getLevelStorage()->getSaveFile();
            FileEntry* fe =
                csf->createFile(ConsoleSavePath(GAME_RULE_SAVENAME));
            csf->setFilePointer(fe, 0, SaveFileSeekOrigin::Begin);
            unsigned int length;
            csf->writeFile(fe, ba.data(), ba.size(), &length);

            csf->closeHandle(fe);
        }
    }
}







void MinecraftServer::forceShutdownSave() {
    if (StorageManager.GetSaveDisabled()) return;
    if (s_bServerHalted) return;

    if (players != nullptr) {
        
        
        
        
        PlayerIO* pio = players->getPlayerIO();
        if (pio != nullptr) {
            for (size_t i = 0; i < players->players.size(); i++) {
                std::shared_ptr<ServerPlayer> p = players->players[i];
                if (p != nullptr) pio->save(p);
            }
            
            pio->saveAllCachedData();
            pio->saveMapIdLookup();
        }
    }

    
    
    
    saveGameRules();
    saveAllChunks();
    if (!levels.empty() && levels[0] != nullptr) {
        levels[0]->saveToDisc(nullptr, true);
    }
}

void MinecraftServer::Suspend() {
    m_suspending = true;
    time_util::Timer timer;
    if (m_bLoaded && (!StorageManager.GetSaveDisabled())) {
        if (players != nullptr) {
            players->saveAll(nullptr);
        }
        for (unsigned int j = 0; j < levels.size(); j++) {
            if (s_bServerHalted) break;
            
            
            
            
            ServerLevel* level = levels[levels.size() - 1 - j];
            level->Suspend();
        }
        if (!s_bServerHalted) {
            saveGameRules();
            levels[0]->saveToDisc(nullptr, true);
        }
    }

    m_suspending = false;
    app.DebugPrintf("Suspend server: Elapsed time %f\n",
                    static_cast<float>(timer.elapsed_seconds()));
}

bool MinecraftServer::IsSuspending() { return m_suspending; }

void MinecraftServer::stopServer(bool didInit) {
    
    
    {
        Minecraft::GetInstance()->gameRenderer->DisableUpdateThread();
    }

    connection->stop();

    app.DebugPrintf("Stopping server\n");
    
    
    

    
    
    if ((m_bPrimaryPlayerSignedOut == false) &&
        ProfileManager.IsSignedIn(InputManager.GetPrimaryPad())) {
        
        
        
        if (m_saveOnExit && (!StorageManager.GetSaveDisabled()) && didInit) {
            if (players != nullptr) {
                players->saveAll(Minecraft::GetInstance()->progressRenderer,
                                 true);
            }
            
            
            
            
            
            
            
            
            
            saveAllChunks();
            
            

            saveGameRules();
            app.m_gameRules.unloadCurrentGameRules();
            if (levels[0] != nullptr)  
                                       
            {
                levels[0]->saveToDisc(
                    Minecraft::GetInstance()->progressRenderer, false);
            }
        }
    }
    
    m_bPrimaryPlayerSignedOut = false;
    s_bServerHalted = false;

    
    
    
    
    

    
    unsigned int iServerLevelC = levels.size();
    for (unsigned int i = 0; i < iServerLevelC; i++) {
        if (levels[i] != nullptr) {
            delete levels[i];
            levels[i] = nullptr;
        }
    }

    delete connection;
    connection = nullptr;
    delete players;
    players = nullptr;
    delete settings;
    settings = nullptr;

    g_NetworkManager.ServerStopped();
}

void MinecraftServer::halt() { running = false; }

void MinecraftServer::setMaxBuildHeight(int maxBuildHeight) {
    this->maxBuildHeight = maxBuildHeight;
}

int MinecraftServer::getMaxBuildHeight() { return maxBuildHeight; }

PlayerList* MinecraftServer::getPlayers() { return players; }

void MinecraftServer::setPlayers(PlayerList* players) {
    this->players = players;
}

ServerConnection* MinecraftServer::getConnection() { return connection; }

bool MinecraftServer::isAnimals() { return animals; }

void MinecraftServer::setAnimals(bool animals) { this->animals = animals; }

bool MinecraftServer::isNpcsEnabled() { return npcs; }

void MinecraftServer::setNpcsEnabled(bool npcs) { this->npcs = npcs; }

bool MinecraftServer::isPvpAllowed() { return pvp; }

void MinecraftServer::setPvpAllowed(bool pvp) { this->pvp = pvp; }

bool MinecraftServer::isFlightAllowed() { return allowFlight; }

void MinecraftServer::setFlightAllowed(bool allowFlight) {
    this->allowFlight = allowFlight;
}

bool MinecraftServer::isCommandBlockEnabled() {
    return false;  
}

bool MinecraftServer::isNetherEnabled() {
    return true;  
}

bool MinecraftServer::isHardcore() { return false; }

int MinecraftServer::getOperatorUserPermissionLevel() {
    return Command::LEVEL_OWNERS;  
                                   
}

CommandDispatcher* MinecraftServer::getCommandDispatcher() {
    return commandDispatcher;
}

Pos* MinecraftServer::getCommandSenderWorldPosition() {
    return new Pos(0, 0, 0);
}

Level* MinecraftServer::getCommandSenderWorld() { return levels[0]; }

int MinecraftServer::getSpawnProtectionRadius() { return 16; }

bool MinecraftServer::isUnderSpawnProtection(Level* level, int x, int y, int z,
                                             std::shared_ptr<Player> player) {
    if (level->dimension->id != 0) return false;
    
    if (getPlayers()->isOp(player->getName())) return false;
    if (getSpawnProtectionRadius() <= 0) return false;

    Pos* spawnPos = level->getSharedSpawnPos();
    int xd = std::abs(x - spawnPos->x);
    int zd = std::abs(z - spawnPos->z);
    int dist = std::max(xd, zd);

    return dist <= getSpawnProtectionRadius();
}

void MinecraftServer::setForceGameType(bool forceGameType) {
    this->forceGameType = forceGameType;
}

bool MinecraftServer::getForceGameType() { return forceGameType; }

int64_t MinecraftServer::getCurrentTimeMillis() {
    return System::currentTimeMillis();
}

int MinecraftServer::getPlayerIdleTimeout() { return playerIdleTimeout; }

void MinecraftServer::setPlayerIdleTimeout(int playerIdleTimeout) {
    this->playerIdleTimeout = playerIdleTimeout;
}

extern int c0a, c0b, c1a, c1b, c1c, c2a, c2b;
void MinecraftServer::run(int64_t seed, void* lpParameter) {
    NetworkGameInitData* initData = nullptr;
    std::uint32_t initSettings = 0;
    bool findSeed = false;
    if (lpParameter != nullptr) {
        initData = (NetworkGameInitData*)lpParameter;
        initSettings = app.GetGameHostOption(eGameHostOption_All);
        findSeed = initData->findSeed;
        m_texturePackId = initData->texturePackId;
    }
    
    bool didInit = false;
    if (initServer(seed, initData, initSettings, findSeed)) {
        didInit = true;
        ServerLevel* levelNormalDimension = levels[0];
        
        
        Minecraft* pMinecraft = Minecraft::GetInstance();
        LevelData* pLevelData = levelNormalDimension->getLevelData();

        if (pLevelData && pLevelData->getHasStronghold() == false) {
            int x, z;
            if (app.GetTerrainFeaturePosition(eTerrainFeature_Stronghold, &x,
                                              &z)) {
                pLevelData->setXStronghold(x);
                pLevelData->setZStronghold(z);
                pLevelData->setHasStronghold();
            }
        }

        int64_t lastTime = getCurrentTimeMillis();
        int64_t unprocessedTime = 0;
        while (running && !s_bServerHalted) {
            int64_t now = getCurrentTimeMillis();

            
            
            
            
            
            

            int64_t passedTime = now - lastTime;
            if (passedTime > MS_PER_TICK * 40) {
                
                
                passedTime = MS_PER_TICK * 40;
            }
            if (passedTime < 0) {
                
                
                passedTime = 0;
            }
            unprocessedTime += passedTime;
            lastTime = now;

            
            if (!m_isServerPaused) {
                bool didTick = false;
                if (levels[0]->allPlayersAreSleeping()) {
                    tick();
                    unprocessedTime = 0;
                } else {
                    
                    
                    
                    while (unprocessedTime > MS_PER_TICK) {
                        unprocessedTime -= MS_PER_TICK;
                        chunkPacketManagement_PreTick();
                        
                        
                        int64_t tickStartNs = System::nanoTime();
                        tick();
                        int64_t tickElapsedNs =
                            System::nanoTime() - tickStartNs;
                        m_tickTimesNs[m_tickTimesIndex] = tickElapsedNs;
                        m_tickTimesIndex =
                            (m_tickTimesIndex + 1) % TPS_SAMPLE_COUNT;
                        if (m_tickTimesFilled < TPS_SAMPLE_COUNT)
                            m_tickTimesFilled++;
                        
                        
                        
                        

                        chunkPacketManagement_PostTick();
                    }
                    
                    
                    
                    
                    
                    
                }
            } else {
                
                
                
                
                
                
                while (unprocessedTime > MS_PER_TICK) {
                    unprocessedTime -= MS_PER_TICK;
                    
                    connection->tick();
                }
            }
            if (MinecraftServer::setTimeAtEndOfTick) {
                MinecraftServer::setTimeAtEndOfTick = false;
                for (unsigned int i = 0; i < levels.size(); i++) {
                    
                    
                    
                    {
                        ServerLevel* level = levels[i];
                        level->setGameTime(MinecraftServer::setTime);
                    }
                }
            }
            if (MinecraftServer::setTimeOfDayAtEndOfTick) {
                MinecraftServer::setTimeOfDayAtEndOfTick = false;
                for (unsigned int i = 0; i < levels.size(); i++) {
                    if (i == 0 || settings->getBoolean(L"allow-nether", true)) {
                        ServerLevel* level = levels[i];
                        level->setDayTime(MinecraftServer::setTimeOfDay);
                    }
                }
            }

            
            eXuiServerAction eAction;
            void* param;
            for (int i = 0; i < XUSER_MAX_COUNT; i++) {
                eAction = app.GetXuiServerAction(i);
                param = app.GetXuiServerActionParam(i);

                switch (eAction) {
                    case eXuiServerAction_AutoSaveGame:
                    case eXuiServerAction_SaveGame:
                        app.lockSaveNotification();
                        if (players != nullptr) {
                            players->saveAll(
                                Minecraft::GetInstance()->progressRenderer);
                        }

                        players->broadcastAll(
                            std::shared_ptr<UpdateProgressPacket>(
                                new UpdateProgressPacket(20)));

                        for (unsigned int j = 0; j < levels.size(); j++) {
                            if (s_bServerHalted) break;
                            
                            
                            
                            
                            
                            
                            ServerLevel* level = levels[levels.size() - 1 - j];
                            level->save(
                                true,
                                Minecraft::GetInstance()->progressRenderer,
                                (eAction == eXuiServerAction_AutoSaveGame));

                            players->broadcastAll(
                                std::shared_ptr<UpdateProgressPacket>(
                                    new UpdateProgressPacket(33 + (j * 33))));
                        }
                        if (!s_bServerHalted) {
                            saveGameRules();

                            levels[0]->saveToDisc(
                                Minecraft::GetInstance()->progressRenderer,
                                (eAction == eXuiServerAction_AutoSaveGame));
                        }
                        app.unlockSaveNotification();
                        break;
                    case eXuiServerAction_DropItem:
                        
                        {
                            std::shared_ptr<ServerPlayer> player =
                                players->players.at(0);
                            size_t id = (size_t)param;
                            player->drop(std::shared_ptr<ItemInstance>(
                                new ItemInstance(id, 1, 0)));
                        }
                        break;
                    case eXuiServerAction_SpawnMob: {
                        std::shared_ptr<ServerPlayer> player =
                            players->players.at(0);
                        eINSTANCEOF factory = (eINSTANCEOF)((size_t)param);
                        std::shared_ptr<Mob> mob =
                            std::dynamic_pointer_cast<Mob>(
                                EntityIO::newByEnumType(factory,
                                                        player->level));
                        mob->moveTo(player->x + 1, player->y, player->z + 1,
                                    player->level->random->nextFloat() * 360,
                                    0);
                        mob->setDespawnProtected();  
                                                     
                                                     
                                                     
                                                     
                        player->level->addEntity(mob);
                    } break;
                    case eXuiServerAction_PauseServer:
                        m_isServerPaused = ((size_t)param == true);
                        if (m_isServerPaused) {
                            m_serverPausedEvent->set();
                        }
                        break;
                    case eXuiServerAction_ToggleRain: {
                        bool isRaining = levels[0]->getLevelData()->isRaining();
                        levels[0]->getLevelData()->setRaining(!isRaining);
                        levels[0]->getLevelData()->setRainTime(
                            levels[0]->random->nextInt(Level::TICKS_PER_DAY *
                                                       7) +
                            Level::TICKS_PER_DAY / 2);
                    } break;
                    case eXuiServerAction_ToggleThunder: {
                        bool isThundering =
                            levels[0]->getLevelData()->isThundering();
                        levels[0]->getLevelData()->setThundering(!isThundering);
                        levels[0]->getLevelData()->setThunderTime(
                            levels[0]->random->nextInt(Level::TICKS_PER_DAY *
                                                       7) +
                            Level::TICKS_PER_DAY / 2);
                    } break;
                    case eXuiServerAction_ServerSettingChanged_Gamertags:
                        players->broadcastAll(
                            std::shared_ptr<ServerSettingsChangedPacket>(
                                new ServerSettingsChangedPacket(
                                    ServerSettingsChangedPacket::HOST_OPTIONS,
                                    app.GetGameHostOption(
                                        eGameHostOption_Gamertags))));
                        break;
                    case eXuiServerAction_ServerSettingChanged_BedrockFog:
                        players->broadcastAll(
                            std::shared_ptr<ServerSettingsChangedPacket>(
                                new ServerSettingsChangedPacket(
                                    ServerSettingsChangedPacket::
                                        HOST_IN_GAME_SETTINGS,
                                    app.GetGameHostOption(
                                        eGameHostOption_All))));
                        break;

                    case eXuiServerAction_ServerSettingChanged_Difficulty:
                        players->broadcastAll(std::shared_ptr<
                                              ServerSettingsChangedPacket>(
                            new ServerSettingsChangedPacket(
                                ServerSettingsChangedPacket::HOST_DIFFICULTY,
                                Minecraft::GetInstance()
                                    ->options->difficulty)));
                        break;
                    case eXuiServerAction_ExportSchematic:
#if !defined(_CONTENT_PACKAGE)
                        app.lockSaveNotification();

                        
                        
                        

                        if (!s_bServerHalted) {
                            ConsoleSchematicFile::XboxSchematicInitParam*
                                initData = (ConsoleSchematicFile::
                                                XboxSchematicInitParam*)param;
                            File targetFileDir(L"Schematics");
                            if (!targetFileDir.exists()) targetFileDir.mkdir();

                            wchar_t filename[128];
                            swprintf(filename, 128, L"%ls%dx%dx%d.sch",
                                     initData->name,
                                     (initData->endX - initData->startX + 1),
                                     (initData->endY - initData->startY + 1),
                                     (initData->endZ - initData->startZ + 1));

                            File dataFile =
                                File(targetFileDir, std::wstring(filename));
                            if (dataFile.exists()) dataFile._delete();
                            FileOutputStream fos = FileOutputStream(dataFile);
                            DataOutputStream dos = DataOutputStream(&fos);
                            ConsoleSchematicFile::generateSchematicFile(
                                &dos, levels[0], initData->startX,
                                initData->startY, initData->startZ,
                                initData->endX, initData->endY, initData->endZ,
                                initData->bSaveMobs, initData->compressionType);
                            dos.close();

                            delete initData;
                        }
                        app.unlockSaveNotification();
#endif
                        break;
                    case eXuiServerAction_SetCameraLocation:
#if !defined(_CONTENT_PACKAGE)
                    {
                        DebugSetCameraPosition* pos =
                            (DebugSetCameraPosition*)param;

                        app.DebugPrintf("DEBUG: Player=%i\n", pos->player);
                        app.DebugPrintf(
                            "DEBUG: Teleporting to pos=(%f.2, %f.2, %f.2), "
                            "looking at=(%f.2,%f.2)\n",
                            pos->m_camX, pos->m_camY, pos->m_camZ, pos->m_yRot,
                            pos->m_elev);

                        std::shared_ptr<ServerPlayer> player =
                            players->players.at(pos->player);
                        player->debug_setPosition(pos->m_camX, pos->m_camY,
                                                  pos->m_camZ, pos->m_yRot,
                                                  pos->m_elev);

                        
                        
                        
                        
                    }
#endif
                    break;
                    default:
                        break;
                }

                app.SetXuiServerAction(i, eXuiServerAction_Idle);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    
    
    
    
    
    
    
    

    
    stopServer(didInit);
    stopped = true;
}

void MinecraftServer::broadcastStartSavingPacket() {
    players->broadcastAll(std::shared_ptr<GameEventPacket>(
        new GameEventPacket(GameEventPacket::START_SAVING, 0)));
    ;
}

void MinecraftServer::broadcastStopSavingPacket() {
    if (!s_bServerHalted) {
        players->broadcastAll(std::shared_ptr<GameEventPacket>(
            new GameEventPacket(GameEventPacket::STOP_SAVING, 0)));
        ;
    }
}

void MinecraftServer::tick() {
    std::vector<std::wstring> toRemove;
    for (auto it = ironTimers.begin(); it != ironTimers.end(); it++) {
        int t = it->second;
        if (t > 0) {
            ironTimers[it->first] = t - 1;
        } else {
            toRemove.push_back(it->first);
        }
    }
    for (unsigned int i = 0; i < toRemove.size(); i++) {
        ironTimers.erase(toRemove[i]);
    }

    tickCount++;

    
    
    
    if ((tickCount % 40) == 0 && !levels.empty() && players != nullptr) {
        LevelData* ld = levels[0]->getLevelData();
        std::wstring worldName = ld ? ld->getLevelName() : std::wstring(L"");
        unsigned int port = 25565;
        if (const char* envP = std::getenv("MC_LISTEN_PORT")) {
            int p = atoi(envP);
            if (p > 0 && p < 65536) port = (unsigned int)p;
        }
        
        
        
        
        std::wstring motd;
        if (settings != nullptr) {
            motd = settings->getString(L"motd", L"");
        }
        if (const char* envM = std::getenv("MC_MOTD")) {
            std::string m(envM);
            motd.assign(m.begin(), m.end());
        }
        LanDiscovery::SetBeaconPayload(
            (uint16_t)port, (uint16_t)VER_NETWORK,
            (uint8_t)std::min(players->getPlayerCount(), 255),
            (uint8_t)std::min(players->getMaxPlayers(), 255),
            (uint8_t)(ld && ld->getGameType() ? ld->getGameType()->getId() : 0),
            g_NetworkManager.IsPrivateGame(), worldName, motd);

        
        
        
        unsigned int liveSettings =
            app.GetGameHostOption(eGameHostOption_All);
        for (unsigned int li = 0; li < levels.size(); ++li) {
            if (levels[li] != nullptr &&
                levels[li]->getLevelData() != nullptr) {
                levels[li]->getLevelData()->setGameHostSettings(liveSettings);
            }
        }
    }

    
    Minecraft* pMinecraft = Minecraft::GetInstance();
    
    







    for (unsigned int i = 0; i < levels.size(); i++) {
        
        
        {
            ServerLevel* level = levels[i];

            
            
            level->difficulty = app.GetGameHostOption(
                eGameHostOption_Difficulty);  

#if DEBUG_SERVER_DONT_SPAWN_MOBS
            level->setSpawnSettings(false, false);
#else
            level->setSpawnSettings(level->difficulty > 0 &&
                                        !Minecraft::GetInstance()->isTutorial(),
                                    animals);
#endif

            if (tickCount % 20 == 0) {
                players->broadcastAll(
                    std::make_shared<SetTimePacket>(
                        level->getGameTime(), level->getDayTime(),
                        level->getGameRules()->getBoolean(
                            GameRules::RULE_DAYLIGHT)),
                    level->dimension->id);
            }
            
            static int64_t stc = 0;
            int64_t st0 = System::currentTimeMillis();
            ((Level*)level)->tick();
            int64_t st1 = System::currentTimeMillis();

            int64_t st2 = System::currentTimeMillis();

            
            
            
            
            
            
            
            
            
            if ((players->getPlayerCount(level) > 0) ||
                (level->hasEntitiesToRemove())) {
                level->tickEntities();
            }

            level->getTracker()->tick();

            int64_t st3 = System::currentTimeMillis();
            
            
            stc = st0;
            
        }
    }
    Entity::tickExtraWandering();  

    connection->tick();

    players->tick();

    

    
    handleConsoleInputs();
    
    
    
    
}

void MinecraftServer::handleConsoleInput(const std::wstring& msg,
                                         ConsoleInputSource* source) {
    consoleInput.push_back(new ConsoleInput(msg, source));
}

void MinecraftServer::handleConsoleInputs() {
    while (consoleInput.size() > 0) {
        auto it = consoleInput.begin();
        ConsoleInput* input = *it;
        consoleInput.erase(it);
        
        
    }
}

void MinecraftServer::main(int64_t seed, void* lpParameter) {
    ShutdownManager::HasStarted(ShutdownManager::eServerThread);
    server = new MinecraftServer();
    server->run(seed, lpParameter);
    delete server;
    server = nullptr;
    ShutdownManager::HasFinished(ShutdownManager::eServerThread);
}

void MinecraftServer::HaltServer(bool bPrimaryPlayerSignedOut) {
    s_bServerHalted = true;
    if (server != nullptr) {
        m_bPrimaryPlayerSignedOut = bPrimaryPlayerSignedOut;
        server->halt();
    }
}

File* MinecraftServer::getFile(const std::wstring& name) {
    return new File(name);
}

void MinecraftServer::info(const std::wstring& string) {}

void MinecraftServer::warn(const std::wstring& string) {}

std::wstring MinecraftServer::getConsoleName() { return L"CONSOLE"; }

ServerLevel* MinecraftServer::getLevel(int dimension) {
    if (dimension == -1)
        return levels[1];
    else if (dimension == 1)
        return levels[2];
    else
        return levels[0];
}


void MinecraftServer::setLevel(int dimension, ServerLevel* level) {
    if (dimension == -1)
        levels[1] = level;
    else if (dimension == 1)
        levels[2] = level;
    else
        levels[0] = level;
}

#if defined(_ACK_CHUNK_SEND_THROTTLING)
bool MinecraftServer::chunkPacketManagement_CanSendTo(INetworkPlayer* player) {
    if (s_hasSentEnoughPackets) return false;
    if (player == nullptr) return false;

    for (int i = 0; i < s_sentTo.size(); i++) {
        if (s_sentTo[i]->IsSameSystem(player)) {
            return false;
        }
    }

    return (player->GetOutstandingAckCount() < 2);
}

void MinecraftServer::chunkPacketManagement_DidSendTo(INetworkPlayer* player) {
    int64_t currentTime = System::currentTimeMillis();

    if ((currentTime - s_tickStartTime) >= MAX_TICK_TIME_FOR_PACKET_SENDS) {
        s_hasSentEnoughPackets = true;
        
        
    } else {
        
        
    }

    player->SentChunkPacket();

    s_sentTo.push_back(player);
}

void MinecraftServer::chunkPacketManagement_PreTick() {
    
    s_hasSentEnoughPackets = false;
    s_tickStartTime = System::currentTimeMillis();
    s_sentTo.clear();

    std::vector<std::shared_ptr<PlayerConnection> >* players =
        connection->getPlayers();

    if (players->size()) {
        std::vector<std::shared_ptr<PlayerConnection> > playersOrig = *players;
        players->clear();

        do {
            int longestTime = 0;
            auto playerConnectionBest = playersOrig.begin();
            for (auto it = playersOrig.begin(); it != playersOrig.end(); it++) {
                int thisTime = 0;
                INetworkPlayer* np = (*it)->getNetworkPlayer();
                if (np) {
                    thisTime = np->GetTimeSinceLastChunkPacket_ms();
                }

                if (thisTime > longestTime) {
                    playerConnectionBest = it;
                    longestTime = thisTime;
                }
            }
            players->push_back(*playerConnectionBest);
            playersOrig.erase(playerConnectionBest);
        } while (playersOrig.size() > 0);
    }
}

void MinecraftServer::chunkPacketManagement_PostTick() {}

#else

bool MinecraftServer::chunkPacketManagement_CanSendTo(INetworkPlayer* player) {
    if (player == nullptr) return false;

    
    
    
    
    
    
    
    
    if (!player->IsLocal()) {
        return true;
    }

    auto now = time_util::clock::now();
    if (player->GetSessionIndex() == s_slowQueuePlayerIndex &&
        (now - s_slowQueueLastTime) > std::chrono::milliseconds(MINECRAFT_SERVER_SLOW_QUEUE_DELAY)) {
        
        
        return true;
    }

    return false;
}

void MinecraftServer::chunkPacketManagement_DidSendTo(INetworkPlayer* player) {
    s_slowQueuePacketSent = true;
}

void MinecraftServer::chunkPacketManagement_PreTick() {}

void MinecraftServer::chunkPacketManagement_PostTick() {
    
    
    auto now = time_util::clock::now();
    if ((s_slowQueuePacketSent) || ((now - s_slowQueueLastTime) >
                                    std::chrono::milliseconds(2 * MINECRAFT_SERVER_SLOW_QUEUE_DELAY))) {
        
        
        
        MinecraftServer::cycleSlowQueueIndex();
        s_slowQueuePacketSent = false;
        s_slowQueueLastTime = now;
    }
    
    
    
    
    
    
}

void MinecraftServer::cycleSlowQueueIndex() {
    if (!g_NetworkManager.IsInSession()) return;

    int startingIndex = s_slowQueuePlayerIndex;
    INetworkPlayer* currentPlayer = nullptr;
    int currentPlayerCount = 0;
    do {
        currentPlayerCount = g_NetworkManager.GetPlayerCount();
        if (startingIndex >= currentPlayerCount) startingIndex = 0;
        ++s_slowQueuePlayerIndex;

        if (currentPlayerCount > 0) {
            s_slowQueuePlayerIndex %= currentPlayerCount;
            
            
            
            
            currentPlayer =
                g_NetworkManager.GetPlayerByIndex(s_slowQueuePlayerIndex);
        } else {
            s_slowQueuePlayerIndex = 0;
        }
    } while (g_NetworkManager.IsInSession() && currentPlayerCount > 0 &&
             s_slowQueuePlayerIndex != startingIndex &&
             currentPlayer != nullptr && currentPlayer->IsLocal());
    
    
}
#endif






bool MinecraftServer::flagEntitiesToBeRemoved(unsigned int* flags) {
    bool removedFound = false;
    for (unsigned int i = 0; i < levels.size(); i++) {
        ServerLevel* level = levels[i];
        if (level) {
            level->flagEntitiesToBeRemoved(flags, &removedFound);
        }
    }
    return removedFound;
}
