#include "PlayerList.h"

#include <string.h>
#include <wchar.h>

#include <algorithm>
#include <cmath>
#include <compare>
#include <cstdint>

#include "platform/sdl2/Profile.h"
#include "platform/sdl2/Storage.h"
#include "app/common/App_enums.h"
#include "app/common/src/GameRules/LevelRules/RuleDefinitions/GameRuleDefinition.h"
#include "app/common/src/GameRules/LevelRules/RuleDefinitions/LevelRuleset.h"
#include "app/common/src/GameRules/LevelRules/Rules/GameRulesInstance.h"
#include "app/common/src/Network/GameNetworkManager.h"
#include "app/common/src/Network/NetworkPlayerInterface.h"
#include "app/common/src/Network/Socket.h"
#include "app/common/src/Tutorial/Tutorial.h"
#include "app/common/src/Tutorial/TutorialEnum.h"
#include "app/mac/MacGame.h"
#include "app/mac/Stubs/winapi_stubs.h"
#include "app/include/NetTypes.h"
#include "MinecraftServer.h"
#include "Settings.h"
#include "minecraft/world/entity/player/SkinTypes.h"
#include "java/Class.h"
#include "java/JavaMath.h"
#include "java/System.h"
#include "minecraft/Pos.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/multiplayer/MultiPlayerGameMode.h"
#include "minecraft/network/Connection.h"
#include "minecraft/network/packet/AddPlayerPacket.h"
#include "minecraft/network/packet/ChatPacket.h"
#include "minecraft/network/packet/DisconnectPacket.h"
#include "minecraft/network/packet/GameEventPacket.h"
#include "minecraft/network/packet/LoginPacket.h"
#include "minecraft/network/packet/PlayerAbilitiesPacket.h"
#include "minecraft/network/packet/PlayerInfoPacket.h"
#include "minecraft/network/packet/RemoveEntitiesPacket.h"
#include "minecraft/network/packet/RespawnPacket.h"
#include "minecraft/network/packet/SetCarriedItemPacket.h"
#include "minecraft/network/packet/SetExperiencePacket.h"
#include "minecraft/network/packet/SetSpawnPositionPacket.h"
#include "minecraft/network/packet/SetTimePacket.h"
#include "minecraft/network/packet/TextureAndGeometryPacket.h"
#include "minecraft/network/packet/TexturePacket.h"
#include "minecraft/network/packet/UpdateMobEffectPacket.h"
#include "minecraft/network/packet/XZPacket.h"
#include "minecraft/server/level/EntityTracker.h"
#include "minecraft/server/level/PlayerChunkMap.h"
#include "minecraft/server/level/ServerChunkCache.h"
#include "minecraft/server/level/ServerLevel.h"
#include "minecraft/server/level/ServerPlayer.h"
#include "minecraft/server/level/ServerPlayerGameMode.h"
#include "minecraft/server/network/PendingConnection.h"
#include "minecraft/server/network/PlayerConnection.h"
#include "minecraft/server/network/ServerConnection.h"
#include "minecraft/util/ProgressListener.h"
#include "minecraft/world/entity/Entity.h"
#include "minecraft/world/entity/EntityIO.h"
#include "minecraft/world/entity/Mob.h"
#include "minecraft/world/entity/SyncedEntityData.h"
#include "minecraft/world/entity/player/Inventory.h"
#include "minecraft/world/entity/player/Player.h"
#include "minecraft/world/item/Item.h"
#include "minecraft/world/item/ItemInstance.h"
#include "minecraft/world/level/ChunkPos.h"
#include "minecraft/world/level/GameRules.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/LevelSettings.h"
#include "minecraft/world/level/PortalForcer.h"
#include "minecraft/world/level/ViewDistanceUtil.h"
#include "minecraft/world/level/dimension/Dimension.h"
#include "minecraft/world/level/saveddata/MapItemSavedData.h"
#include "minecraft/world/level/storage/LevelData.h"
#include "minecraft/world/level/storage/LevelStorage.h"
#include "minecraft/world/level/storage/PlayerIO.h"
#include "nbt/CompoundTag.h"
#include "strings.h"

class MobEffectInstance;




PlayerList::PlayerList(MinecraftServer* server) {
    playerIo = nullptr;

    this->server = server;

    sendAllPlayerInfoIn = 0;
    overrideGameMode = nullptr;
    allowCheatsForAllPlayers = false;

    
    
    
    
    
    
    
    
#if defined(_LARGE_WORLDS)
    int defaultVD = 16;
#else
    int defaultVD = 10;
#endif

    int rawViewDistance = server->settings->getInt(L"view-distance", defaultVD);
    viewDistance = clampViewDistance(rawViewDistance);
    if (rawViewDistance != viewDistance) {
        app.DebugPrintf(
            "WARNING: view-distance %d out of range [%d, %d], clamped to %d\n",
            rawViewDistance, PlayerChunkMap::MIN_VIEW_DISTANCE,
            PlayerChunkMap::MAX_VIEW_DISTANCE, viewDistance);
    }

    
    
    
    
    
    server->settings->setIntAndSave(L"view-distance", viewDistance);

    maxPlayers = server->settings->getInt(L"max-players", 20);
    doWhiteList = false;

    
    
    
    
    
    m_lastFullDiskFlushMs = 0;
    m_mpAutosaveCountdown = 20 * 90;  
}

PlayerList::~PlayerList() {
    for (auto it = players.begin(); it < players.end(); it++) {
        (*it)->connection = nullptr;  
                                      
        delete (*it)->gameMode;  
                                 
        (*it)->gameMode = nullptr;
    }
}

