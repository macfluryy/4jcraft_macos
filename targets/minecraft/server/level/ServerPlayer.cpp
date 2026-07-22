#include "ServerPlayer.h"

#include <assert.h>
#include <string.h>

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <format>

#include "platform/sdl2/Input.h"
#include "EntityTracker.h"
#include "app/common/src/Console_Debug_enum.h"
#include "app/common/src/GameRules/LevelRules/Rules/GameRulesInstance.h"
#include "app/common/src/Network/GameNetworkManager.h"
#include "app/common/src/Network/NetworkPlayerInterface.h"
#include "app/mac/MacGame.h"
#include "ServerLevel.h"
#include "ServerPlayerGameMode.h"
#include "PlayerChunkMap.h"
#include "java/InputOutputStream/ByteArrayInputStream.h"
#include "java/InputOutputStream/ByteArrayOutputStream.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"
#include "java/Random.h"
#include "java/System.h"
#include "minecraft/Pos.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/multiplayer/MultiPlayerLevel.h"
#include "minecraft/client/renderer/LevelRenderer.h"
#include "minecraft/network/packet/AnimatePacket.h"
#include "minecraft/network/packet/AwardStatPacket.h"
#include "minecraft/network/packet/BlockRegionUpdatePacket.h"
#include "minecraft/network/packet/ContainerClosePacket.h"
#include "minecraft/network/packet/ContainerOpenPacket.h"
#include "minecraft/network/packet/ContainerSetContentPacket.h"
#include "minecraft/network/packet/ContainerSetDataPacket.h"
#include "minecraft/network/packet/ContainerSetSlotPacket.h"
#include "minecraft/network/packet/CustomPayloadPacket.h"
#include "minecraft/network/packet/EntityActionAtPositionPacket.h"
#include "minecraft/network/packet/EntityEventPacket.h"
#include "minecraft/network/packet/GameEventPacket.h"
#include "minecraft/network/packet/Packet.h"
#include "minecraft/network/packet/PlayerAbilitiesPacket.h"
#include "minecraft/network/packet/RemoveEntitiesPacket.h"
#include "minecraft/network/packet/RemoveMobEffectPacket.h"
#include "minecraft/network/packet/SetEntityLinkPacket.h"
#include "minecraft/network/packet/SetExperiencePacket.h"
#include "minecraft/network/packet/SetHealthPacket.h"
#include "minecraft/network/packet/TileEditorOpenPacket.h"
#include "minecraft/network/packet/UpdateMobEffectPacket.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/server/PlayerList.h"
#include "minecraft/server/network/PlayerConnection.h"
#include "minecraft/stats/GenericStats.h"
#include "minecraft/stats/Stat.h"
#include "minecraft/world/Container.h"
#include "minecraft/world/damageSource/CombatTracker.h"
#include "minecraft/world/damageSource/DamageSource.h"
#include "minecraft/world/damageSource/EntityDamageSource.h"
#include "minecraft/world/entity/Entity.h"
#include "minecraft/world/entity/EntityEvent.h"
#include "minecraft/world/entity/LivingEntity.h"
#include "minecraft/world/entity/animal/EntityHorse.h"
#include "minecraft/world/entity/item/MinecartHopper.h"
#include "minecraft/world/entity/player/Inventory.h"
#include "minecraft/world/entity/projectile/Arrow.h"
#include "minecraft/world/food/FoodData.h"
#include "minecraft/world/inventory/AbstractContainerMenu.h"
#include "minecraft/world/inventory/AnvilMenu.h"
#include "minecraft/world/inventory/BeaconMenu.h"
#include "minecraft/world/inventory/BrewingStandMenu.h"
#include "minecraft/world/inventory/ContainerMenu.h"
#include "minecraft/world/inventory/CraftingMenu.h"
#include "minecraft/world/inventory/EnchantmentMenu.h"
#include "minecraft/world/inventory/FireworksMenu.h"
#include "minecraft/world/inventory/FurnaceMenu.h"
#include "minecraft/world/inventory/HopperMenu.h"
#include "minecraft/world/inventory/HorseInventoryMenu.h"
#include "minecraft/world/inventory/MerchantContainer.h"
#include "minecraft/world/inventory/MerchantMenu.h"
#include "minecraft/world/inventory/ResultSlot.h"
#include "minecraft/world/inventory/Slot.h"
#include "minecraft/world/inventory/TrapMenu.h"
#include "minecraft/world/item/ComplexItem.h"
#include "minecraft/world/item/Item.h"
#include "minecraft/world/item/ItemInstance.h"
#include "minecraft/world/item/UseAnim.h"
#include "minecraft/world/item/trading/Merchant.h"
#include "minecraft/world/item/trading/MerchantRecipeList.h"
#include "minecraft/world/level/ChunkPos.h"
#include "minecraft/world/level/GameRules.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/LevelSettings.h"
#include "minecraft/world/level/ViewDistanceUtil.h"
#include "minecraft/world/level/biome/Biome.h"
#include "minecraft/world/level/chunk/LevelChunk.h"
#include "minecraft/world/level/dimension/Dimension.h"
#include "minecraft/world/level/storage/LevelData.h"
#include "minecraft/world/level/tile/Tile.h"
#include "minecraft/world/level/tile/entity/BeaconTileEntity.h"
#include "minecraft/world/level/tile/entity/BrewingStandTileEntity.h"
#include "minecraft/world/level/tile/entity/DispenserTileEntity.h"
#include "minecraft/world/level/tile/entity/FurnaceTileEntity.h"
#include "minecraft/world/level/tile/entity/HopperTileEntity.h"
#include "minecraft/world/level/tile/entity/SignTileEntity.h"
#include "minecraft/world/level/tile/entity/TileEntity.h"
#include "minecraft/world/scores/Score.h"
#include "minecraft/world/scores/Scoreboard.h"
#include "minecraft/world/scores/criteria/ObjectiveCriteria.h"
#include "nbt/CompoundTag.h"
#include "strings.h"

class Objective;