void PlayerList::placeNewPlayer(Connection* connection,
                                std::shared_ptr<ServerPlayer> player,
                                std::shared_ptr<LoginPacket> packet) {
    CompoundTag* playerTag = load(player);

    bool newPlayer = playerTag == nullptr;

    player->setLevel(server->getLevel(player->dimension));
    player->gameMode->setLevel((ServerLevel*)player->level);

    
    INetworkPlayer* networkPlayer = connection->getSocket()->getPlayer();
    if (networkPlayer != nullptr && networkPlayer->IsHost()) {
        player->enableAllPlayerPrivileges(true);
        player->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_HOST, 1);
    }

    
    
    
    
    validatePlayerSpawnPosition(player);

    
    
    

    ServerLevel* level = server->getLevel(player->dimension);

    std::uint8_t playerIndex = 0;
    {
        bool usedIndexes[MINECRAFT_NET_MAX_PLAYERS];
        memset(&usedIndexes, 0, MINECRAFT_NET_MAX_PLAYERS * sizeof(bool));
        for (auto it = players.begin(); it < players.end(); ++it) {
            usedIndexes[(int)(*it)->getPlayerIndex()] = true;
        }
        for (unsigned int i = 0; i < MINECRAFT_NET_MAX_PLAYERS; ++i) {
            if (!usedIndexes[i]) {
                playerIndex = i;
                break;
            }
        }
    }
    player->setPlayerIndex(playerIndex);
    player->setCustomSkin(packet->m_playerSkinId);
    player->setCustomCape(packet->m_playerCapeId);

    
    
    std::shared_ptr<PlayerConnection> playerConnection =
        std::shared_ptr<PlayerConnection>(
            new PlayerConnection(server, connection, player));
    
    

    if (newPlayer) {
        int mapScale = 3;
#if defined(_LARGE_WORLDS)
        int scale = MapItemSavedData::MAP_SIZE * 2 * (1 << mapScale);
        int centreXC = (int)(Math::round(player->x / scale) * scale);
        int centreZC = (int)(Math::round(player->z / scale) * scale);
#else
        
        
        int centreXC = 0;
        int centreZC = 0;
#endif
        
        player->inventory->setItem(
            9, std::make_shared<ItemInstance>(
                   Item::map_Id, 1,
                   level->getAuxValueForMap(player->getXuid(), 0, centreXC,
                                            centreZC, mapScale)));
        if (app.getGameRuleDefinitions() != nullptr) {
            app.getGameRuleDefinitions()->postProcessPlayer(player);
        }
    }

    if (!player->customTextureUrl.empty() &&
        player->customTextureUrl.substr(0, 3).compare(L"def") != 0 &&
        !app.IsFileInMemoryTextures(player->customTextureUrl)) {
        if (server->getConnection()->addPendingTextureRequest(
                player->customTextureUrl)) {
#if !defined(_CONTENT_PACKAGE)
            wprintf(
                L"Sending texture packet to get custom skin %ls from player "
                L"%ls\n",
                player->customTextureUrl.c_str(), player->name.c_str());
#endif
            playerConnection->send(std::shared_ptr<TextureAndGeometryPacket>(
                new TextureAndGeometryPacket(player->customTextureUrl, nullptr,
                                             0)));
        }
    } else if (!player->customTextureUrl.empty() &&
               app.IsFileInMemoryTextures(player->customTextureUrl)) {
        
        app.AddMemoryTextureFile(player->customTextureUrl, nullptr, 0);
    }

    if (!player->customTextureUrl2.empty() &&
        player->customTextureUrl2.substr(0, 3).compare(L"def") != 0 &&
        !app.IsFileInMemoryTextures(player->customTextureUrl2)) {
        if (server->getConnection()->addPendingTextureRequest(
                player->customTextureUrl2)) {
#if !defined(_CONTENT_PACKAGE)
            wprintf(
                L"Sending texture packet to get custom skin %ls from player "
                L"%ls\n",
                player->customTextureUrl2.c_str(), player->name.c_str());
#endif
            playerConnection->send(std::shared_ptr<TexturePacket>(
                new TexturePacket(player->customTextureUrl2, nullptr, 0)));
        }
    } else if (!player->customTextureUrl2.empty() &&
               app.IsFileInMemoryTextures(player->customTextureUrl2)) {
        
        app.AddMemoryTextureFile(player->customTextureUrl2, nullptr, 0);
    }

    player->setIsGuest(packet->m_isGuest);

    Pos* spawnPos = level->getSharedSpawnPos();

    updatePlayerGameMode(player, nullptr, level);

    
    GameType* gameType = Player::getPlayerGamePrivilege(
                             player->getAllPlayerGamePrivileges(),
                             Player::ePlayerGamePrivilege_CreativeMode)
                             ? GameType::CREATIVE
                             : GameType::SURVIVAL;
    gameType = LevelSettings::validateGameType(gameType->getId());
    if (player->gameMode->getGameModeForPlayer() != gameType) {
        player->setPlayerGamePrivilege(
            Player::ePlayerGamePrivilege_CreativeMode,
            player->gameMode->getGameModeForPlayer()->getId());
    }

    
    
    
    player->connection =
        playerConnection;  
                           

    
    playerConnection->m_friendsOnlyUGC = packet->m_friendsOnlyUGC;
    playerConnection->m_offlineXUID = packet->m_offlineXuid;
    playerConnection->m_onlineXUID = packet->m_onlineXuid;

    
    
    if (packet->m_friendsOnlyUGC) ++server->m_ugcPlayersVersion;

    addPlayerToReceiving(player);

    playerConnection->send(std::make_shared<LoginPacket>(
        L"", player->entityId, level->getLevelData()->getGenerator(),
        level->getSeed(), player->gameMode->getGameModeForPlayer()->getId(),
        (uint8_t)level->dimension->id, (uint8_t)level->getMaxBuildHeight(),
        (uint8_t)getMaxPlayers(), level->difficulty,
        0 ,
        (uint8_t)playerIndex, level->useNewSeaLevel(),
        player->getAllPlayerGamePrivileges(),
        level->getLevelData()->getXZSize(),
        level->getLevelData()->getHellScale(),
        getViewDistance()));  
                              
    playerConnection->send(std::shared_ptr<SetSpawnPositionPacket>(
        new SetSpawnPositionPacket(spawnPos->x, spawnPos->y, spawnPos->z)));
    playerConnection->send(std::shared_ptr<PlayerAbilitiesPacket>(
        new PlayerAbilitiesPacket(&player->abilities)));
    playerConnection->send(std::shared_ptr<SetCarriedItemPacket>(
        new SetCarriedItemPacket(player->inventory->selected)));
    delete spawnPos;

    updateEntireScoreboard((ServerScoreboard*)level->getScoreboard(), player);

    sendLevelInfo(player, level);

    
    
    
    
    
    
    
    

    
    
    
    
    broadcastAll(std::shared_ptr<ChatPacket>(
        new ChatPacket(player->name, ChatPacket::e_ChatPlayerJoinedGame)));

    add(player);

    player->doTick(
        true, true,
        false);  
                 
    playerConnection->teleport(player->x, player->y, player->z, player->yRot,
                               player->xRot);
    if (!playerConnection->isLocal() && !g_NetworkManager.SystemFlagGet(
            playerConnection->getNetworkPlayer(),
            ServerPlayer::getFlagIndexForChunk(
                ChunkPos(player->xChunk, player->zChunk),
                player->level->dimension->id))) {
        player->doTick(true, true, true);
    }

    server->getConnection()->addPlayerConnection(playerConnection);
    playerConnection->send(std::make_shared<SetTimePacket>(
        level->getGameTime(), level->getDayTime(),
        level->getGameRules()->getBoolean(GameRules::RULE_DAYLIGHT)));

    auto activeEffects = player->getActiveEffects();
    for (auto it = activeEffects->begin(); it != activeEffects->end(); ++it) {
        MobEffectInstance* effect = *it;
        playerConnection->send(std::shared_ptr<UpdateMobEffectPacket>(
            new UpdateMobEffectPacket(player->entityId, effect)));
    }

    player->initMenu();

    if (playerTag != nullptr && playerTag->contains(Entity::RIDING_TAG)) {
        
        std::shared_ptr<Entity> mount = EntityIO::loadStatic(
            playerTag->getCompound(Entity::RIDING_TAG), level);
        if (mount != nullptr) {
            mount->forcedLoading = true;
            level->addEntity(mount);
            player->ride(mount);
            mount->forcedLoading = false;
        }
    }

    
    
    
    INetworkPlayer* thisPlayer = player->connection->getNetworkPlayer();
    if (thisPlayer != nullptr) {
        for (auto it = players.begin(); it != players.end(); ++it) {
            std::shared_ptr<ServerPlayer> servPlayer = *it;
            INetworkPlayer* checkPlayer =
                servPlayer->connection->getNetworkPlayer();
            if (thisPlayer != checkPlayer && checkPlayer != nullptr &&
                thisPlayer->IsSameSystem(checkPlayer) && servPlayer->wonGame) {
                player->wonGame = true;
                break;
            }
        }
    }
}

void PlayerList::updateEntireScoreboard(ServerScoreboard* scoreboard,
                                        std::shared_ptr<ServerPlayer> player) {
    

    
    
    
    
    

    
    
    

    
    
    
    

    
    
    
    

    
    
    
}

void PlayerList::setLevel(std::vector<ServerLevel*>& levels) {
    playerIo = levels[0]->getLevelStorage()->getPlayerIO();
}

void PlayerList::changeDimension(std::shared_ptr<ServerPlayer> player,
                                 ServerLevel* from) {
    ServerLevel* to = player->getLevel();

    if (from != nullptr) from->getChunkMap()->remove(player);
    to->getChunkMap()->add(player);

    to->cache->create(((int)player->x) >> 4, ((int)player->z) >> 4);
}

int PlayerList::getMaxRange() {
    return PlayerChunkMap::convertChunkRangeToBlock(getViewDistance());
}

CompoundTag* PlayerList::load(std::shared_ptr<ServerPlayer> player) {
    return playerIo->load(player);
}

void PlayerList::save(std::shared_ptr<ServerPlayer> player) {
    playerIo->save(player);
}