ServerPlayer::ServerPlayer(MinecraftServer* server, Level* level,
                           const std::wstring& name,
                           ServerPlayerGameMode* gameMode)
    : Player(level, name) {
    
    connection = nullptr;
    lastMoveX = lastMoveZ = 0;
    spewTimer = 0;
    lastRecordedHealthAndAbsorption = FLT_MIN;
    lastSentHealth = -99999999.0f;
    lastSentFood = -99999999;
    lastFoodSaturationZero = true;
    lastSentExp = -99999999.0f;
    invulnerableTime = 20 * 3;
    containerCounter = 0;
    ignoreSlotUpdateHack = false;
    latency = 0;
    wonGame = false;
    m_enteredEndExitPortal = false;
    
    lastActionTime = 0;

    viewDistance = server->getPlayers()->getViewDistance();

    
    
    this->gameMode = gameMode;

    Pos* spawnPos = level->getSharedSpawnPos();
    int xx = spawnPos->x;
    int zz = spawnPos->z;
    int yy = spawnPos->y;
    delete spawnPos;

    if (!level->dimension->hasCeiling &&
        level->getLevelData()->getGameType() != GameType::ADVENTURE) {
        level->isFindingSpawn = true;

        int radius = std::max(5, server->getSpawnProtectionRadius() - 6);

        
        
        
        int waterDepth = 0;
        int attemptCount = 0;
        int xx2, yy2, zz2;

        int minXZ = -(level->dimension->getXZSize() * 16) / 2;
        int maxXZ = (level->dimension->getXZSize() * 16) / 2 - 1;

        bool playerNear = false;
        do {
            
            do {
                xx2 = xx + random->nextInt(radius * 2) - radius;
                zz2 = zz + random->nextInt(radius * 2) - radius;
            } while ((xx2 > maxXZ) || (xx2 < minXZ) || (zz2 > maxXZ) ||
                     (zz2 < minXZ));
            yy2 = level->getTopSolidBlock(xx2, zz2);

            waterDepth = 0;
            int yw = yy2;
            while ((yw < 128) &&
                   ((level->getTile(xx2, yw, zz2) == Tile::water_Id) ||
                    (level->getTile(xx2, yw, zz2) == Tile::calmWater_Id))) {
                yw++;
                waterDepth++;
            }
            attemptCount++;
            playerNear =
                (level->getNearestPlayer(xx + 0.5, yy, zz + 0.5, 3) != nullptr);
        } while ((waterDepth > 1) && (!playerNear) && (attemptCount < 20));
        xx = xx2;
        yy = yy2;
        zz = zz2;

        level->isFindingSpawn = false;
    }

    this->server = server;
    footSize = 0;

    heightOffset =
        0;  
            
    this->moveTo(xx + 0.5, yy, zz + 0.5, 0, 0);

    
    
    
    
    

    

    
    lastBrupSendTickCount = 0;
}

ServerPlayer::~ServerPlayer() {}








void ServerPlayer::flagEntitiesToBeRemoved(unsigned int* flags,
                                           bool* removedFound) {
    if (entitiesToRemove.empty()) {
        return;
    }
    if ((*removedFound) == false) {
        *removedFound = true;
        memset(flags, 0, 2048 / 32);
    }

    auto it = entitiesToRemove.begin();
    for (auto it = entitiesToRemove.begin(); it != entitiesToRemove.end();
         it++) {
        int index = *it;
        if (index < 2048) {
            unsigned int i = index / 32;
            unsigned int j = index % 32;
            unsigned int uiMask = 0x80000000 >> j;

            flags[i] |= uiMask;
        }
    }
}

void ServerPlayer::readAdditionalSaveData(CompoundTag* entityTag) {
    Player::readAdditionalSaveData(entityTag);

    if (entityTag->contains(L"playerGameType")) {
        
        
        
        
        
        
        
        
        
        
    }

    GameRulesInstance* grs = gameMode->getGameRules();
    if (entityTag->contains(L"GameRules") && grs != nullptr) {
        std::vector<uint8_t> ba = entityTag->getByteArray(L"GameRules");
        ByteArrayInputStream bais(ba);
        DataInputStream dis(&bais);
        grs->read(&dis);
        dis.close();
        bais.close();
    }
}

void ServerPlayer::addAdditonalSaveData(CompoundTag* entityTag) {
    Player::addAdditonalSaveData(entityTag);

    GameRulesInstance* grs = gameMode->getGameRules();
    if (grs != nullptr) {
        ByteArrayOutputStream baos;
        DataOutputStream dos(&baos);
        grs->write(&dos);
        entityTag->putByteArray(L"GameRules", baos.buf);
        baos.buf.clear();
        dos.close();
        baos.close();
    }

    
    
    
    
}

void ServerPlayer::giveExperienceLevels(int amount) {
    Player::giveExperienceLevels(amount);
    lastSentExp = -1;
}

void ServerPlayer::initMenu() { containerMenu->addSlotListener(this); }

void ServerPlayer::setDefaultHeadHeight() { heightOffset = 0; }

float ServerPlayer::getHeadHeight() { return 1.62f; }

void ServerPlayer::tick() {
    gameMode->tick();

    if (invulnerableTime > 0) invulnerableTime--;
    containerMenu->broadcastChanges();

    
    Biome* newBiome = level->getBiome(x, z);
    if (newBiome != currentBiome) {
        awardStat(GenericStats::enteredBiome(newBiome->id),
                  GenericStats::param_enteredBiome(newBiome->id));
        currentBiome = newBiome;
    }

    if (!level->isClientSide) {
        if (!containerMenu->stillValid(
                std::dynamic_pointer_cast<Player>(shared_from_this()))) {
            closeContainer();
            containerMenu = inventoryMenu;
        }
    }

    flushEntitiesToRemove();
}


void ServerPlayer::flushEntitiesToRemove() {
    while (!entitiesToRemove.empty()) {
        int sz = entitiesToRemove.size();
        int amount = std::min(sz, RemoveEntitiesPacket::MAX_PER_PACKET);
        std::vector<int> ids(amount);
        int pos = 0;

        auto it = entitiesToRemove.begin();
        while (it != entitiesToRemove.end() && pos < amount) {
            ids[pos++] = *it;
            it = entitiesToRemove.erase(it);
        }

        connection->send(std::shared_ptr<RemoveEntitiesPacket>(
            new RemoveEntitiesPacket(ids)));
    }
}




void ServerPlayer::doTick(bool sendChunks, bool dontDelayChunks ,
                          bool ignorePortal ) {
    m_ignorePortal = ignorePortal;
    if (sendChunks) {
        updateFrameTick();
    }
    doTickA();
    if (sendChunks) {
        doChunkSendingTick(dontDelayChunks);
    }
    doTickB();
    m_ignorePortal = false;
}

void ServerPlayer::doTickA() {
    Player::tick();

    for (unsigned int i = 0; i < inventory->getContainerSize(); i++) {
        std::shared_ptr<ItemInstance> ie = inventory->getItem(i);
        if (ie != nullptr) {
            
            
            
            
            
            if (Item::items[ie->id]
                    ->isComplex())  
                                    
            {
                std::shared_ptr<Packet> packet =
                    (dynamic_cast<ComplexItem*>(Item::items[ie->id])
                         ->getUpdatePacket(ie, level,
                                           std::dynamic_pointer_cast<Player>(
                                               shared_from_this())));
                if (packet != nullptr) {
                    connection->send(packet);
                }
            }
        }
    }
}



void ServerPlayer::doChunkSendingTick(bool dontDelayChunks) {
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    int maxChunksThisTick = connection->isLocal() ? 64 : 16;
    while (maxChunksThisTick-- > 0 && !chunksToSend.empty()) {
        ChunkPos nearest = chunksToSend.front();
        bool nearestValid = false;

        
        
        
        
        
        
        
        
        
        
        
        int pcx = ((int)x) >> 4;
        int pcz = ((int)z) >> 4;
        double dist = DBL_MAX;
        for (auto it = chunksToSend.begin(); it != chunksToSend.end(); it++) {
            ChunkPos chunk = *it;
            if (level->isChunkFinalised(chunk.x, chunk.z)) {
                
                
                
                
                
                
                int dcx = chunk.x - pcx;
                if (dcx < 0) dcx = -dcx;
                int dcz = chunk.z - pcz;
                if (dcz < 0) dcz = -dcz;
                if (dcx > viewDistance || dcz > viewDistance) continue;
                double newDist = chunk.distanceToSqr(x, z);
                if ((!nearestValid) || (newDist < dist)) {
                    nearest = chunk;
                    dist = chunk.distanceToSqr(x, z);
                    nearestValid = true;
                }
            }
        }

        
        
        if (!nearestValid) break;

        
        
        if (nearestValid) {
            bool okToSend = false;

            
            if (connection->isLocal()) {
                if (!connection->done) okToSend = true;
            } else {
                bool canSendToPlayer =
                    MinecraftServer::chunkPacketManagement_CanSendTo(
                        connection->getNetworkPlayer());

                
                
                
                
                
                
                
                
                
                
                
                
                
                

                if (dontDelayChunks ||
                    (canSendToPlayer &&
                     
                     
                     
                     
                     
                     
                     (connection->countDelayedPackets() < 32) &&
                     (g_NetworkManager.GetHostPlayer()
                          ->GetSendQueueSizeMessages(nullptr, true) < 32) &&
                     
                     
                     !connection->done)) {
                    lastBrupSendTickCount = tickCount;
                    okToSend = true;
                    MinecraftServer::chunkPacketManagement_DidSendTo(
                        connection->getNetworkPlayer());

                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                } else {
                    
                    
                    
                    
                    
                    
                    break;
                }
            }

            if (okToSend) {
                ServerLevel* level = server->getLevel(dimension);
                int flagIndex =
                    getFlagIndexForChunk(nearest, this->level->dimension->id);
                chunksToSend.remove(nearest);

                bool chunkDataSent = false;

                
                
                
                
                
                if (!connection
                         ->isLocal())  
                {
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    if (!g_NetworkManager.SystemFlagGet(
                            connection->getNetworkPlayer(), flagIndex)) {
                        
                        
                        int64_t before = System::currentTimeMillis();
                        std::shared_ptr<BlockRegionUpdatePacket> packet =
                            std::shared_ptr<BlockRegionUpdatePacket>(
                                new BlockRegionUpdatePacket(
                                    nearest.x * 16, 0, nearest.z * 16, 16,
                                    Level::maxBuildHeight, 16, level));
                        int64_t after = System::currentTimeMillis();
                        
                        

                        if (dontDelayChunks) packet->shouldDelay = false;

                        if (packet->shouldDelay == true) {
                            
                            
                            
                            connection->queueSend(packet);
                        } else {
                            connection->send(packet);
                        }
                        
                        
                        g_NetworkManager.SystemFlagSet(
                            connection->getNetworkPlayer(), flagIndex);

                        chunkDataSent = true;
                    }
                } else {
                    
                    
                    
                    
                    
                    
                    
                    

                    MultiPlayerLevel* clientLevel =
                        Minecraft::GetInstance()->getLevel(
                            level->dimension->id);
                    if (clientLevel) {
                        LevelChunk* lc =
                            clientLevel->getChunk(nearest.x, nearest.z);
                        lc->reSyncLighting();
                        lc->recalcHeightmapOnly();
                        clientLevel->setTilesDirty(
                            nearest.x * 16 + 1, 1, nearest.z * 16 + 1,
                            nearest.x * 16 + 14, Level::maxBuildHeight - 2,
                            nearest.z * 16 + 14);
                    }
                }
                
                
                
                
                
                
                
                
                
                
                
                if (connection->isLocal() || chunkDataSent) {
                    std::vector<std::shared_ptr<TileEntity> >* tes =
                        level->getTileEntitiesInRegion(
                            nearest.x * 16, 0, nearest.z * 16,
                            nearest.x * 16 + 16, Level::maxBuildHeight,
                            nearest.z * 16 + 16);
                    for (unsigned int i = 0; i < tes->size(); i++) {
                        
                        
                        
                        
                        broadcast(tes->at(i),
                                  !connection->isLocal() && !dontDelayChunks);
                    }
                    delete tes;
                }
            }
        }
    }
}

void ServerPlayer::doTickB() {
#if !defined(_CONTENT_PACKAGE)
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    if (app.GetGameSettingsDebugMask(InputManager.GetPrimaryPad()) &
        (1L << eDebugSetting_GoToOverworld)) {
        if (level->dimension->id != 0) {
            isInsidePortal = true;
            portalTime = 1;
        }
        unsigned int uiVal =
            app.GetGameSettingsDebugMask(InputManager.GetPrimaryPad());
        app.SetGameSettingsDebugMask(
            InputManager.GetPrimaryPad(),
            uiVal & ~(1L << eDebugSetting_GoToOverworld));
    }
#endif

    if (getHealth() != lastSentHealth ||
        lastSentFood != foodData.getFoodLevel() ||
        ((foodData.getSaturationLevel() == 0) != lastFoodSaturationZero)) {
        
        connection->send(std::make_shared<SetHealthPacket>(
            getHealth(), foodData.getFoodLevel(), foodData.getSaturationLevel(),
            m_lastDamageSource));
        lastSentHealth = getHealth();
        lastSentFood = foodData.getFoodLevel();
        lastFoodSaturationZero = foodData.getSaturationLevel() == 0;
    }

    if (getHealth() + getAbsorptionAmount() !=
        lastRecordedHealthAndAbsorption) {
        lastRecordedHealthAndAbsorption = getHealth() + getAbsorptionAmount();

        std::vector<Objective*>* objectives =
            getScoreboard()->findObjectiveFor(ObjectiveCriteria::HEALTH);
        if (objectives) {
            std::vector<std::shared_ptr<Player> > players =
                std::vector<std::shared_ptr<Player> >();
            players.push_back(
                std::dynamic_pointer_cast<Player>(shared_from_this()));

            for (auto it = objectives->begin(); it != objectives->end(); ++it) {
                Objective* objective = *it;
                getScoreboard()
                    ->getPlayerScore(getAName(), objective)
                    ->updateFor(&players);
            }
            delete objectives;
        }
    }

    if (totalExperience != lastSentExp) {
        lastSentExp = totalExperience;
        connection->send(std::make_shared<SetExperiencePacket>(
            experienceProgress, totalExperience, experienceLevel));
    }
}

std::shared_ptr<ItemInstance> ServerPlayer::getCarried(int slot) {
    if (slot == 0) return inventory->getSelected();
    return inventory->armor[slot - 1];
}

void ServerPlayer::die(DamageSource* source) {
    server->getPlayers()->broadcastAll(
        getCombatTracker()->getDeathMessagePacket());

    if (!level->getGameRules()->getBoolean(GameRules::RULE_KEEPINVENTORY)) {
        inventory->dropAll();
    }

    std::vector<Objective*>* objectives =
        level->getScoreboard()->findObjectiveFor(
            ObjectiveCriteria::DEATH_COUNT);
    if (objectives) {
        for (int i = 0; i < objectives->size(); i++) {
            Objective* objective = objectives->at(i);

            Score* score =
                getScoreboard()->getPlayerScore(getAName(), objective);
            score->increment();
        }
        delete objectives;
    }

    std::shared_ptr<LivingEntity> killer = getKillCredit();
    if (killer != nullptr)
        killer->awardKillScore(shared_from_this(), deathScore);
    
}