void PlayerList::validatePlayerSpawnPosition(
    std::shared_ptr<ServerPlayer> player) {
    
    
    
    app.DebugPrintf("Original pos is %f, %f, %f in dimension %d\n", player->x,
                    player->y, player->z, player->dimension);

    bool spawnForced = player->isRespawnForced();

    double targetX = 0;
    if (player->x < 0)
        targetX = std::ceil(player->x) - 0.5;
    else
        targetX = std::floor(player->x) + 0.5;

    double targetY = floor(player->y);

    double targetZ = 0;
    if (player->z < 0)
        targetZ = std::ceil(player->z) - 0.5;
    else
        targetZ = std::floor(player->z) + 0.5;

    player->setPos(targetX, targetY, targetZ);

    app.DebugPrintf("New pos is %f, %f, %f in dimension %d\n", player->x,
                    player->y, player->z, player->dimension);

    ServerLevel* level = server->getLevel(player->dimension);
    while (level->getCubes(player, &player->bb)->size() != 0) {
        player->setPos(player->x, player->y + 1, player->z);
    }
    app.DebugPrintf("Final pos is %f, %f, %f in dimension %d\n", player->x,
                    player->y, player->z, player->dimension);

    
    
    
    
    
    
    
    if (level->dimension->id == -1 && player->y > 125) {
        app.DebugPrintf(
            "Player in the nether tried to spawn at y = %f, moving to "
            "overworld\n",
            player->y);
        player->setLevel(server->getLevel(0));
        player->gameMode->setLevel(server->getLevel(0));
        player->dimension = 0;

        level = server->getLevel(player->dimension);

        Pos* levelSpawn = level->getSharedSpawnPos();
        player->setPos(levelSpawn->x, levelSpawn->y, levelSpawn->z);
        delete levelSpawn;

        Pos* bedPosition = player->getRespawnPosition();
        if (bedPosition != nullptr) {
            Pos* respawnPosition = Player::checkBedValidRespawnPosition(
                server->getLevel(player->dimension), bedPosition, spawnForced);
            if (respawnPosition != nullptr) {
                player->moveTo(respawnPosition->x + 0.5f,
                               respawnPosition->y + 0.1f,
                               respawnPosition->z + 0.5f, 0, 0);
                player->setRespawnPosition(bedPosition, spawnForced);
            }
            delete bedPosition;
        }
        while (level->getCubes(player, &player->bb)->size() != 0) {
            player->setPos(player->x, player->y + 1, player->z);
        }

        app.DebugPrintf("Updated pos is %f, %f, %f in dimension %d\n",
                        player->x, player->y, player->z, player->dimension);
    }
}

void PlayerList::add(std::shared_ptr<ServerPlayer> player) {
    
    
    if (player->connection->getNetworkPlayer()) {
        broadcastAll(std::make_shared<PlayerInfoPacket>(player));
    }

    players.push_back(player);

    
    addPlayerToReceiving(player);

    
    ServerLevel* level = server->getLevel(player->dimension);

    
    
    
    
    

    
    
    
    changeDimension(player, nullptr);
    level->addEntity(player);

    for (int i = 0; i < players.size(); i++) {
        std::shared_ptr<ServerPlayer> op = players.at(i);
        
        
        if (op->connection->getNetworkPlayer()) {
            player->connection->send(std::make_shared<PlayerInfoPacket>(op));
        }
    }

    
    
    
    
    
    
    
    
    
    
    auto buildAddPacket = [](std::shared_ptr<ServerPlayer> p) {
        int xp = (int)std::floor(p->x * 32.0);
        int yp = (int)std::floor(p->y * 32.0);
        int zp = (int)std::floor(p->z * 32.0);
        int yRotp = (int)std::floor(p->yRot * 256.0f / 360.0f);
        int xRotp = (int)std::floor(p->xRot * 256.0f / 360.0f);
        int yHeadRotp =
            (int)std::floor(p->getYHeadRot() * 256.0f / 360.0f);
        return std::make_shared<AddPlayerPacket>(
            std::dynamic_pointer_cast<Player>(p), p->getXuid(),
            p->getOnlineXuid(), xp, yp, zp, yRotp, xRotp, yHeadRotp);
    };

    if (player->connection->getNetworkPlayer()) {
        
        
        
        
        
        
        
        auto joinerPacket = buildAddPacket(player);
        for (auto& op : players) {
            if (op == player) continue;
            if (op->connection != nullptr &&
                op->connection->getNetworkPlayer()) {
                op->connection->send(joinerPacket);
            }
        }

        
        
        for (auto& op : players) {
            if (op == player) continue;
            if (op->connection != nullptr &&
                op->connection->getNetworkPlayer()) {
                player->connection->send(buildAddPacket(op));
            }
        }
    }

    if (level->isAtLeastOnePlayerSleeping()) {
        std::shared_ptr<ServerPlayer> firstSleepingPlayer = nullptr;
        for (unsigned int i = 0; i < players.size(); i++) {
            std::shared_ptr<ServerPlayer> thisPlayer = players[i];
            if (thisPlayer->isSleeping()) {
                if (firstSleepingPlayer == nullptr)
                    firstSleepingPlayer = thisPlayer;
                thisPlayer->connection->send(std::make_shared<ChatPacket>(
                    thisPlayer->name, ChatPacket::e_ChatBedMeSleep));
            }
        }
        player->connection->send(std::make_shared<ChatPacket>(
            firstSleepingPlayer->name, ChatPacket::e_ChatBedPlayerSleep));
    }
}

void PlayerList::move(std::shared_ptr<ServerPlayer> player) {
    player->getLevel()->getChunkMap()->move(player);
}

void PlayerList::remove(std::shared_ptr<ServerPlayer> player) {
    save(player);

    
    
    
    
    
    
    
    
    if (playerIo != nullptr) {
        playerIo->saveAllCachedData();
        playerIo->saveMapIdLookup();

        int64_t nowMs = System::currentTimeMillis();
        if (nowMs - m_lastFullDiskFlushMs > 60 * 1000) {
            m_lastFullDiskFlushMs = nowMs;
            ServerLevel* level0 = server->getLevel(0);
            if (level0 != nullptr) {
                level0->saveToDisc(nullptr, true);  
            }
        }
    }

    
    
    if (player->isGuest()) playerIo->deleteMapFilesForPlayer(player);
    ServerLevel* level = player->getLevel();
    if (player->riding != nullptr) {
        
        
        
        level->removeEntityImmediately(player->riding);
        app.DebugPrintf("removing player mount");
    }
    level->removeEntity(player);
    level->getChunkMap()->remove(player);
    auto it = find(players.begin(), players.end(), player);
    if (it != players.end()) {
        players.erase(it);
    }
    
    

    
    
    
    
    
    {
        std::vector<int> ids;
        ids.push_back(player->entityId);
        broadcastAll(std::make_shared<RemoveEntitiesPacket>(ids));
    }

    removePlayerFromReceiving(player);
    player->connection = nullptr;  
                                   
    delete player->gameMode;  
                              
    player->gameMode = nullptr;

    
    
    saveAll(nullptr, false);
}

std::shared_ptr<ServerPlayer> PlayerList::getPlayerForLogin(
    PendingConnection* pendingConnection, const std::wstring& userName,
    PlayerUID xuid, PlayerUID onlineXuid) {
    if (players.size() >= maxPlayers) {
        pendingConnection->disconnect(DisconnectPacket::eDisconnect_ServerFull);
        return std::shared_ptr<ServerPlayer>();
    }

    std::shared_ptr<ServerPlayer> player = std::shared_ptr<ServerPlayer>(
        new ServerPlayer(server, server->getLevel(0), userName,
                         new ServerPlayerGameMode(server->getLevel(0))));
    player->gameMode->player = player;  
                                        

    
    
    
    
    
    
    
    
    
    PlayerUID effectiveXuid = xuid;
    {
        INetworkPlayer* np =
            pendingConnection->connection != nullptr &&
                    pendingConnection->connection->getSocket() != nullptr
                ? pendingConnection->connection->getSocket()->getPlayer()
                : nullptr;
        bool isHost = (np != nullptr && np->IsHost());
        if (!isHost && !userName.empty()) {
            
            
            
            uint64_t h = 1469598103934665603ULL;
            for (wchar_t wc : userName) {
                h ^= (uint64_t)(uint32_t)wc;
                h *= 1099511628211ULL;
            }
            effectiveXuid =
                (PlayerUID)((h & 0x0fffffffffffffffULL) |
                            0xC000000000000000ULL);
            fprintf(stderr,
                    "[NET] Remote player '%ls' mapped to stable save XUID "
                    "0x%016llx\n",
                    userName.c_str(), (unsigned long long)effectiveXuid);
        }
    }
    player->setXuid(effectiveXuid);     
    player->setOnlineXuid(onlineXuid);  

    
    INetworkPlayer* networkPlayer =
        pendingConnection->connection->getSocket()->getPlayer();
    if (networkPlayer != nullptr && !networkPlayer->IsHost()) {
        player->enableAllPlayerPrivileges(
            app.GetGameHostOption(eGameHostOption_TrustPlayers) > 0);
    }

    
    LevelRuleset* serverRuleDefs = app.getGameRuleDefinitions();
    if (serverRuleDefs != nullptr) {
        player->gameMode->setGameRules(
            GameRuleDefinition::generateNewGameRulesInstance(
                GameRulesInstance::eGameRulesInstanceType_ServerPlayer,
                serverRuleDefs, pendingConnection->connection));
    }

    return player;
}

std::shared_ptr<ServerPlayer> PlayerList::respawn(
    std::shared_ptr<ServerPlayer> serverPlayer, int targetDimension,
    bool keepAllPlayerData) {
    
    
    
    
    bool deathHadBack = serverPlayer->m_hasBack;
    double deathBackX = serverPlayer->m_backX;
    double deathBackY = serverPlayer->m_backY;
    double deathBackZ = serverPlayer->m_backZ;
    int deathBackDim = serverPlayer->m_backDim;
    bool deathHadHome = serverPlayer->m_hasHome;
    double deathHomeX = serverPlayer->m_homeX;
    double deathHomeY = serverPlayer->m_homeY;
    double deathHomeZ = serverPlayer->m_homeZ;
    int deathHomeDim = serverPlayer->m_homeDim;
    std::wstring deathReplyTo = serverPlayer->m_lastReplyTo;
    
    bool dyingHasBack = true;
    double dyingBackX = serverPlayer->x;
    double dyingBackY = serverPlayer->y;
    double dyingBackZ = serverPlayer->z;
    int dyingBackDim =
        (serverPlayer->level != nullptr &&
         serverPlayer->level->dimension != nullptr)
            ? serverPlayer->level->dimension->id
            : 0;
    
    
    
    bool isPrimary = canReceiveAllPackets(
        serverPlayer);  
    int oldDimension = serverPlayer->dimension;
    bool isEmptying =
        (targetDimension !=
         oldDimension);  
                         

    
    
    if (isEmptying) {
        INetworkPlayer* thisPlayer =
            serverPlayer->connection->getNetworkPlayer();

        for (unsigned int i = 0; i < players.size(); i++) {
            std::shared_ptr<ServerPlayer> ep = players[i];
            if (ep == serverPlayer) continue;
            if (ep->dimension != oldDimension) continue;

            INetworkPlayer* otherPlayer = ep->connection->getNetworkPlayer();
            if (otherPlayer != nullptr &&
                thisPlayer->IsSameSystem(otherPlayer)) {
                
                
                isEmptying = false;
            }
        }
    }

    
    
    
    
    
    
    

    if (isPrimary) {
        if (isEmptying) {
            app.DebugPrintf("Emptying this dimension\n");
            serverPlayer->getLevel()->getTracker()->clear(serverPlayer);
        } else {
            app.DebugPrintf("Transferring... storing flags\n");
            serverPlayer->getLevel()->getTracker()->removeEntity(serverPlayer);
        }
    } else {
        app.DebugPrintf("Not primary player\n");
        serverPlayer->getLevel()->getTracker()->removeEntity(serverPlayer);
    }

    serverPlayer->getLevel()->getChunkMap()->remove(serverPlayer);
    auto it = find(players.begin(), players.end(), serverPlayer);
    if (it != players.end()) {
        players.erase(it);
    }
    server->getLevel(serverPlayer->dimension)
        ->removeEntityImmediately(serverPlayer);

    Pos* bedPosition = serverPlayer->getRespawnPosition();
    bool spawnForced = serverPlayer->isRespawnForced();

    removePlayerFromReceiving(serverPlayer);
    serverPlayer->dimension = targetDimension;

    EDefaultSkins skin = serverPlayer->getPlayerDefaultSkin();
    std::uint8_t playerIndex = serverPlayer->getPlayerIndex();

    PlayerUID playerXuid = serverPlayer->getXuid();
    PlayerUID playerOnlineXuid = serverPlayer->getOnlineXuid();

    std::shared_ptr<ServerPlayer> player = std::shared_ptr<ServerPlayer>(
        new ServerPlayer(server, server->getLevel(serverPlayer->dimension),
                         serverPlayer->getName(),
                         new ServerPlayerGameMode(
                             server->getLevel(serverPlayer->dimension))));
    player->connection = serverPlayer->connection;
    player->restoreFrom(serverPlayer, keepAllPlayerData);
    if (keepAllPlayerData) {
        
        
        
        player->inventory->selected = serverPlayer->inventory->selected;
    }
    player->gameMode->player = player;  
                                        
    player->setXuid(playerXuid);        
    player->setOnlineXuid(playerOnlineXuid);  

    
    
    player->m_hasHome = deathHadHome;
    player->m_homeX = deathHomeX;
    player->m_homeY = deathHomeY;
    player->m_homeZ = deathHomeZ;
    player->m_homeDim = deathHomeDim;
    
    
    if (dyingHasBack) {
        player->m_hasBack = true;
        player->m_backX = dyingBackX;
        player->m_backY = dyingBackY;
        player->m_backZ = dyingBackZ;
        player->m_backDim = dyingBackDim;
    } else {
        player->m_hasBack = deathHadBack;
        player->m_backX = deathBackX;
        player->m_backY = deathBackY;
        player->m_backZ = deathBackZ;
        player->m_backDim = deathBackDim;
    }
    player->m_lastReplyTo = deathReplyTo;

    
    
    
    

    player->setPlayerDefaultSkin(skin);
    player->setIsGuest(serverPlayer->isGuest());
    player->setPlayerIndex(playerIndex);
    player->setCustomSkin(serverPlayer->getCustomSkin());
    player->setCustomCape(serverPlayer->getCustomCape());
    player->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_All,
                                   serverPlayer->getAllPlayerGamePrivileges());
    player->gameMode->setGameRules(serverPlayer->gameMode->getGameRules());
    player->dimension = targetDimension;

    
    
    
    player->connection->setPlayer(player);

    addPlayerToReceiving(player);

    ServerLevel* level = server->getLevel(serverPlayer->dimension);

    
    
    updatePlayerGameMode(player, serverPlayer, level);

    if (serverPlayer->wonGame && targetDimension == oldDimension &&
        serverPlayer->getHealth() > 0) {
        
        
        
        player->moveTo(serverPlayer->x, serverPlayer->y, serverPlayer->z,
                       serverPlayer->yRot, serverPlayer->xRot);
        if (bedPosition != nullptr) {
            player->setRespawnPosition(bedPosition, spawnForced);
            delete bedPosition;
        }
        
        
        
        player->inventory->selected = serverPlayer->inventory->selected;
    } else if (bedPosition != nullptr) {
        Pos* respawnPosition = Player::checkBedValidRespawnPosition(
            server->getLevel(serverPlayer->dimension), bedPosition,
            spawnForced);
        if (respawnPosition != nullptr) {
            player->moveTo(respawnPosition->x + 0.5f, respawnPosition->y + 0.1f,
                           respawnPosition->z + 0.5f, 0, 0);
            player->setRespawnPosition(bedPosition, spawnForced);
        } else {
            player->connection->send(std::make_shared<GameEventPacket>(
                GameEventPacket::NO_RESPAWN_BED_AVAILABLE, 0));
        }
        delete bedPosition;
    }

    
    level->cache->create(((int)player->x) >> 4, ((int)player->z) >> 4);

    while (!level->getCubes(player, &player->bb)->empty()) {
        player->setPos(player->x, player->y + 1, player->z);
    }

    player->connection->send(std::make_shared<RespawnPacket>(
        (char)player->dimension, player->level->getSeed(),
        player->level->getMaxBuildHeight(),
        player->gameMode->getGameModeForPlayer(), level->difficulty,
        level->getLevelData()->getGenerator(), player->level->useNewSeaLevel(),
        player->entityId, level->getLevelData()->getXZSize(),
        level->getLevelData()->getHellScale()));
    player->connection->teleport(player->x, player->y, player->z, player->yRot,
                                 player->xRot);
    player->connection->send(std::make_shared<SetExperiencePacket>(
        player->experienceProgress, player->totalExperience,
        player->experienceLevel));

    if (keepAllPlayerData) {
        std::vector<MobEffectInstance*>* activeEffects =
            player->getActiveEffects();
        for (auto it = activeEffects->begin(); it != activeEffects->end();
             ++it) {
            MobEffectInstance* effect = *it;

            player->connection->send(std::shared_ptr<UpdateMobEffectPacket>(
                new UpdateMobEffectPacket(player->entityId, effect)));
        }
        delete activeEffects;
        player->getEntityData()->markDirty(Mob::DATA_EFFECT_COLOR_ID);
    }

    sendLevelInfo(player, level);

    level->getChunkMap()->add(player);
    level->addEntity(player);
    players.push_back(player);

    player->initMenu();
    player->setHealth(player->getHealth());

    
    
    if (Minecraft::GetInstance()->isTutorial() &&
        (!Minecraft::GetInstance()->gameMode->getTutorial()->isStateCompleted(
            e_Tutorial_State_Food_Bar))) {
        app.getGameRuleDefinitions()->postProcessPlayer(player);
    }

    if (oldDimension == 1 && player->dimension != 1) {
        player->displayClientMessage(IDS_PLAYER_LEFT_END);
    }

    return player;
}