bool ServerPlayer::hurt(DamageSource* dmgSource, float dmg) {
    if (isInvulnerable()) return false;

    
    
    
    
    
    
    if (!server->isPvpAllowed() && invulnerableTime > 0 &&
        dmgSource != DamageSource::outOfWorld)
        return false;

    if (dynamic_cast<EntityDamageSource*>(dmgSource) != nullptr) {
        
        
        
        
        std::shared_ptr<Entity> source = dmgSource->getDirectEntity();

        if (source->instanceof(eTYPE_PLAYER) &&
            !std::dynamic_pointer_cast<Player>(source)->canHarmPlayer(
                std::dynamic_pointer_cast<Player>(shared_from_this()))) {
            return false;
        }

        if ((source != nullptr) && source->instanceof(eTYPE_ARROW)) {
            std::shared_ptr<Arrow> arrow =
                std::dynamic_pointer_cast<Arrow>(source);
            if ((arrow->owner != nullptr) &&
                arrow->owner->instanceof(eTYPE_PLAYER) &&
                !canHarmPlayer(
                    std::dynamic_pointer_cast<Player>(arrow->owner))) {
                return false;
            }
        }
    }

    return Player::hurt(dmgSource, dmg);
}

bool ServerPlayer::canHarmPlayer(std::shared_ptr<Player> target) {
    if (!server->isPvpAllowed()) return false;
    if (!isAllowedToAttackPlayers()) return false;
    return Player::canHarmPlayer(target);
}



bool ServerPlayer::canHarmPlayer(std::wstring targetName) {
    bool canHarm = true;

    std::shared_ptr<ServerPlayer> owner =
        server->getPlayers()->getPlayer(targetName);
    if (owner != nullptr) {
        if ((shared_from_this() != owner) && canHarmPlayer(owner))
            canHarm = false;
    } else {
        if (this->name != targetName &&
            (!isAllowedToAttackPlayers() || !server->isPvpAllowed()))
            canHarm = false;
    }

    return canHarm;
}

void ServerPlayer::changeDimension(int i) {
    if (!connection->hasClientTickedOnce()) return;

    if (dimension == 1 && i == 1) {
        app.DebugPrintf("Start win game\n");
        awardStat(GenericStats::winGame(), GenericStats::param_winGame());

        
        
        INetworkPlayer* thisPlayer = connection->getNetworkPlayer();

        if (!wonGame) {
            level->removeEntity(shared_from_this());
            wonGame = true;
            m_enteredEndExitPortal =
                true;  
            connection->send(std::make_shared<GameEventPacket>(
                GameEventPacket::WIN_GAME, thisPlayer->GetUserIndex()));
            app.DebugPrintf("Sending packet to %d\n",
                            thisPlayer->GetUserIndex());
        }
        if (thisPlayer != nullptr) {
            for (auto it = MinecraftServer::getInstance()
                               ->getPlayers()
                               ->players.begin();
                 it !=
                 MinecraftServer::getInstance()->getPlayers()->players.end();
                 ++it) {
                std::shared_ptr<ServerPlayer> servPlayer = *it;
                INetworkPlayer* checkPlayer =
                    servPlayer->connection->getNetworkPlayer();
                if (thisPlayer != checkPlayer && checkPlayer != nullptr &&
                    thisPlayer->IsSameSystem(checkPlayer) &&
                    !servPlayer->wonGame) {
                    servPlayer->wonGame = true;
                    servPlayer->connection->send(
                        std::shared_ptr<GameEventPacket>(
                            new GameEventPacket(GameEventPacket::WIN_GAME,
                                                thisPlayer->GetUserIndex())));
                    app.DebugPrintf("Sending packet to %d\n",
                                    thisPlayer->GetUserIndex());
                }
            }
        }
        app.DebugPrintf("End win game\n");
    } else {
        if (dimension == 0 && i == 1) {
            awardStat(GenericStats::theEnd(), GenericStats::param_theEnd());

            Pos* pos = server->getLevel(i)->getDimensionSpecificSpawn();
            if (pos != nullptr) {
                connection->teleport(pos->x, pos->y, pos->z, 0, 0);
                delete pos;
            }

            i = 1;
        } else {
            
            
        }
        server->getPlayers()->toggleDimension(
            std::dynamic_pointer_cast<ServerPlayer>(shared_from_this()), i);
        lastSentExp = -1;
        lastSentHealth = -1;
        lastSentFood = -1;
    }
}


void ServerPlayer::broadcast(std::shared_ptr<TileEntity> te,
                             bool delay ) {
    if (te != nullptr) {
        std::shared_ptr<Packet> p = te->getUpdatePacket();
        if (p != nullptr) {
            p->shouldDelay = delay;
            if (delay)
                connection->queueSend(p);
            else
                connection->send(p);
        }
    }
}

void ServerPlayer::take(std::shared_ptr<Entity> e, int orgCount) {
    Player::take(e, orgCount);
    containerMenu->broadcastChanges();
}

Player::BedSleepingResult ServerPlayer::startSleepInBed(int x, int y, int z,
                                                        bool bTestUse) {
    BedSleepingResult result = Player::startSleepInBed(x, y, z, bTestUse);
    if (result == OK) {
        std::shared_ptr<Packet> p =
            std::shared_ptr<EntityActionAtPositionPacket>(
                new EntityActionAtPositionPacket(
                    shared_from_this(),
                    EntityActionAtPositionPacket::START_SLEEP, x, y, z));
        getLevel()->getTracker()->broadcast(shared_from_this(), p);
        connection->teleport(this->x, this->y, this->z, yRot, xRot);
        connection->send(p);
    }
    return result;
}

void ServerPlayer::stopSleepInBed(bool forcefulWakeUp, bool updateLevelList,
                                  bool saveRespawnPoint) {
    if (isSleeping()) {
        getLevel()->getTracker()->broadcastAndSend(
            shared_from_this(),
            std::shared_ptr<AnimatePacket>(
                new AnimatePacket(shared_from_this(), AnimatePacket::WAKE_UP)));
    }
    Player::stopSleepInBed(forcefulWakeUp, updateLevelList, saveRespawnPoint);
    if (connection != nullptr) connection->teleport(x, y, z, yRot, xRot);
}

void ServerPlayer::ride(std::shared_ptr<Entity> e) {
    Player::ride(e);
    connection->send(std::make_shared<SetEntityLinkPacket>(
        SetEntityLinkPacket::RIDING, shared_from_this(), riding));

    
    
    
    
    
    
}

void ServerPlayer::checkFallDamage(double ya, bool onGround) {}

void ServerPlayer::doCheckFallDamage(double ya, bool onGround) {
    Player::checkFallDamage(ya, onGround);
}

void ServerPlayer::openTextEdit(std::shared_ptr<TileEntity> sign) {
    std::shared_ptr<SignTileEntity> signTE =
        std::dynamic_pointer_cast<SignTileEntity>(sign);
    if (signTE != nullptr) {
        signTE->setAllowedPlayerEditor(
            std::dynamic_pointer_cast<Player>(shared_from_this()));
        connection->send(std::make_shared<TileEditorOpenPacket>(
            TileEditorOpenPacket::SIGN, sign->x, sign->y, sign->z));
    }
}

void ServerPlayer::nextContainerCounter() {
    containerCounter = (containerCounter % 100) + 1;
}