void PlayerList::toggleDimension(std::shared_ptr<ServerPlayer> player,
                                 int targetDimension) {
    int lastDimension = player->dimension;
    
    
    
    bool isPrimary = canReceiveAllPackets(
        player);  
    bool isEmptying = true;

    
    
    INetworkPlayer* thisPlayer = player->connection->getNetworkPlayer();

    for (unsigned int i = 0; i < players.size(); i++) {
        std::shared_ptr<ServerPlayer> ep = players[i];
        if (ep == player) continue;
        if (ep->dimension != lastDimension) continue;

        INetworkPlayer* otherPlayer = ep->connection->getNetworkPlayer();
        if (otherPlayer != nullptr && thisPlayer->IsSameSystem(otherPlayer)) {
            
            
            isEmptying = false;
        }
    }

    
    
    
    
    
    
    

    if (isPrimary) {
        if (isEmptying) {
            app.DebugPrintf("Toggle... Emptying this dimension\n");
            player->getLevel()->getTracker()->clear(player);
        } else {
            app.DebugPrintf("Toggle...  transferring\n");
            player->getLevel()->getTracker()->removeEntity(player);
        }
    } else {
        app.DebugPrintf("Toggle...  Not primary player\n");
        player->getLevel()->getTracker()->removeEntity(player);
    }

    ServerLevel* oldLevel = server->getLevel(player->dimension);

    
    
    player->getLevel()->getChunkMap()->remove(player);

    if (player->dimension != 1 && targetDimension == 1) {
        player->displayClientMessage(IDS_PLAYER_ENTERED_END);
    } else if (player->dimension == 1) {
        player->displayClientMessage(IDS_PLAYER_LEFT_END);
    }

    player->dimension = targetDimension;

    ServerLevel* newLevel = server->getLevel(player->dimension);

    
    
    player->clearFire();  

    
    
    player->flushEntitiesToRemove();

    player->connection->send(std::make_shared<RespawnPacket>(
        (char)player->dimension, newLevel->getSeed(),
        newLevel->getMaxBuildHeight(), player->gameMode->getGameModeForPlayer(),
        newLevel->difficulty, newLevel->getLevelData()->getGenerator(),
        newLevel->useNewSeaLevel(), player->entityId,
        newLevel->getLevelData()->getXZSize(),
        newLevel->getLevelData()->getHellScale()));

    oldLevel->removeEntityImmediately(player);
    player->removed = false;

    repositionAcrossDimension(player, lastDimension, oldLevel, newLevel);
    changeDimension(player, oldLevel);

    player->gameMode->setLevel(newLevel);

    
    if (!g_NetworkManager.SystemFlagGet(
            player->connection->getNetworkPlayer(),
            ServerPlayer::getFlagIndexForChunk(
                ChunkPos(player->xChunk, player->zChunk),
                player->level->dimension->id))) {
        player->connection->teleport(player->x, player->y, player->z,
                                     player->yRot, player->xRot, false);
        
        player->doTick(true, true, true);
    }

    player->connection->teleport(player->x, player->y, player->z, player->yRot,
                                 player->xRot);

    
    
    std::vector<MobEffectInstance*>* activeEffects = player->getActiveEffects();
    for (auto it = activeEffects->begin(); it != activeEffects->end(); ++it) {
        MobEffectInstance* effect = *it;

        player->connection->send(std::shared_ptr<UpdateMobEffectPacket>(
            new UpdateMobEffectPacket(player->entityId, effect)));
    }
    delete activeEffects;
    player->getEntityData()->markDirty(Mob::DATA_EFFECT_COLOR_ID);

    sendLevelInfo(player, newLevel);
    sendAllPlayerInfo(player);
}

void PlayerList::repositionAcrossDimension(std::shared_ptr<Entity> entity,
                                           int lastDimension,
                                           ServerLevel* oldLevel,
                                           ServerLevel* newLevel) {
    double xt = entity->x;
    double zt = entity->z;
    double xOriginal = entity->x;
    double yOriginal = entity->y;
    double zOriginal = entity->z;
    float yRotOriginal = entity->yRot;
    double scale =
        newLevel->getLevelData()
            ->getHellScale();  
    if (entity->dimension == -1) {
        xt /= scale;
        zt /= scale;
        entity->moveTo(xt, entity->y, zt, entity->yRot, entity->xRot);
        if (entity->isAlive()) {
            oldLevel->tick(entity, false);
        }
    } else if (entity->dimension == 0) {
        xt *= scale;
        zt *= scale;
        entity->moveTo(xt, entity->y, zt, entity->yRot, entity->xRot);
        if (entity->isAlive()) {
            oldLevel->tick(entity, false);
        }
    } else {
        Pos* p;

        if (lastDimension == 1) {
            
            p = newLevel->getSharedSpawnPos();
        } else {
            
            p = newLevel->getDimensionSpecificSpawn();
        }

        xt = p->x;
        entity->y = p->y;
        zt = p->z;
        delete p;
        entity->moveTo(xt, entity->y, zt, 90, 0);
        if (entity->isAlive()) {
            oldLevel->tick(entity, false);
        }
    }

    if (entity->GetType() == eTYPE_SERVERPLAYER) {
        std::shared_ptr<ServerPlayer> player =
            std::dynamic_pointer_cast<ServerPlayer>(entity);
        removePlayerFromReceiving(player, false, lastDimension);
        addPlayerToReceiving(player);
    }

    if (lastDimension != 1) {
        xt = (double)std::clamp((int)xt, -Level::MAX_LEVEL_SIZE + 128,
                                Level::MAX_LEVEL_SIZE - 128);
        zt = (double)std::clamp((int)zt, -Level::MAX_LEVEL_SIZE + 128,
                                Level::MAX_LEVEL_SIZE - 128);
        if (entity->isAlive()) {
            newLevel->addEntity(entity);
            entity->moveTo(xt, entity->y, zt, entity->yRot, entity->xRot);
            newLevel->tick(entity, false);
            newLevel->cache->autoCreate = true;
            newLevel->getPortalForcer()->force(entity, xOriginal, yOriginal,
                                               zOriginal, yRotOriginal);
            newLevel->cache->autoCreate = false;
        }
    }

    entity->setLevel(newLevel);
}