bool ServerPlayer::startCrafting(int x, int y, int z) {
    if (containerMenu == inventoryMenu) {
        nextContainerCounter();
        connection->send(std::make_shared<ContainerOpenPacket>(
            containerCounter, ContainerOpenPacket::WORKBENCH, L"", 9, false));
        containerMenu = new CraftingMenu(inventory, level, x, y, z);
        containerMenu->containerId = containerCounter;
        containerMenu->addSlotListener(this);
    } else {
        app.DebugPrintf(
            "ServerPlayer tried to open crafting container when one was "
            "already open\n");
    }

    return true;
}

bool ServerPlayer::openFireworks(int x, int y, int z) {
    if (containerMenu == inventoryMenu) {
        nextContainerCounter();
        connection->send(std::make_shared<ContainerOpenPacket>(
            containerCounter, ContainerOpenPacket::FIREWORKS, L"", 9, false));
        containerMenu = new FireworksMenu(inventory, level, x, y, z);
        containerMenu->containerId = containerCounter;
        containerMenu->addSlotListener(this);
    } else if (dynamic_cast<CraftingMenu*>(containerMenu) != nullptr) {
        closeContainer();

        nextContainerCounter();
        connection->send(std::make_shared<ContainerOpenPacket>(
            containerCounter, ContainerOpenPacket::FIREWORKS, L"", 9, false));
        containerMenu = new FireworksMenu(inventory, level, x, y, z);
        containerMenu->containerId = containerCounter;
        containerMenu->addSlotListener(this);
    } else {
        app.DebugPrintf(
            "ServerPlayer tried to open crafting container when one was "
            "already open\n");
    }

    return true;
}

bool ServerPlayer::startEnchanting(int x, int y, int z,
                                   const std::wstring& name) {
    if (containerMenu == inventoryMenu) {
        nextContainerCounter();
        connection->send(std::make_shared<ContainerOpenPacket>(
            containerCounter, ContainerOpenPacket::ENCHANTMENT,
            name.empty() ? L"" : name, 9, !name.empty()));
        containerMenu = new EnchantmentMenu(inventory, level, x, y, z);
        containerMenu->containerId = containerCounter;
        containerMenu->addSlotListener(this);
    } else {
        app.DebugPrintf(
            "ServerPlayer tried to open enchanting container when one was "
            "already open\n");
    }

    return true;
}

bool ServerPlayer::startRepairing(int x, int y, int z) {
    if (containerMenu == inventoryMenu) {
        nextContainerCounter();
        connection->send(std::make_shared<ContainerOpenPacket>(
            containerCounter, ContainerOpenPacket::REPAIR_TABLE, L"", 9,
            false));
        containerMenu = new AnvilMenu(
            inventory, level, x, y, z,
            std::dynamic_pointer_cast<Player>(shared_from_this()));
        containerMenu->containerId = containerCounter;
        containerMenu->addSlotListener(this);
    } else {
        app.DebugPrintf(
            "ServerPlayer tried to open enchanting container when one was "
            "already open\n");
    }

    return true;
}

bool ServerPlayer::openContainer(std::shared_ptr<Container> container) {
    if (containerMenu == inventoryMenu) {
        nextContainerCounter();

        
        
        int containerType = container->getContainerType();
        assert(containerType >= 0);

        connection->send(std::make_shared<ContainerOpenPacket>(
            containerCounter, containerType, container->getCustomName(),
            container->getContainerSize(), container->hasCustomName()));

        containerMenu = new ContainerMenu(inventory, container);
        containerMenu->containerId = containerCounter;
        containerMenu->addSlotListener(this);
    } else {
        app.DebugPrintf(
            "ServerPlayer tried to open container when one was already open\n");
    }

    return true;
}

bool ServerPlayer::openHopper(std::shared_ptr<HopperTileEntity> container) {
    if (containerMenu == inventoryMenu) {
        nextContainerCounter();
        connection->send(std::make_shared<ContainerOpenPacket>(
            containerCounter, ContainerOpenPacket::HOPPER,
            container->getCustomName(), container->getContainerSize(),
            container->hasCustomName()));
        containerMenu = new HopperMenu(inventory, container);
        containerMenu->containerId = containerCounter;
        containerMenu->addSlotListener(this);
    } else {
        app.DebugPrintf(
            "ServerPlayer tried to open hopper container when one was already "
            "open\n");
    }

    return true;
}

bool ServerPlayer::openHopper(std::shared_ptr<MinecartHopper> container) {
    if (containerMenu == inventoryMenu) {
        nextContainerCounter();
        connection->send(std::make_shared<ContainerOpenPacket>(
            containerCounter, ContainerOpenPacket::HOPPER,
            container->getCustomName(), container->getContainerSize(),
            container->hasCustomName()));
        containerMenu = new HopperMenu(inventory, container);
        containerMenu->containerId = containerCounter;
        containerMenu->addSlotListener(this);
    } else {
        app.DebugPrintf(
            "ServerPlayer tried to open minecart hopper container when one was "
            "already open\n");
    }

    return true;
}

bool ServerPlayer::openFurnace(std::shared_ptr<FurnaceTileEntity> furnace) {
    if (containerMenu == inventoryMenu) {
        nextContainerCounter();
        connection->send(std::make_shared<ContainerOpenPacket>(
            containerCounter, ContainerOpenPacket::FURNACE,
            furnace->getCustomName(), furnace->getContainerSize(),
            furnace->hasCustomName()));
        containerMenu = new FurnaceMenu(inventory, furnace);
        containerMenu->containerId = containerCounter;
        containerMenu->addSlotListener(this);
    } else {
        app.DebugPrintf(
            "ServerPlayer tried to open furnace when one was already open\n");
    }

    return true;
}

bool ServerPlayer::openTrap(std::shared_ptr<DispenserTileEntity> trap) {
    if (containerMenu == inventoryMenu) {
        nextContainerCounter();
        connection->send(std::make_shared<ContainerOpenPacket>(
            containerCounter,
            trap->GetType() == eTYPE_DROPPERTILEENTITY
                ? ContainerOpenPacket::DROPPER
                : ContainerOpenPacket::TRAP,
            trap->getCustomName(), trap->getContainerSize(),
            trap->hasCustomName()));
        containerMenu = new TrapMenu(inventory, trap);
        containerMenu->containerId = containerCounter;
        containerMenu->addSlotListener(this);
    } else {
        app.DebugPrintf(
            "ServerPlayer tried to open dispenser when one was already open\n");
    }

    return true;
}

bool ServerPlayer::openBrewingStand(
    std::shared_ptr<BrewingStandTileEntity> brewingStand) {
    if (containerMenu == inventoryMenu) {
        nextContainerCounter();
        connection->send(std::make_shared<ContainerOpenPacket>(
            containerCounter, ContainerOpenPacket::BREWING_STAND,
            brewingStand->getCustomName(), brewingStand->getContainerSize(),
            brewingStand->hasCustomName()));
        containerMenu = new BrewingStandMenu(inventory, brewingStand);
        containerMenu->containerId = containerCounter;
        containerMenu->addSlotListener(this);
    } else {
        app.DebugPrintf(
            "ServerPlayer tried to open brewing stand when one was already "
            "open\n");
    }

    return true;
}