void PlayerList::tick() {
    
    if (++sendAllPlayerInfoIn > SEND_PLAYER_INFO_INTERVAL) {
        sendAllPlayerInfoIn = 0;
    }

    
    
    
    
    
    
    bool hasRemotePlayer = false;
    for (size_t i = 0; i < players.size(); i++) {
        std::shared_ptr<ServerPlayer> sp = players[i];
        if (sp == nullptr || sp->connection == nullptr ||
            sp->connection->connection == nullptr) {
            continue;
        }
        Socket* sock = sp->connection->connection->getSocket();
        if (sock != nullptr) {
            INetworkPlayer* np = sock->getPlayer();
            if (np != nullptr && !np->IsHost()) {
                hasRemotePlayer = true;
                break;
            }
        }
    }
    if (hasRemotePlayer) {
        if (--m_mpAutosaveCountdown <= 0) {
            m_mpAutosaveCountdown = 20 * 90;  
            if (playerIo != nullptr &&
                !StorageManager.GetSaveDisabled()) {
                playerIo->saveAllCachedData();
                playerIo->saveMapIdLookup();
                ServerLevel* level0 = server->getLevel(0);
                if (level0 != nullptr) {
                    level0->saveToDisc(nullptr, true);
                }
                m_lastFullDiskFlushMs = System::currentTimeMillis();
            }
        }
    } else {
        
        
        
        m_mpAutosaveCountdown = 20 * 90;
    }

    if (sendAllPlayerInfoIn < players.size()) {
        std::shared_ptr<ServerPlayer> op = players[sendAllPlayerInfoIn];
        
        
        
        
        
        if (op != nullptr && op->connection != nullptr &&
            op->connection->getNetworkPlayer()) {
            broadcastAll(std::make_shared<PlayerInfoPacket>(op));
        }
    }

    {
        std::lock_guard<std::mutex> lock(m_closePlayersCS);
        while (!m_smallIdsToClose.empty()) {
            std::uint8_t smallId = m_smallIdsToClose.front();
            m_smallIdsToClose.pop_front();

            std::shared_ptr<ServerPlayer> player = nullptr;

            for (unsigned int i = 0; i < players.size(); i++) {
                std::shared_ptr<ServerPlayer> p = players.at(i);
                
                
                if (p != nullptr && p->connection != nullptr &&
                    p->connection->connection != nullptr &&
                    p->connection->connection->getSocket() != nullptr &&
                    p->connection->connection->getSocket()->getSmallId() ==
                        smallId) {
                    player = p;
                    break;
                }
            }

            if (player != nullptr) {
                player->connection->disconnect(
                    DisconnectPacket::eDisconnect_Closed);
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(m_kickPlayersCS);
        while (!m_smallIdsToKick.empty()) {
            std::uint8_t smallId = m_smallIdsToKick.front();
            m_smallIdsToKick.pop_front();
            INetworkPlayer* selectedPlayer =
                g_NetworkManager.GetPlayerBySmallId(smallId);
            if (selectedPlayer != nullptr) {
                if (selectedPlayer->IsLocal() != true) {
                    
                    PlayerUID xuid = selectedPlayer->GetUID();
                    
                    std::shared_ptr<ServerPlayer> player = nullptr;

                    for (unsigned int i = 0; i < players.size(); i++) {
                        std::shared_ptr<ServerPlayer> p = players.at(i);
                        PlayerUID playersXuid = p->getOnlineXuid();
                        if (p != nullptr &&
                            ProfileManager.AreXUIDSEqual(playersXuid, xuid)) {
                            player = p;
                            break;
                        }
                    }

                    if (player != nullptr) {
                        m_bannedXuids.push_back(player->getOnlineXuid());
                        
                        
                        
                        player->enableAllPlayerPrivileges(false);
                        player->connection->setWasKicked();
                        player->connection->send(
                            std::shared_ptr<DisconnectPacket>(
                                new DisconnectPacket(
                                    DisconnectPacket::eDisconnect_Kicked)));
                    }
                    
                }
            }
        }
    }

    
    
    for (unsigned int dim = 0; dim < 2; ++dim) {
        for (unsigned int i = 0; i < receiveAllPlayers[dim].size(); ++i) {
            std::shared_ptr<ServerPlayer> currentPlayer =
                receiveAllPlayers[dim][i];
            if (currentPlayer->removed) {
                std::shared_ptr<ServerPlayer> newPlayer =
                    findAlivePlayerOnSystem(currentPlayer);
                if (newPlayer != nullptr) {
                    receiveAllPlayers[dim][i] = newPlayer;
                    app.DebugPrintf(
                        "Replacing primary player %ls with %ls in dimension "
                        "%d\n",
                        currentPlayer->name.c_str(), newPlayer->name.c_str(),
                        dim);
                }
            }
        }
    }
}

bool PlayerList::isTrackingTile(int x, int y, int z, int dimension) {
    return server->getLevel(dimension)->getChunkMap()->isTrackingTile(x, y, z);
}



void PlayerList::prioritiseTileChanges(int x, int y, int z, int dimension) {
    server->getLevel(dimension)->getChunkMap()->prioritiseTileChanges(x, y, z);
}

void PlayerList::broadcastAll(std::shared_ptr<Packet> packet) {
    
    
    
    
    
    std::vector<std::shared_ptr<ServerPlayer> > snapshot = players;
    for (auto& player : snapshot) {
        if (player == nullptr) continue;
        auto conn = player->connection;
        if (conn == nullptr) continue;
        conn->send(packet);
    }
}

void PlayerList::broadcastAll(std::shared_ptr<Packet> packet, int dimension) {
    std::vector<std::shared_ptr<ServerPlayer> > snapshot = players;
    for (auto& player : snapshot) {
        if (player == nullptr) continue;
        if (player->dimension != dimension) continue;
        auto conn = player->connection;
        if (conn == nullptr) continue;
        conn->send(packet);
    }
}

std::wstring PlayerList::getPlayerNames() {
    std::wstring msg;
    for (unsigned int i = 0; i < players.size(); i++) {
        if (i > 0) msg += L", ";
        msg += players[i]->name;
    }
    return msg;
}

bool PlayerList::isWhiteListed(const std::wstring& name) { return true; }

bool PlayerList::isOp(const std::wstring& name) { return false; }

bool PlayerList::isOp(std::shared_ptr<ServerPlayer> player) {
    bool cheatsEnabled = app.GetGameHostOption(eGameHostOption_CheatsEnabled);
#if defined(_DEBUG_MENUS_ENABLED)
    cheatsEnabled = cheatsEnabled || app.GetUseDPadForDebug();
#endif
    INetworkPlayer* networkPlayer = player->connection->getNetworkPlayer();
    bool isOp = cheatsEnabled &&
                (player->isModerator() ||
                 (networkPlayer != nullptr && networkPlayer->IsHost()));
    return isOp;
}

std::shared_ptr<ServerPlayer> PlayerList::getPlayer(const std::wstring& name) {
    for (unsigned int i = 0; i < players.size(); i++) {
        std::shared_ptr<ServerPlayer> p = players[i];
        if (p->name ==
            name)  
                   
        {
            return p;
        }
    }
    return nullptr;
}


std::shared_ptr<ServerPlayer> PlayerList::getPlayer(PlayerUID uid) {
    for (unsigned int i = 0; i < players.size(); i++) {
        std::shared_ptr<ServerPlayer> p = players[i];
        if (p->getXuid() == uid ||
            p->getOnlineXuid() == uid)  
                                        
                                        
        {
            return p;
        }
    }
    return nullptr;
}

std::shared_ptr<ServerPlayer> PlayerList::getNearestPlayer(Pos* position,
                                                           int range) {
    if (players.empty()) return nullptr;
    if (position == nullptr) return players.at(0);
    std::shared_ptr<ServerPlayer> current = nullptr;
    double dist = -1;
    int rangeSqr = range * range;

    for (int i = 0; i < players.size(); i++) {
        std::shared_ptr<ServerPlayer> next = players.at(i);
        double newDist =
            position->distSqr(next->getCommandSenderWorldPosition());

        if ((dist == -1 || newDist < dist) &&
            (range <= 0 || newDist <= rangeSqr)) {
            dist = newDist;
            current = next;
        }
    }

    return current;
}

std::vector<ServerPlayer>* PlayerList::getPlayers(
    Pos* position, int rangeMin, int rangeMax, int count, int mode,
    int levelMin, int levelMax,
    std::unordered_map<std::wstring, int>* scoreRequirements,
    const std::wstring& playerName, const std::wstring& teamName,
    Level* level) {
    app.DebugPrintf("getPlayers NOT IMPLEMENTED!");
    return nullptr;

    














































}

bool PlayerList::meetsScoreRequirements(
    std::shared_ptr<Player> player,
    std::unordered_map<std::wstring, int> scoreRequirements) {
    app.DebugPrintf("meetsScoreRequirements NOT IMPLEMENTED!");
    return false;

    
    

    
    
    

    
    
    
    

    
    
    
    
    

    
    
    
    
    
    

    
}

void PlayerList::sendMessage(const std::wstring& name,
                             const std::wstring& message) {
    std::shared_ptr<ServerPlayer> player = getPlayer(name);
    if (player != nullptr) {
        player->connection->send(std::make_shared<ChatPacket>(message));
    }
}

void PlayerList::broadcast(double x, double y, double z, double range,
                           int dimension, std::shared_ptr<Packet> packet) {
    broadcast(nullptr, x, y, z, range, dimension, packet);
}

void PlayerList::broadcast(std::shared_ptr<Player> except, double x, double y,
                           double z, double range, int dimension,
                           std::shared_ptr<Packet> packet) {
    
    
    
    std::vector<std::shared_ptr<ServerPlayer> > sentTo;
    if (except != nullptr) {
        sentTo.push_back(std::dynamic_pointer_cast<ServerPlayer>(except));
    }

    for (unsigned int i = 0; i < players.size(); i++) {
        std::shared_ptr<ServerPlayer> p = players[i];
        if (p == except) continue;
        if (p->dimension != dimension) continue;

        
        bool dontSend = false;
        if (sentTo.size()) {
            INetworkPlayer* thisPlayer = p->connection->getNetworkPlayer();
            if (thisPlayer == nullptr) {
                dontSend = true;
            } else {
                for (unsigned int j = 0; j < sentTo.size(); j++) {
                    std::shared_ptr<ServerPlayer> player2 = sentTo[j];
                    INetworkPlayer* otherPlayer =
                        player2->connection->getNetworkPlayer();
                    if (otherPlayer != nullptr &&
                        thisPlayer->IsSameSystem(otherPlayer)) {
                        dontSend = true;
                    }
                }
            }
        }
        if (dontSend) {
            continue;
        }

        double xd = x - p->x;
        double yd = y - p->y;
        double zd = z - p->z;
        if (xd * xd + yd * yd + zd * zd < range * range) {
            p->connection->send(packet);
            sentTo.push_back(p);
        }
    }
}

void PlayerList::saveAll(ProgressListener* progressListener,
                         bool bDeleteGuestMaps ) {
    if (progressListener != nullptr)
        progressListener->progressStart(IDS_PROGRESS_SAVING_PLAYERS);
    
    
    if (playerIo) {
        playerIo->saveAllCachedData();
        for (unsigned int i = 0; i < players.size(); i++) {
            playerIo->save(players[i]);

            
            
            if (bDeleteGuestMaps && players[i]->isGuest())
                playerIo->deleteMapFilesForPlayer(players[i]);

            if (progressListener != nullptr)
                progressListener->progressStagePercentage(
                    (i * 100) / ((int)players.size()));
        }
        playerIo->clearOldPlayerFiles();
        playerIo->saveMapIdLookup();
    }
}

void PlayerList::whiteList(const std::wstring& playerName) {}

void PlayerList::blackList(const std::wstring& playerName) {}

void PlayerList::reloadWhitelist() {}

void PlayerList::sendLevelInfo(std::shared_ptr<ServerPlayer> player,
                               ServerLevel* level) {
    player->connection->send(std::make_shared<SetTimePacket>(
        level->getGameTime(), level->getDayTime(),
        level->getGameRules()->getBoolean(GameRules::RULE_DAYLIGHT)));
    if (level->isRaining()) {
        player->connection->send(std::shared_ptr<GameEventPacket>(
            new GameEventPacket(GameEventPacket::START_RAINING, 0)));
    } else {
        
        
        
        player->connection->send(std::shared_ptr<GameEventPacket>(
            new GameEventPacket(GameEventPacket::STOP_RAINING, 0)));
    }

    
    if ((level->dimension->id == 0) &&
        level->getLevelData()->getHasStronghold()) {
        player->connection->send(std::make_shared<XZPacket>(
            XZPacket::STRONGHOLD, level->getLevelData()->getXStronghold(),
            level->getLevelData()->getZStronghold()));
    }
}

void PlayerList::sendAllPlayerInfo(std::shared_ptr<ServerPlayer> player) {
    player->refreshContainer(player->inventoryMenu);
    player->resetSentInfo();
    player->connection->send(std::shared_ptr<SetCarriedItemPacket>(
        new SetCarriedItemPacket(player->inventory->selected)));
}

int PlayerList::getPlayerCount() { return (int)players.size(); }

int PlayerList::getPlayerCount(ServerLevel* level) {
    int count = 0;

    for (auto it = players.begin(); it != players.end(); ++it) {
        if ((*it)->level == level) ++count;
    }

    return count;
}

int PlayerList::getMaxPlayers() { return maxPlayers; }

MinecraftServer* PlayerList::getServer() { return server; }

int PlayerList::getViewDistance() { return viewDistance; }

void PlayerList::setOverrideGameMode(GameType* gameMode) {
    overrideGameMode = gameMode;
}

void PlayerList::updatePlayerGameMode(std::shared_ptr<ServerPlayer> newPlayer,
                                      std::shared_ptr<ServerPlayer> oldPlayer,
                                      Level* level) {
    
    
    if (oldPlayer != nullptr) {
        newPlayer->gameMode->setGameModeForPlayer(
            oldPlayer->gameMode->getGameModeForPlayer());
    } else if (overrideGameMode != nullptr) {
        newPlayer->gameMode->setGameModeForPlayer(overrideGameMode);
    }
    newPlayer->gameMode->updateGameMode(level->getLevelData()->getGameType());
}

void PlayerList::setAllowCheatsForAllPlayers(bool allowCommands) {
    this->allowCheatsForAllPlayers = allowCommands;
}

std::shared_ptr<ServerPlayer> PlayerList::findAlivePlayerOnSystem(
    std::shared_ptr<ServerPlayer> player) {
    int dimIndex, playerDim;
    dimIndex = playerDim = player->dimension;
    if (dimIndex == -1)
        dimIndex = 1;
    else if (dimIndex == 1)
        dimIndex = 2;

    INetworkPlayer* thisPlayer = player->connection->getNetworkPlayer();
    if (thisPlayer != nullptr) {
        for (auto itP = players.begin(); itP != players.end(); ++itP) {
            std::shared_ptr<ServerPlayer> newPlayer = *itP;

            INetworkPlayer* otherPlayer =
                newPlayer->connection->getNetworkPlayer();

            if (!newPlayer->removed && newPlayer != player &&
                newPlayer->dimension == playerDim && otherPlayer != nullptr &&
                otherPlayer->IsSameSystem(thisPlayer)) {
                return newPlayer;
            }
        }
    }

    return nullptr;
}

void PlayerList::removePlayerFromReceiving(std::shared_ptr<ServerPlayer> player,
                                           bool usePlayerDimension ,
                                           int dimension ) {
    int dimIndex, playerDim;
    dimIndex = playerDim = usePlayerDimension ? player->dimension : dimension;
    if (dimIndex == -1)
        dimIndex = 1;
    else if (dimIndex == 1)
        dimIndex = 2;

#if !defined(_CONTENT_PACKAGE)
    app.DebugPrintf("Requesting remove player %ls as primary in dimension %d\n",
                    player->name.c_str(), dimIndex);
#endif
    bool playerRemoved = false;

    auto it = find(receiveAllPlayers[dimIndex].begin(),
                   receiveAllPlayers[dimIndex].end(), player);
    if (it != receiveAllPlayers[dimIndex].end()) {
#if !defined(_CONTENT_PACKAGE)
        app.DebugPrintf(
            "Remove: Removing player %ls as primary in dimension %d\n",
            player->name.c_str(), dimIndex);
#endif
        receiveAllPlayers[dimIndex].erase(it);
        playerRemoved = true;
    }

    INetworkPlayer* thisPlayer = player->connection->getNetworkPlayer();
    if (thisPlayer != nullptr && playerRemoved) {
        for (auto itP = players.begin(); itP != players.end(); ++itP) {
            std::shared_ptr<ServerPlayer> newPlayer = *itP;

            INetworkPlayer* otherPlayer =
                newPlayer->connection->getNetworkPlayer();

            if (newPlayer != player && newPlayer->dimension == playerDim &&
                otherPlayer != nullptr &&
                otherPlayer->IsSameSystem(thisPlayer)) {
#if !defined(_CONTENT_PACKAGE)
                app.DebugPrintf(
                    "Remove: Adding player %ls as primary in dimension %d\n",
                    newPlayer->name.c_str(), dimIndex);
#endif
                receiveAllPlayers[dimIndex].push_back(newPlayer);
                break;
            }
        }
    } else if (thisPlayer == nullptr) {
#if !defined(_CONTENT_PACKAGE)
        app.DebugPrintf(
            "Remove: Qnet player for %ls was nullptr so re-checking all "
            "players\n",
            player->name.c_str());
#endif
        
        
        
        for (auto itP = players.begin(); itP != players.end(); ++itP) {
            std::shared_ptr<ServerPlayer> newPlayer = *itP;
            INetworkPlayer* checkingPlayer =
                newPlayer->connection->getNetworkPlayer();

            if (checkingPlayer != nullptr) {
                int newPlayerDim = 0;
                if (newPlayer->dimension == -1)
                    newPlayerDim = 1;
                else if (newPlayer->dimension == 1)
                    newPlayerDim = 2;
                bool foundPrimary = false;
                for (auto it = receiveAllPlayers[newPlayerDim].begin();
                     it != receiveAllPlayers[newPlayerDim].end(); ++it) {
                    std::shared_ptr<ServerPlayer> primaryPlayer = *it;
                    INetworkPlayer* primPlayer =
                        primaryPlayer->connection->getNetworkPlayer();
                    if (primPlayer != nullptr &&
                        checkingPlayer->IsSameSystem(primPlayer)) {
                        foundPrimary = true;
                        break;
                    }
                }
                if (!foundPrimary) {
#if !defined(_CONTENT_PACKAGE)
                    app.DebugPrintf(
                        "Remove: Adding player %ls as primary in dimension "
                        "%d\n",
                        newPlayer->name.c_str(), newPlayerDim);
#endif
                    receiveAllPlayers[newPlayerDim].push_back(newPlayer);
                }
            }
        }
    }
}

void PlayerList::addPlayerToReceiving(std::shared_ptr<ServerPlayer> player) {
    int playerDim = 0;
    if (player->dimension == -1)
        playerDim = 1;
    else if (player->dimension == 1)
        playerDim = 2;

#if !defined(_CONTENT_PACKAGE)
    app.DebugPrintf("Requesting add player %ls as primary in dimension %d\n",
                    player->name.c_str(), playerDim);
#endif

    bool shouldAddPlayer = true;

    INetworkPlayer* thisPlayer = player->connection->getNetworkPlayer();

    if (thisPlayer == nullptr) {
#if !defined(_CONTENT_PACKAGE)
        app.DebugPrintf(
            "Add: Qnet player for player %ls is nullptr so not adding them\n",
            player->name.c_str());
#endif
        shouldAddPlayer = false;
    } else {
        for (auto it = receiveAllPlayers[playerDim].begin();
             it != receiveAllPlayers[playerDim].end(); ++it) {
            std::shared_ptr<ServerPlayer> oldPlayer = *it;
            INetworkPlayer* checkingPlayer =
                oldPlayer->connection->getNetworkPlayer();
            if (checkingPlayer != nullptr &&
                checkingPlayer->IsSameSystem(thisPlayer)) {
                shouldAddPlayer = false;
                break;
            }
        }
    }

    if (shouldAddPlayer) {
#if !defined(_CONTENT_PACKAGE)
        app.DebugPrintf("Add: Adding player %ls as primary in dimension %d\n",
                        player->name.c_str(), playerDim);
#endif
        receiveAllPlayers[playerDim].push_back(player);
    }
}

bool PlayerList::canReceiveAllPackets(std::shared_ptr<ServerPlayer> player) {
    int playerDim = 0;
    if (player->dimension == -1)
        playerDim = 1;
    else if (player->dimension == 1)
        playerDim = 2;
    for (auto it = receiveAllPlayers[playerDim].begin();
         it != receiveAllPlayers[playerDim].end(); ++it) {
        std::shared_ptr<ServerPlayer> newPlayer = *it;
        if (newPlayer == player) {
            return true;
        }
    }
    return false;
}

void PlayerList::kickPlayerByShortId(std::uint8_t networkSmallId) {
    {
        std::lock_guard<std::mutex> lock(m_kickPlayersCS);
        m_smallIdsToKick.push_back(networkSmallId);
    }
}

void PlayerList::closePlayerConnectionBySmallId(std::uint8_t networkSmallId) {
    {
        std::lock_guard<std::mutex> lock(m_closePlayersCS);
        m_smallIdsToClose.push_back(networkSmallId);
    }
}

bool PlayerList::isXuidBanned(PlayerUID xuid) {
    if (xuid == INVALID_XUID) return false;

    bool banned = false;

    for (auto it = m_bannedXuids.begin(); it != m_bannedXuids.end(); ++it) {
        if (ProfileManager.AreXUIDSEqual(xuid, *it)) {
            banned = true;
            break;
        }
    }

    return banned;
}

bool PlayerList::banXuid(PlayerUID xuid) {
    if (xuid == INVALID_XUID) return false;
    if (isXuidBanned(xuid)) return false;
    m_bannedXuids.push_back(xuid);
    return true;
}

bool PlayerList::pardonXuid(PlayerUID xuid) {
    if (xuid == INVALID_XUID) return false;
    for (auto it = m_bannedXuids.begin(); it != m_bannedXuids.end(); ++it) {
        if (ProfileManager.AreXUIDSEqual(xuid, *it)) {
            m_bannedXuids.erase(it);
            return true;
        }
    }
    return false;
}


void PlayerList::setViewDistance(int newViewDistance) {
    
    
    
    
    int clamped = clampViewDistance(newViewDistance);
    viewDistance = clamped;
    for (ServerLevel* level : server->levels) {
        if (level == nullptr) continue;
        level->getChunkMap()->setRadius(clamped);  
        level->getTracker()->updateMaxRange();      
    }
}