bool ServerPlayer::openBeacon(std::shared_ptr<BeaconTileEntity> beacon) {
    if (containerMenu == inventoryMenu) {
        nextContainerCounter();
        connection->send(std::make_shared<ContainerOpenPacket>(
            containerCounter, ContainerOpenPacket::BEACON,
            beacon->getCustomName(), beacon->getContainerSize(),
            beacon->hasCustomName()));
        containerMenu = new BeaconMenu(inventory, beacon);
        containerMenu->containerId = containerCounter;
        containerMenu->addSlotListener(this);
    } else {
        app.DebugPrintf(
            "ServerPlayer tried to open beacon when one was already open\n");
    }

    return true;
}

bool ServerPlayer::openTrading(std::shared_ptr<Merchant> traderTarget,
                               const std::wstring& name) {
    if (containerMenu == inventoryMenu) {
        nextContainerCounter();
        containerMenu = new MerchantMenu(inventory, traderTarget, level);
        containerMenu->containerId = containerCounter;
        containerMenu->addSlotListener(this);
        std::shared_ptr<Container> container =
            ((MerchantMenu*)containerMenu)->getTradeContainer();

        connection->send(std::make_shared<ContainerOpenPacket>(
            containerCounter, ContainerOpenPacket::TRADER_NPC,
            name.empty() ? L"" : name, container->getContainerSize(),
            !name.empty()));

        MerchantRecipeList* offers = traderTarget->getOffers(
            std::dynamic_pointer_cast<Player>(shared_from_this()));
        if (offers != nullptr) {
            ByteArrayOutputStream rawOutput;
            DataOutputStream output(&rawOutput);

            
            output.writeInt(containerCounter);
            offers->writeToStream(&output);

            connection->send(std::shared_ptr<CustomPayloadPacket>(
                new CustomPayloadPacket(CustomPayloadPacket::TRADER_LIST_PACKET,
                                        rawOutput.toByteArray())));
        }
    } else {
        app.DebugPrintf(
            "ServerPlayer tried to open trading menu when one was already "
            "open\n");
    }

    return true;
}

bool ServerPlayer::openHorseInventory(std::shared_ptr<EntityHorse> horse,
                                      std::shared_ptr<Container> container) {
    if (containerMenu != inventoryMenu) {
        closeContainer();
    }
    nextContainerCounter();
    connection->send(std::make_shared<ContainerOpenPacket>(
        containerCounter, ContainerOpenPacket::HORSE, horse->getCustomName(),
        container->getContainerSize(), container->hasCustomName(),
        horse->entityId));
    containerMenu = new HorseInventoryMenu(inventory, container, horse);
    containerMenu->containerId = containerCounter;
    containerMenu->addSlotListener(this);

    return true;
}

void ServerPlayer::slotChanged(AbstractContainerMenu* container, int slotIndex,
                               std::shared_ptr<ItemInstance> item) {
    if (dynamic_cast<ResultSlot*>(container->getSlot(slotIndex))) {
        return;
    }

    if (ignoreSlotUpdateHack) {
        
        
        
        
        
        return;
    }

    connection->send(std::shared_ptr<ContainerSetSlotPacket>(
        new ContainerSetSlotPacket(container->containerId, slotIndex, item)));
}

void ServerPlayer::refreshContainer(AbstractContainerMenu* menu) {
    std::vector<std::shared_ptr<ItemInstance> >* items = menu->getItems();
    refreshContainer(menu, items);
    delete items;
}

void ServerPlayer::refreshContainer(
    AbstractContainerMenu* container,
    std::vector<std::shared_ptr<ItemInstance> >* items) {
    connection->send(std::shared_ptr<ContainerSetContentPacket>(
        new ContainerSetContentPacket(container->containerId, items)));
    connection->send(std::shared_ptr<ContainerSetSlotPacket>(
        new ContainerSetSlotPacket(-1, -1, inventory->getCarried())));
}

void ServerPlayer::setContainerData(AbstractContainerMenu* container, int id,
                                    int value) {
    
    if (ignoreSlotUpdateHack) {
        
        
        
        
        
        return;
    }
    connection->send(std::shared_ptr<ContainerSetDataPacket>(
        new ContainerSetDataPacket(container->containerId, id, value)));
}

void ServerPlayer::closeContainer() {
    connection->send(std::shared_ptr<ContainerClosePacket>(
        new ContainerClosePacket(containerMenu->containerId)));
    doCloseContainer();
}

void ServerPlayer::broadcastCarriedItem() {
    if (ignoreSlotUpdateHack) {
        
        
        
        
        return;
    }
    connection->send(std::shared_ptr<ContainerSetSlotPacket>(
        new ContainerSetSlotPacket(-1, -1, inventory->getCarried())));
}

void ServerPlayer::doCloseContainer() {
    containerMenu->removed(
        std::dynamic_pointer_cast<Player>(shared_from_this()));
    containerMenu = inventoryMenu;
}

void ServerPlayer::setPlayerInput(float xxa, float yya, bool jumping,
                                  bool sneaking) {
    if (riding != nullptr) {
        if (xxa >= -1 && xxa <= 1) this->xxa = xxa;
        if (yya >= -1 && yya <= 1) this->yya = yya;
        this->jumping = jumping;
        this->setSneaking(sneaking);
    }
}

void ServerPlayer::awardStat(Stat* stat, const std::vector<uint8_t>& param) {
    if (stat == nullptr) {
        return;
    }

    if (!stat->awardLocallyOnly) {
        int count = *((int*)param.data());

        connection->send(std::shared_ptr<AwardStatPacket>(
            new AwardStatPacket(stat->id, count)));
    }
}

void ServerPlayer::disconnect() {
    if (rider.lock() != nullptr) rider.lock()->ride(shared_from_this());
    if (m_isSleeping) {
        stopSleepInBed(true, false, false);
    }
}

void ServerPlayer::resetSentInfo() { lastSentHealth = -99999999.0f; }

void ServerPlayer::displayClientMessage(int messageId) {
    ChatPacket::EChatPacketMessage messageType = ChatPacket::e_ChatCustom;
    
    
    switch (messageId) {
        case IDS_TILE_BED_OCCUPIED:
            messageType = ChatPacket::e_ChatBedOccupied;
            connection->send(std::make_shared<ChatPacket>(L"", messageType));
            break;
        case IDS_TILE_BED_NO_SLEEP:
            messageType = ChatPacket::e_ChatBedNoSleep;
            connection->send(std::make_shared<ChatPacket>(L"", messageType));
            break;
        case IDS_TILE_BED_NOT_VALID:
            messageType = ChatPacket::e_ChatBedNotValid;
            connection->send(std::make_shared<ChatPacket>(L"", messageType));
            break;
        case IDS_TILE_BED_NOTSAFE:
            messageType = ChatPacket::e_ChatBedNotSafe;
            connection->send(std::make_shared<ChatPacket>(L"", messageType));
            break;
        case IDS_TILE_BED_PLAYERSLEEP:
            messageType = ChatPacket::e_ChatBedPlayerSleep;
            
            for (unsigned int i = 0; i < server->getPlayers()->players.size();
                 i++) {
                std::shared_ptr<ServerPlayer> player =
                    server->getPlayers()->players[i];
                if (shared_from_this() != player) {
                    player->connection->send(std::make_shared<ChatPacket>(
                        name, ChatPacket::e_ChatBedPlayerSleep));
                } else {
                    player->connection->send(std::shared_ptr<ChatPacket>(
                        new ChatPacket(name, ChatPacket::e_ChatBedMeSleep)));
                }
            }
            return;
            break;
        case IDS_PLAYER_ENTERED_END:
            for (unsigned int i = 0; i < server->getPlayers()->players.size();
                 i++) {
                std::shared_ptr<ServerPlayer> player =
                    server->getPlayers()->players[i];
                if (shared_from_this() != player) {
                    player->connection->send(std::make_shared<ChatPacket>(
                        name, ChatPacket::e_ChatPlayerEnteredEnd));
                }
            }
            break;
        case IDS_PLAYER_LEFT_END:
            for (unsigned int i = 0; i < server->getPlayers()->players.size();
                 i++) {
                std::shared_ptr<ServerPlayer> player =
                    server->getPlayers()->players[i];
                if (shared_from_this() != player) {
                    player->connection->send(std::shared_ptr<ChatPacket>(
                        new ChatPacket(name, ChatPacket::e_ChatPlayerLeftEnd)));
                }
            }
            break;
        case IDS_TILE_BED_MESLEEP:
            messageType = ChatPacket::e_ChatBedMeSleep;
            connection->send(std::make_shared<ChatPacket>(L"", messageType));
            break;

        case IDS_MAX_PIGS_SHEEP_COWS_CATS_SPAWNED:
            for (unsigned int i = 0; i < server->getPlayers()->players.size();
                 i++) {
                std::shared_ptr<ServerPlayer> player =
                    server->getPlayers()->players[i];
                if (shared_from_this() == player) {
                    player->connection->send(std::make_shared<ChatPacket>(
                        name, ChatPacket::e_ChatPlayerMaxPigsSheepCows));
                }
            }
            break;
        case IDS_MAX_CHICKENS_SPAWNED:
            for (unsigned int i = 0; i < server->getPlayers()->players.size();
                 i++) {
                std::shared_ptr<ServerPlayer> player =
                    server->getPlayers()->players[i];
                if (shared_from_this() == player) {
                    player->connection->send(std::make_shared<ChatPacket>(
                        name, ChatPacket::e_ChatPlayerMaxChickens));
                }
            }
            break;
        case IDS_MAX_SQUID_SPAWNED:
            for (unsigned int i = 0; i < server->getPlayers()->players.size();
                 i++) {
                std::shared_ptr<ServerPlayer> player =
                    server->getPlayers()->players[i];
                if (shared_from_this() == player) {
                    player->connection->send(std::make_shared<ChatPacket>(
                        name, ChatPacket::e_ChatPlayerMaxSquid));
                }
            }
            break;
        case IDS_MAX_BATS_SPAWNED:
            for (unsigned int i = 0; i < server->getPlayers()->players.size();
                 i++) {
                std::shared_ptr<ServerPlayer> player =
                    server->getPlayers()->players[i];
                if (shared_from_this() == player) {
                    player->connection->send(std::shared_ptr<ChatPacket>(
                        new ChatPacket(name, ChatPacket::e_ChatPlayerMaxBats)));
                }
            }
            break;
        case IDS_MAX_WOLVES_SPAWNED:
            for (unsigned int i = 0; i < server->getPlayers()->players.size();
                 i++) {
                std::shared_ptr<ServerPlayer> player =
                    server->getPlayers()->players[i];
                if (shared_from_this() == player) {
                    player->connection->send(std::make_shared<ChatPacket>(
                        name, ChatPacket::e_ChatPlayerMaxWolves));
                }
            }
            break;
        case IDS_MAX_MOOSHROOMS_SPAWNED:
            for (unsigned int i = 0; i < server->getPlayers()->players.size();
                 i++) {
                std::shared_ptr<ServerPlayer> player =
                    server->getPlayers()->players[i];
                if (shared_from_this() == player) {
                    player->connection->send(std::make_shared<ChatPacket>(
                        name, ChatPacket::e_ChatPlayerMaxMooshrooms));
                }
            }
            break;
        case IDS_MAX_ENEMIES_SPAWNED:
            for (unsigned int i = 0; i < server->getPlayers()->players.size();
                 i++) {
                std::shared_ptr<ServerPlayer> player =
                    server->getPlayers()->players[i];
                if (shared_from_this() == player) {
                    player->connection->send(std::make_shared<ChatPacket>(
                        name, ChatPacket::e_ChatPlayerMaxEnemies));
                }
            }
            break;

        case IDS_MAX_VILLAGERS_SPAWNED:
            for (unsigned int i = 0; i < server->getPlayers()->players.size();
                 i++) {
                std::shared_ptr<ServerPlayer> player =
                    server->getPlayers()->players[i];
                if (shared_from_this() == player) {
                    player->connection->send(std::make_shared<ChatPacket>(
                        name, ChatPacket::e_ChatPlayerMaxVillagers));
                }
            }
            break;
        case IDS_MAX_PIGS_SHEEP_COWS_CATS_BRED:
            for (unsigned int i = 0; i < server->getPlayers()->players.size();
                 i++) {
                std::shared_ptr<ServerPlayer> player =
                    server->getPlayers()->players[i];
                if (shared_from_this() == player) {
                    player->connection->send(std::make_shared<ChatPacket>(
                        name, ChatPacket::e_ChatPlayerMaxBredPigsSheepCows));
                }
            }
            break;
        case IDS_MAX_CHICKENS_BRED:
            for (unsigned int i = 0; i < server->getPlayers()->players.size();
                 i++) {
                std::shared_ptr<ServerPlayer> player =
                    server->getPlayers()->players[i];
                if (shared_from_this() == player) {
                    player->connection->send(std::make_shared<ChatPacket>(
                        name, ChatPacket::e_ChatPlayerMaxBredChickens));
                }
            }
            break;
        case IDS_MAX_MUSHROOMCOWS_BRED:
            for (unsigned int i = 0; i < server->getPlayers()->players.size();
                 i++) {
                std::shared_ptr<ServerPlayer> player =
                    server->getPlayers()->players[i];
                if (shared_from_this() == player) {
                    player->connection->send(std::make_shared<ChatPacket>(
                        name, ChatPacket::e_ChatPlayerMaxBredMooshrooms));
                }
            }
            break;

        case IDS_MAX_WOLVES_BRED:
            for (unsigned int i = 0; i < server->getPlayers()->players.size();
                 i++) {
                std::shared_ptr<ServerPlayer> player =
                    server->getPlayers()->players[i];
                if (shared_from_this() == player) {
                    player->connection->send(std::make_shared<ChatPacket>(
                        name, ChatPacket::e_ChatPlayerMaxBredWolves));
                }
            }
            break;

        case IDS_CANT_SHEAR_MOOSHROOM:
            for (unsigned int i = 0; i < server->getPlayers()->players.size();
                 i++) {
                std::shared_ptr<ServerPlayer> player =
                    server->getPlayers()->players[i];
                if (shared_from_this() == player) {
                    player->connection->send(std::make_shared<ChatPacket>(
                        name, ChatPacket::e_ChatPlayerCantShearMooshroom));
                }
            }
            break;

        case IDS_MAX_HANGINGENTITIES:
            for (unsigned int i = 0; i < server->getPlayers()->players.size();
                 i++) {
                std::shared_ptr<ServerPlayer> player =
                    server->getPlayers()->players[i];
                if (shared_from_this() == player) {
                    player->connection->send(std::make_shared<ChatPacket>(
                        name, ChatPacket::e_ChatPlayerMaxHangingEntities));
                }
            }
            break;
        case IDS_CANT_SPAWN_IN_PEACEFUL:
            for (unsigned int i = 0; i < server->getPlayers()->players.size();
                 i++) {
                std::shared_ptr<ServerPlayer> player =
                    server->getPlayers()->players[i];
                if (shared_from_this() == player) {
                    player->connection->send(std::make_shared<ChatPacket>(
                        name, ChatPacket::e_ChatPlayerCantSpawnInPeaceful));
                }
            }
            break;

        case IDS_MAX_BOATS:
            for (unsigned int i = 0; i < server->getPlayers()->players.size();
                 i++) {
                std::shared_ptr<ServerPlayer> player =
                    server->getPlayers()->players[i];
                if (shared_from_this() == player) {
                    player->connection->send(std::make_shared<ChatPacket>(
                        name, ChatPacket::e_ChatPlayerMaxBoats));
                }
            }
            break;

        default:
            app.DebugPrintf(
                "Tried to send a chat packet to the player with an unhandled "
                "messageId\n");
            assert(false);
            break;
    }

    
    
    
    
    
}

void ServerPlayer::completeUsingItem() {
    connection->send(std::shared_ptr<EntityEventPacket>(
        new EntityEventPacket(entityId, EntityEvent::USE_ITEM_COMPLETE)));
    Player::completeUsingItem();
}

void ServerPlayer::startUsingItem(std::shared_ptr<ItemInstance> instance,
                                  int duration) {
    Player::startUsingItem(instance, duration);

    if (instance != nullptr && instance->getItem() != nullptr &&
        instance->getItem()->getUseAnimation(instance) == UseAnim_eat) {
        getLevel()->getTracker()->broadcastAndSend(
            shared_from_this(),
            std::shared_ptr<AnimatePacket>(
                new AnimatePacket(shared_from_this(), AnimatePacket::EAT)));
    }
}

void ServerPlayer::restoreFrom(std::shared_ptr<Player> oldPlayer,
                               bool restoreAll) {
    Player::restoreFrom(oldPlayer, restoreAll);
    lastSentExp = -1;
    lastSentHealth = -1;
    lastSentFood = -1;
    entitiesToRemove =
        std::dynamic_pointer_cast<ServerPlayer>(oldPlayer)->entitiesToRemove;
}

void ServerPlayer::onEffectAdded(MobEffectInstance* effect) {
    Player::onEffectAdded(effect);
    connection->send(std::shared_ptr<UpdateMobEffectPacket>(
        new UpdateMobEffectPacket(entityId, effect)));
}

void ServerPlayer::onEffectUpdated(MobEffectInstance* effect,
                                   bool doRefreshAttributes) {
    Player::onEffectUpdated(effect, doRefreshAttributes);
    connection->send(std::shared_ptr<UpdateMobEffectPacket>(
        new UpdateMobEffectPacket(entityId, effect)));
}

void ServerPlayer::onEffectRemoved(MobEffectInstance* effect) {
    Player::onEffectRemoved(effect);
    connection->send(std::shared_ptr<RemoveMobEffectPacket>(
        new RemoveMobEffectPacket(entityId, effect)));
}

void ServerPlayer::teleportTo(double x, double y, double z) {
    connection->teleport(x, y, z, yRot, xRot);
}

void ServerPlayer::crit(std::shared_ptr<Entity> entity) {
    getLevel()->getTracker()->broadcastAndSend(
        shared_from_this(),
        std::make_shared<AnimatePacket>(entity, AnimatePacket::CRITICAL_HIT));
}

void ServerPlayer::magicCrit(std::shared_ptr<Entity> entity) {
    getLevel()->getTracker()->broadcastAndSend(
        shared_from_this(), std::make_shared<AnimatePacket>(
                                entity, AnimatePacket::MAGIC_CRITICAL_HIT));
}

void ServerPlayer::onUpdateAbilities() {
    if (connection == nullptr) return;
    connection->send(std::shared_ptr<PlayerAbilitiesPacket>(
        new PlayerAbilitiesPacket(&abilities)));
}

ServerLevel* ServerPlayer::getLevel() { return (ServerLevel*)level; }

void ServerPlayer::setGameMode(GameType* mode) {
    gameMode->setGameModeForPlayer(mode);
    connection->send(std::shared_ptr<GameEventPacket>(
        new GameEventPacket(GameEventPacket::CHANGE_GAME_MODE, mode->getId())));
}

void ServerPlayer::sendMessage(
    const std::wstring& message,
    ChatPacket::EChatPacketMessage type ,
    int customData , const std::wstring& additionalMessage ) {
    connection->send(std::shared_ptr<ChatPacket>(
        new ChatPacket(message, type, customData, additionalMessage)));
}

bool ServerPlayer::hasPermission(EGameCommand command) {
    return server->getPlayers()->isOp(
        std::dynamic_pointer_cast<ServerPlayer>(shared_from_this()));

    
    






}




























int ServerPlayer::getViewDistance() { return viewDistance; }










void ServerPlayer::setEffectiveViewDistance(int chunks) {
    int target = clampViewDistance(chunks);

    
    
    int serverCap = server->getPlayers()->getViewDistance();
    if (target > serverCap) target = serverCap;

    if (target == viewDistance) return;  

    int old = viewDistance;
    viewDistance = target;

    getLevel()->getChunkMap()->adjustPlayerViewDistance(
        std::dynamic_pointer_cast<ServerPlayer>(shared_from_this()), old,
        target);
}











Pos* ServerPlayer::getCommandSenderWorldPosition() {
    return new Pos(std::floor(x), std::floor(y + .5), std::floor(z));
}

void ServerPlayer::resetLastActionTime() {
    this->lastActionTime = MinecraftServer::getCurrentTimeMillis();
}



int ServerPlayer::getFlagIndexForChunk(const ChunkPos& pos, int dimension) {
    
    
    return LevelRenderer::getGlobalIndexForChunk(pos.x * 16, 0, pos.z * 16,
                                                 dimension) /
           (Level::maxBuildHeight /
            16);  
}


int ServerPlayer::getPlayerViewDistanceModifier() {
    int value = 0;

    if (!connection->isLocal()) {
        INetworkPlayer* player = connection->getNetworkPlayer();

        if (player != nullptr) {
            int rtt = player->GetCurrentRtt();

            value = rtt >> 6;

            if (value > 4) value = 4;
        }
    }

    return value;
}

void ServerPlayer::handleCollectItem(std::shared_ptr<ItemInstance> item) {
    if (gameMode->getGameRules() != nullptr)
        gameMode->getGameRules()->onCollectItem(item);
}

#if !defined(_CONTENT_PACKAGE)
void ServerPlayer::debug_setPosition(double x, double y, double z, double nYRot,
                                     double nXRot) {
    connection->teleport(x, y, z, nYRot, nXRot);
}
#endif
